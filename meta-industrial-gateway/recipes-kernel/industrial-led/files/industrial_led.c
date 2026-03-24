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

#define DRIVER_NAME "industrial_led"
#define DEVICE_NAME "industrial_led"

struct industrial_led_data {
    struct gpio_desc *led_gpio;
    struct cdev cdev;
    dev_t dev_num;
    struct class *class;
    struct device *dev;
    struct mutex lock;
};

static int led_open(struct inode *inode, struct file *file)
{
    struct industrial_led_data *data = container_of(inode->i_cdev, struct industrial_led_data, cdev);
    file->private_data = data;
    pr_info("%s: Device opened\n", DRIVER_NAME);
    return 0;
}

static int led_release(struct inode *inode, struct file *file)
{
    pr_info("%s: Device closed\n", DRIVER_NAME);
    return 0;
}

static ssize_t led_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offs)
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

static ssize_t led_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *offs)
{
    struct industrial_led_data *data = file->private_data;
    char buf[2];
    
    if (count > 2)
        count = 2;
        
    if (copy_from_user(buf, user_buffer, count))
        return -EFAULT;
        
    mutex_lock(&data->lock);
    if (buf[0] == '1') {
        gpiod_set_value(data->led_gpio, 1);
        pr_info("%s: LED turned ON\n", DRIVER_NAME);
    } else if (buf[0] == '0') {
        gpiod_set_value(data->led_gpio, 0);
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
    .owner = THIS_MODULE,
    .open = led_open,
    .release = led_release,
    .read = led_read,
    .write = led_write,
};

static int led_probe(struct platform_device *pdev)
{
    int ret;
    struct device *dev = &pdev->dev;
    struct industrial_led_data *led_data;

    pr_info("%s: Probe function called\n", DRIVER_NAME);

    led_data = devm_kzalloc(dev, sizeof(struct industrial_led_data), GFP_KERNEL);
    if (!led_data)
        return -ENOMEM;

    platform_set_drvdata(pdev, led_data);
    mutex_init(&led_data->lock);

    led_data->led_gpio = devm_gpiod_get(dev, NULL, GPIOD_OUT_LOW);
    if (IS_ERR(led_data->led_gpio)) {
        pr_err("%s: Failed to get GPIO from Device Tree\n", DRIVER_NAME);
        return PTR_ERR(led_data->led_gpio);
    }

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

    led_data->dev = device_create(led_data->class, NULL, led_data->dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(led_data->dev)) {
        ret = PTR_ERR(led_data->dev);
        goto delete_cdev;
    }

    pr_info("%s: Driver initialized successfully. Device created at /dev/%s\n", DRIVER_NAME, DEVICE_NAME);
    return 0;

delete_cdev:
    cdev_del(&led_data->cdev);
destroy_class:
    class_destroy(led_data->class);
unregister_chrdev:
    unregister_chrdev_region(led_data->dev_num, 1);
    return ret;
}

static int led_remove(struct platform_device *pdev)
{
    struct industrial_led_data *led_data = platform_get_drvdata(pdev);

    pr_info("%s: Remove function called\n", DRIVER_NAME);

    mutex_lock(&led_data->lock);
    gpiod_set_value(led_data->led_gpio, 0);
    mutex_unlock(&led_data->lock);

    device_destroy(led_data->class, led_data->dev_num);
    cdev_del(&led_data->cdev);
    class_destroy(led_data->class);
    unregister_chrdev_region(led_data->dev_num, 1);

    pr_info("%s: Driver removed successfully\n", DRIVER_NAME);
    return 0;
}

static const struct of_device_id led_of_match[] = {
    { .compatible = "industrial,led-gpio", },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, led_of_match);

static struct platform_driver led_driver = {
    .probe = led_probe,
    .remove = led_remove,
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = led_of_match,
    },
};

module_platform_driver(led_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Imel23");
MODULE_DESCRIPTION("Industrial LED Control LKM for BeagleBone Black");
MODULE_VERSION("1.0");