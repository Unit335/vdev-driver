#pragma once

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mod_devicetable.h>
#include <linux/mm.h>
#include <linux/module.h>

#include <asm/page.h>
#include <asm-generic/memory_model.h>

#define RDWR    0x11
#define RDONLY  0x01
#define WRONLY  0x10
#undef pr_fmt
#define pr_fmt(fmt) "%s: %s: " fmt, KBUILD_MODNAME, __func__

struct pcdev_platform_data {
    int size;
};


/* Private data of a single device */
struct pcdev_private_data {
    struct pcdev_platform_data pdata;
    char *buffer;
    dev_t device_number;
    struct cdev pcd_cdev;
    int position;
};

/* Private data of the driver */
struct pcdrv_private_data {
    int total_devices;
    dev_t device_num_base;
    struct class *class_pcd;
    struct device *device_pcd;
};

int vd_open(struct inode *inode, struct file *fptr); 
int vd_close (struct inode *inode, struct file *fptr);
ssize_t vd_read(struct file *fptr, char __user *buff, size_t count, loff_t *f_pos);
ssize_t vd_write(struct file *fptr, const char __user *buff, size_t count, loff_t *f_pos);
loff_t vd_lseek(struct file *fptr, loff_t offset, int whence); 
int vd_mmap(struct file *filp, struct vm_area_struct *vma);