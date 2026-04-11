#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#define DEVICE_NAME "pir"
#define CLASS_NAME  "pir_class"

struct pir_dev {
    int irq;
    struct gpio_desc *gpiod;
    int motion_detected;

    struct cdev cdev;
    dev_t devt;
};

static struct class *pir_class;
static int major_number;

/* ================= IRQ Handler ================= */
static irqreturn_t pir_irq_handler(int irq, void *dev_id)
{
    struct pir_dev *pir = dev_id;
    int val;

    val = gpiod_get_value(pir->gpiod);
    pir->motion_detected = val;

    printk(KERN_INFO "PIR interrupt! value=%d\n", val);

    return IRQ_HANDLED;
}

/* ================= File Ops ================= */
static int pir_open(struct inode *inode, struct file *file)
{
    struct pir_dev *pir;

    pir = container_of(inode->i_cdev, struct pir_dev, cdev);
    file->private_data = pir;

    return 0;
}

static ssize_t pir_read(struct file *file, char __user *buf,
                        size_t count, loff_t *ppos)
{
    struct pir_dev *pir = file->private_data;
    char kbuf[16];
    int len;

    if (*ppos > 0)
        return 0;

    len = snprintf(kbuf, sizeof(kbuf), "%d\n", pir->motion_detected);

    if (copy_to_user(buf, kbuf, len))
        return -EFAULT;

    *ppos = len;

    return len;
}

static struct file_operations pir_fops = {
    .owner = THIS_MODULE,
    .open = pir_open,
    .read = pir_read,
};

/* ================= Probe ================= */
static int pir_probe(struct platform_device *pdev)
{
    struct pir_dev *pir;
    int ret;

    printk(KERN_INFO "PIR driver probe\n");

    pir = devm_kzalloc(&pdev->dev, sizeof(*pir), GFP_KERNEL);
    if (!pir)
        return -ENOMEM;

    /* GPIO */
    pir->gpiod = devm_gpiod_get(&pdev->dev, "pir", GPIOD_IN);
    if (IS_ERR(pir->gpiod)) {
        dev_err(&pdev->dev, "Failed to get GPIO\n");
        return PTR_ERR(pir->gpiod);
    }

    /* IRQ */
    pir->irq = gpiod_to_irq(pir->gpiod);
    if (pir->irq < 0) {
        dev_err(&pdev->dev, "Failed to get IRQ\n");
        return pir->irq;
    }

    ret = devm_request_irq(&pdev->dev, pir->irq,
                           pir_irq_handler,
                           IRQF_TRIGGER_RISING,// | IRQF_TRIGGER_FALLING,
                           "pir_irq", pir);
    if (ret) {
        dev_err(&pdev->dev, "Failed to request IRQ\n");
        return ret;
    }

    /* Char Device allocation */
    ret = alloc_chrdev_region(&pir->devt, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        dev_err(&pdev->dev, "alloc_chrdev_region failed\n");
        return ret;
    }

    major_number = MAJOR(pir->devt);

    cdev_init(&pir->cdev, &pir_fops);
    pir->cdev.owner = THIS_MODULE;

    ret = cdev_add(&pir->cdev, pir->devt, 1);
    if (ret) {
        dev_err(&pdev->dev, "cdev_add failed\n");
        goto unregister_chrdev;
    }

    /* class create */
    pir_class = class_create(CLASS_NAME);
    if (IS_ERR(pir_class)) {
        dev_err(&pdev->dev, "class_create failed\n");
        ret = PTR_ERR(pir_class);
        goto del_cdev;
    }

    /* device create -> /dev/pir */
    if (!device_create(pir_class, NULL, pir->devt, NULL, DEVICE_NAME)) {
        dev_err(&pdev->dev, "device_create failed\n");
        ret = -ENOMEM;
        goto destroy_class;
    }

    platform_set_drvdata(pdev, pir);

    printk(KERN_INFO "PIR char device initialized (/dev/%s)\n", DEVICE_NAME);

    return 0;

destroy_class:
    class_destroy(pir_class);
del_cdev:
    cdev_del(&pir->cdev);
unregister_chrdev:
    unregister_chrdev_region(pir->devt, 1);
    return ret;
}

/* ================= Remove ================= */
static void pir_remove(struct platform_device *pdev)
{
    struct pir_dev *pir = platform_get_drvdata(pdev);

    device_destroy(pir_class, pir->devt);
    class_destroy(pir_class);

    cdev_del(&pir->cdev);
    unregister_chrdev_region(pir->devt, 1);

    printk(KERN_INFO "PIR driver removed\n");
}

/* ================= OF Match ================= */
static const struct of_device_id pir_of_match[] = {
    { .compatible = "custom,pir" },
    { }
};
MODULE_DEVICE_TABLE(of, pir_of_match);

/* ================= Platform Driver ================= */
static struct platform_driver pir_driver = {
    .probe = pir_probe,
    .remove = pir_remove,
    .driver = {
        .name = "pir_driver",
        .of_match_table = pir_of_match,
    },
};

module_platform_driver(pir_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hans");
MODULE_DESCRIPTION("PIR Sensor Char Device Driver");
