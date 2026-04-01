// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * industrial_led.c - Industrial LED Control LKM for BeagleBone Black
 *
 * This driver provides:
 *   - Character device interface (/dev/industrial_led) for ON/OFF control
 *   - sysfs attribute (blink_freq_hz) for variable-frequency blinking
 *     using a high-resolution kernel timer (hrtimer)
 *
 * Writing a frequency (in Hz) to /sys/devices/platform/.../blink_freq_hz
 * starts blinking the LED at that rate.  Writing 0 stops blinking.
 *
 * Copyright (C) 2026  Imel23
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/version.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/sysfs.h>
#include <linux/spinlock.h>

#define DRIVER_NAME  "industrial_led"
#define DEVICE_NAME  "industrial_led"

/* Maximum supported blink frequency in Hz */
#define MAX_BLINK_FREQ_HZ  1000

struct industrial_led_data {
	struct gpio_desc  *led_gpio;
	struct cdev        cdev;
	dev_t              dev_num;
	struct class      *class;
	struct device     *dev;
	struct mutex       lock;

	/* hrtimer blinking support */
	struct hrtimer     blink_timer;
	spinlock_t         timer_lock;
	unsigned int       blink_freq_hz;  /* 0 = off */
	ktime_t            blink_period;   /* half-period (time per toggle) */
	bool               led_state;      /* current LED state during blink */
	bool               blinking;       /* true when timer is active */
};

/* ------------------------------------------------------------------ */
/*  hrtimer callback – runs in hard IRQ context                       */
/* ------------------------------------------------------------------ */
static enum hrtimer_restart led_blink_timer_cb(struct hrtimer *timer)
{
	struct industrial_led_data *data =
		container_of(timer, struct industrial_led_data, blink_timer);
	unsigned long flags;

	spin_lock_irqsave(&data->timer_lock, flags);

	if (!data->blinking) {
		spin_unlock_irqrestore(&data->timer_lock, flags);
		return HRTIMER_NORESTART;
	}

	/* Toggle LED */
	data->led_state = !data->led_state;
	gpiod_set_value(data->led_gpio, data->led_state ? 1 : 0);

	/* Re-arm for the next half-period */
	hrtimer_forward_now(timer, data->blink_period);

	spin_unlock_irqrestore(&data->timer_lock, flags);
	return HRTIMER_RESTART;
}

/* ------------------------------------------------------------------ */
/*  sysfs: blink_freq_hz  (show / store)                              */
/* ------------------------------------------------------------------ */
static ssize_t blink_freq_hz_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	struct industrial_led_data *data = dev_get_drvdata(dev);
	unsigned int freq;
	unsigned long flags;

	spin_lock_irqsave(&data->timer_lock, flags);
	freq = data->blink_freq_hz;
	spin_unlock_irqrestore(&data->timer_lock, flags);

	return sysfs_emit(buf, "%u\n", freq);
}

static ssize_t blink_freq_hz_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct industrial_led_data *data = dev_get_drvdata(dev);
	unsigned int freq;
	unsigned long flags;
	int ret;

	ret = kstrtouint(buf, 10, &freq);
	if (ret)
		return ret;

	if (freq > MAX_BLINK_FREQ_HZ) {
		dev_err(dev, "Frequency %u Hz exceeds max %d Hz\n",
			freq, MAX_BLINK_FREQ_HZ);
		return -EINVAL;
	}

	spin_lock_irqsave(&data->timer_lock, flags);

	if (freq == 0) {
		/* Stop blinking */
		data->blinking = false;
		data->blink_freq_hz = 0;
		spin_unlock_irqrestore(&data->timer_lock, flags);

		hrtimer_cancel(&data->blink_timer);

		/* Turn LED off when stopping */
		mutex_lock(&data->lock);
		gpiod_set_value(data->led_gpio, 0);
		data->led_state = false;
		mutex_unlock(&data->lock);

		dev_info(dev, "Blinking stopped\n");
	} else {
		/*
		 * Half-period = 1 / (2 * freq) seconds
		 * In nanoseconds: 1_000_000_000 / (2 * freq)
		 */
		ktime_t half_period = ktime_set(0,
			(unsigned long)(NSEC_PER_SEC / (2UL * freq)));

		data->blink_freq_hz = freq;
		data->blink_period = half_period;
		data->led_state = false;

		if (!data->blinking) {
			/* First start – arm the timer */
			data->blinking = true;
			spin_unlock_irqrestore(&data->timer_lock, flags);
			hrtimer_start(&data->blink_timer, half_period,
				      HRTIMER_MODE_REL);
		} else {
			/* Already blinking – timer will pick up new period
			 * at next expiry via hrtimer_forward_now() */
			spin_unlock_irqrestore(&data->timer_lock, flags);
		}

		dev_info(dev, "Blinking at %u Hz (half-period %lld ns)\n",
			 freq, ktime_to_ns(half_period));
	}

	return count;
}

static DEVICE_ATTR_RW(blink_freq_hz);

static struct attribute *led_attrs[] = {
	&dev_attr_blink_freq_hz.attr,
	NULL,
};

static const struct attribute_group led_attr_group = {
	.attrs = led_attrs,
};

/* ------------------------------------------------------------------ */
/*  Character device file operations  (unchanged from US103)          */
/* ------------------------------------------------------------------ */
static int led_open(struct inode *inode, struct file *file)
{
	struct industrial_led_data *data =
		container_of(inode->i_cdev, struct industrial_led_data, cdev);
	file->private_data = data;
	pr_info("%s: Device opened\n", DRIVER_NAME);
	return 0;
}

static int led_release(struct inode *inode, struct file *file)
{
	pr_info("%s: Device closed\n", DRIVER_NAME);
	return 0;
}

static ssize_t led_read(struct file *file, char __user *user_buffer,
			size_t count, loff_t *offs)
{
	struct industrial_led_data *data = file->private_data;
	char buf[4];
	int len;
	int state;

	if (*offs > 0)
		return 0;

	mutex_lock(&data->lock);
	state = gpiod_get_value(data->led_gpio);
	mutex_unlock(&data->lock);

	len = scnprintf(buf, sizeof(buf), "%d\n", state);
	if (count < len)
		return -EINVAL;

	if (copy_to_user(user_buffer, buf, len))
		return -EFAULT;

	*offs += len;
	return len;
}

static ssize_t led_write(struct file *file, const char __user *user_buffer,
			 size_t count, loff_t *offs)
{
	struct industrial_led_data *data = file->private_data;
	unsigned long flags;
	char buf[2];

	if (count > 2)
		count = 2;

	if (copy_from_user(buf, user_buffer, count))
		return -EFAULT;

	/*
	 * Manual ON/OFF via /dev/industrial_led implicitly stops any
	 * active blink timer so the two control paths don't fight.
	 */
	spin_lock_irqsave(&data->timer_lock, flags);
	if (data->blinking) {
		data->blinking = false;
		data->blink_freq_hz = 0;
		spin_unlock_irqrestore(&data->timer_lock, flags);
		hrtimer_cancel(&data->blink_timer);
	} else {
		spin_unlock_irqrestore(&data->timer_lock, flags);
	}

	mutex_lock(&data->lock);
	if (buf[0] == '1') {
		gpiod_set_value(data->led_gpio, 1);
		data->led_state = true;
		pr_info("%s: LED turned ON\n", DRIVER_NAME);
	} else if (buf[0] == '0') {
		gpiod_set_value(data->led_gpio, 0);
		data->led_state = false;
		pr_info("%s: LED turned OFF\n", DRIVER_NAME);
	} else {
		pr_err("%s: Invalid input. Use '1' or '0'\n", DRIVER_NAME);
		mutex_unlock(&data->lock);
		return -EINVAL;
	}
	mutex_unlock(&data->lock);

	return count;
}

static const struct file_operations led_fops = {
	.owner   = THIS_MODULE,
	.open    = led_open,
	.release = led_release,
	.read    = led_read,
	.write   = led_write,
};

/* ------------------------------------------------------------------ */
/*  Platform driver probe / remove                                     */
/* ------------------------------------------------------------------ */
static int led_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct industrial_led_data *led_data;

	pr_info("%s: Probe function called\n", DRIVER_NAME);

	led_data = devm_kzalloc(dev, sizeof(*led_data), GFP_KERNEL);
	if (!led_data)
		return -ENOMEM;

	platform_set_drvdata(pdev, led_data);
	mutex_init(&led_data->lock);
	spin_lock_init(&led_data->timer_lock);

	/* ---- GPIO ---- */
	led_data->led_gpio = devm_gpiod_get(dev, NULL, GPIOD_OUT_LOW);
	if (IS_ERR(led_data->led_gpio)) {
		pr_err("%s: Failed to get GPIO from Device Tree\n", DRIVER_NAME);
		return PTR_ERR(led_data->led_gpio);
	}

	/* ---- hrtimer init (does NOT start it yet) ---- */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 12, 0)
	hrtimer_setup(&led_data->blink_timer, led_blink_timer_cb,
		      CLOCK_MONOTONIC, HRTIMER_MODE_REL);
#else
	hrtimer_init(&led_data->blink_timer, CLOCK_MONOTONIC,
		     HRTIMER_MODE_REL);
	led_data->blink_timer.function = led_blink_timer_cb;
#endif
	led_data->blinking = false;
	led_data->blink_freq_hz = 0;
	led_data->led_state = false;

	/* ---- char device registration ---- */
	ret = alloc_chrdev_region(&led_data->dev_num, 0, 1, DEVICE_NAME);
	if (ret < 0) {
		pr_err("%s: Failed to allocate char dev region\n", DRIVER_NAME);
		return ret;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
	led_data->class = class_create("industrial_led_class");
#else
	led_data->class = class_create(THIS_MODULE, "industrial_led_class");
#endif
	if (IS_ERR(led_data->class)) {
		ret = PTR_ERR(led_data->class);
		goto unregister_chrdev;
	}

	cdev_init(&led_data->cdev, &led_fops);
	led_data->cdev.owner = THIS_MODULE;

	ret = cdev_add(&led_data->cdev, led_data->dev_num, 1);
	if (ret < 0) {
		pr_err("%s: Failed to add cdev\n", DRIVER_NAME);
		goto destroy_class;
	}

	led_data->dev = device_create(led_data->class, NULL,
				      led_data->dev_num, NULL, DEVICE_NAME);
	if (IS_ERR(led_data->dev)) {
		ret = PTR_ERR(led_data->dev);
		goto delete_cdev;
	}

	/* ---- sysfs attribute group on the *platform* device ---- */
	ret = sysfs_create_group(&dev->kobj, &led_attr_group);
	if (ret) {
		pr_err("%s: Failed to create sysfs group\n", DRIVER_NAME);
		goto destroy_device;
	}

	pr_info("%s: Driver initialized successfully. Device created at /dev/%s\n",
		DRIVER_NAME, DEVICE_NAME);
	pr_info("%s: Blink frequency sysfs: /sys/devices/platform/.../%s/blink_freq_hz\n",
		DRIVER_NAME, DRIVER_NAME);
	return 0;

destroy_device:
	device_destroy(led_data->class, led_data->dev_num);
delete_cdev:
	cdev_del(&led_data->cdev);
destroy_class:
	class_destroy(led_data->class);
unregister_chrdev:
	unregister_chrdev_region(led_data->dev_num, 1);
	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0)
static void led_remove(struct platform_device *pdev)
#else
static int led_remove(struct platform_device *pdev)
#endif
{
	struct industrial_led_data *led_data = platform_get_drvdata(pdev);
	unsigned long flags;

	pr_info("%s: Remove function called\n", DRIVER_NAME);

	/* ---- Stop the blink timer first ---- */
	spin_lock_irqsave(&led_data->timer_lock, flags);
	led_data->blinking = false;
	led_data->blink_freq_hz = 0;
	spin_unlock_irqrestore(&led_data->timer_lock, flags);

	hrtimer_cancel(&led_data->blink_timer);

	/* ---- Turn LED off ---- */
	mutex_lock(&led_data->lock);
	gpiod_set_value(led_data->led_gpio, 0);
	led_data->led_state = false;
	mutex_unlock(&led_data->lock);

	/* ---- Tear down sysfs, char dev, class ---- */
	sysfs_remove_group(&pdev->dev.kobj, &led_attr_group);
	device_destroy(led_data->class, led_data->dev_num);
	cdev_del(&led_data->cdev);
	class_destroy(led_data->class);
	unregister_chrdev_region(led_data->dev_num, 1);

	pr_info("%s: Driver removed successfully\n", DRIVER_NAME);
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0)
	return 0;
#endif
}

static const struct of_device_id led_of_match[] = {
	{ .compatible = "industrial,led-gpio", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, led_of_match);

static struct platform_driver led_driver = {
	.probe  = led_probe,
	.remove = led_remove,
	.driver = {
		.name           = DRIVER_NAME,
		.of_match_table = led_of_match,
	},
};

module_platform_driver(led_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Imel23");
MODULE_DESCRIPTION("Industrial LED Control LKM with hrtimer blink support");
MODULE_VERSION("2.0");