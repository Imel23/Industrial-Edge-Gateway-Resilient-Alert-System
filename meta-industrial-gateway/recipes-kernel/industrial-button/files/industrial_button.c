#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>
#include <linux/version.h>

#define DRIVER_NAME "industrial_button"
#define DEBOUNCE_TIME_MS 50 

struct industrial_button_data {
    struct gpio_desc *btn_gpio;
    int irq_num;
    unsigned int press_count;
    struct timer_list debounce_timer;
    bool is_debouncing;
};



static void button_debounce_timer_callback(struct timer_list *t)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 15, 0)
    struct industrial_button_data *data = timer_container_of(data, t, debounce_timer);
#else
    struct industrial_button_data *data = from_timer(data, t, debounce_timer);
#endif
    
    data->is_debouncing = false;

    if (!gpiod_get_value(data->btn_gpio)) {
        pr_info("%s: Button released (stable)\n", DRIVER_NAME);
    }
}

static irqreturn_t button_irq_handler(int irq, void *dev_id)
{
    struct industrial_button_data *data = dev_id;

    if (data->is_debouncing) {
        return IRQ_HANDLED;
    }

    data->is_debouncing = true;

    if (gpiod_get_value(data->btn_gpio)) {
        data->press_count++;
        pr_info("%s: Button PRESSED! Total presses: %u\n", DRIVER_NAME, data->press_count);
    }

    mod_timer(&data->debounce_timer, jiffies + msecs_to_jiffies(DEBOUNCE_TIME_MS));

    return IRQ_HANDLED;
}

static int button_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct industrial_button_data *btn_data;
    int ret;

    pr_info("%s: Probe function called\n", DRIVER_NAME);

    btn_data = devm_kzalloc(dev, sizeof(struct industrial_button_data), GFP_KERNEL);
    if (!btn_data)
        return -ENOMEM;

    platform_set_drvdata(pdev, btn_data);
    btn_data->press_count = 0;
    btn_data->is_debouncing = false;

    btn_data->btn_gpio = devm_gpiod_get(dev, NULL, GPIOD_IN);
    if (IS_ERR(btn_data->btn_gpio)) {
        pr_err("%s: Failed to get GPIO from Device Tree\n", DRIVER_NAME);
        return PTR_ERR(btn_data->btn_gpio);
    }

    btn_data->irq_num = gpiod_to_irq(btn_data->btn_gpio);
    if (btn_data->irq_num < 0) {
        pr_err("%s: Failed to get IRQ number for GPIO\n", DRIVER_NAME);
        return btn_data->irq_num;
    }
    pr_info("%s: GPIO mapped to IRQ number %d\n", DRIVER_NAME, btn_data->irq_num);

    timer_setup(&btn_data->debounce_timer, button_debounce_timer_callback, 0);

    ret = devm_request_irq(dev, 
                           btn_data->irq_num, 
                           button_irq_handler, 
                           IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, 
                           DRIVER_NAME, 
                           btn_data); 
                           
    if (ret) {
        pr_err("%s: Failed to request IRQ %d\n", DRIVER_NAME, btn_data->irq_num);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 15, 0)
        timer_delete_sync(&btn_data->debounce_timer);
#else
        del_timer_sync(&btn_data->debounce_timer);
#endif
        return ret;
    }

    pr_info("%s: Driver initialized successfully.\n", DRIVER_NAME);
    return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0)
static void button_remove(struct platform_device *pdev)
#else
static int button_remove(struct platform_device *pdev)
#endif
{
    struct industrial_button_data *btn_data = platform_get_drvdata(pdev);

    pr_info("%s: Remove function called\n", DRIVER_NAME);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 15, 0)
    timer_delete_sync(&btn_data->debounce_timer);
#else
    del_timer_sync(&btn_data->debounce_timer);
#endif

    pr_info("%s: Driver removed successfully\n", DRIVER_NAME);
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0)
    return 0;
#endif
}

static const struct of_device_id button_of_match[] = {
    { .compatible = "industrial,button-gpio", },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, button_of_match);

static struct platform_driver button_driver = {
    .probe = button_probe,
    .remove = button_remove,
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = button_of_match,
    },
};

module_platform_driver(button_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Imel23");
MODULE_DESCRIPTION("Industrial Button IRQ Driver with Debouncing for BeagleBone Black");
MODULE_VERSION("1.0");