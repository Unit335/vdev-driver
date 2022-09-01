#include "vd_driver.h"

struct pcdrv_private_data pcdrv_data;

struct file_operations pcd_fops = {
    .open = vd_open,
    .read = vd_read,
    .write = vd_write,
    .llseek = vd_lseek,
    .mmap = vd_mmap,
    .release = vd_close,
    .owner = THIS_MODULE
};


// ============     sysfs       ==============
ssize_t get_value (struct device *dev, 
               struct device_attribute *attr,
               char *buf) 
{
    struct pcdev_private_data *dev_data = dev_get_drvdata(dev->parent);
    if(!dev_data) {
        pr_info("null reference by dev_data, %p\n", dev_data);
        return 0;
    }    
    return sysfs_emit(buf, "%hhu\n", dev_data->buffer[dev_data->position] );;
}

ssize_t set_value (struct device *dev, 
                        struct device_attribute *attr,
                        const char *buf, 
                        size_t count) 
{
    unsigned long result;
    int ret;
    struct pcdev_private_data *dev_data = dev_get_drvdata(dev->parent);

    ret = kstrtol(buf, 10, &result);
    if ( ret ) {
        return ret;
    }
    pr_info("New data = %lu; addr = %d\n", result, dev_data->position);
    snprintf(dev_data->buffer+(dev_data->position), count, "%c", (char)result);
    return count;
}

ssize_t get_addr (struct device *dev, 
               struct device_attribute *attr,
               char *buf) 
{
    struct pcdev_private_data *dev_data = dev_get_drvdata(dev->parent);
    return sysfs_emit(buf, "%d\n", dev_data->position);
}

ssize_t set_addr (struct device *dev, 
                        struct device_attribute *attr,
                        const char *buf, 
                        size_t count) 
{
    long result;
    int ret;
    struct pcdev_private_data *dev_data = dev_get_drvdata(dev->parent);
    ret = kstrtol(buf, 10, &result);
    if ( ret ){
        return ret;
    }
    dev_data->position = result;
    pr_info("New position for sysfs access = %ld\n", result);
    return count;
}

static DEVICE_ATTR(value, S_IRUGO|S_IWUSR, get_value, set_value);
static DEVICE_ATTR(address, S_IRUGO|S_IWUSR, get_addr, set_addr);

struct attribute *pcd_attrs[] = {
    &dev_attr_value.attr,
    &dev_attr_address.attr,
    NULL
};

struct attribute_group pcd_attr_group = {
    .attrs = pcd_attrs
};

int pcd_sysfs_create_file(struct device *pcd_dev) {
   return sysfs_create_group(&pcd_dev->kobj, &pcd_attr_group);
}

// ============     sysfs       ==============


//probe
int pcd_platform_driver_probe (struct platform_device *pdev) {
    u32 ret;
    struct pcdev_private_data *dev_data;
    struct pcdev_platform_data *pdata;
    struct device *dev = &pdev->dev;

    pr_info("Device detected\n");

    pdata = (struct pcdev_platform_data *)dev_get_platdata(&pdev->dev);
    if(!pdata) {
        pr_info("No platform data available\n");
        ret = -EINVAL;
        return ret;
    }
    dev_data = devm_kzalloc(&pdev->dev, sizeof(*dev_data), GFP_KERNEL);
    if(!dev_data) {
        pr_info("Can`t allocate memory\n");
        return -ENOMEM;
    } 

    dev_set_drvdata(&pdev->dev, dev_data);
    dev_data->pdata.size = pdata->size;
    pr_info("Device size = %d\n", dev_data->pdata.size);

    dev_data->buffer = kzalloc(dev_data->pdata.size, GFP_KERNEL);
    if(!dev_data->buffer) {
        pr_info("Cannot allocate memory\n");
        return -ENOMEM;
    }

    dev_data->device_number = pcdrv_data.device_num_base + pdev->id;

    cdev_init(&dev_data->pcd_cdev, &pcd_fops);
    dev_data->pcd_cdev.owner = THIS_MODULE;
    ret = cdev_add(&dev_data->pcd_cdev, dev_data->device_number, 1);
    if (ret < 0) {
        pr_err("cdev add failed\n");
        return ret; 
    }

    pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd, dev, dev_data->device_number, NULL, "vdev-%d", pdev->id);
    if(IS_ERR(pcdrv_data.device_pcd)) {
        pr_err("device create failed\n");
        ret = PTR_ERR(pcdrv_data.device_pcd);
        cdev_del(&dev_data->pcd_cdev);
        return ret;
    }
    
    pcdrv_data.total_devices++;
    ret = pcd_sysfs_create_file(pcdrv_data.device_pcd);
    if(ret) {
        device_destroy(pcdrv_data.class_pcd, dev_data->device_number);
        return ret;
    }

    pr_info("Probe successful\n");
    return 0;
}

int pcd_platform_driver_remove (struct platform_device *pdev) {
    
    struct pcdev_private_data *dev_data = dev_get_drvdata(&pdev->dev);
    device_destroy(pcdrv_data.class_pcd, dev_data->device_number);
    cdev_del(&dev_data->pcd_cdev);
    pcdrv_data.total_devices--;
    kfree(dev_data->buffer);
    pr_info("Device removed\n");
    return 0;
}

struct platform_device_id pcdevs_ids[] = {
    [0] = {.name= "vdev-A1x"},
    [1] = {.name= "vdev-B1x"},
    [2] = {.name= "vdev-C1x"},
    [3] = {.name= "vdev-D1x"},
    {} 
};

struct platform_driver pcd_platform_driver = {
    .probe = pcd_platform_driver_probe,
    .remove = pcd_platform_driver_remove,
    .id_table = pcdevs_ids,
    .driver = {
        .name = "virt_device"
    }
};

#define MAX_DEVICES 10
static int __init pcd_platform_driver_init(void)
{
    u32 ret;
    ret = alloc_chrdev_region(&pcdrv_data.device_num_base, 0, MAX_DEVICES, "pcdevs");
    if(ret < 0) {
        pr_err("Alloc chrdev failed\n");
        return ret;
    }

    pcdrv_data.class_pcd = class_create(THIS_MODULE, "vd_class");
    if(IS_ERR(pcdrv_data.class_pcd)) {
        pr_err("class creation failed\n");
        ret = PTR_ERR(pcdrv_data.class_pcd);
        unregister_chrdev_region(pcdrv_data.device_num_base, MAX_DEVICES);
        return ret;
    }

    platform_driver_register(&pcd_platform_driver);
    pr_info("Driver loaded\n");
    return 0;
}

static void __exit pcd_platform_driver_cleanup(void)
{
    platform_driver_unregister(&pcd_platform_driver);
    class_destroy(pcdrv_data.class_pcd);
    unregister_chrdev_region(pcdrv_data.device_num_base, MAX_DEVICES);
    pr_info("Driver unloaded\n");
}

module_init(pcd_platform_driver_init);
module_exit(pcd_platform_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ilya Dubovskii");
