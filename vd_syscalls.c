#include "vd_driver.h"

int vd_open(struct inode *inode, struct file *fptr)
{
    u32 minor_n;
    struct pcdev_private_data *pcdev_data;

    minor_n = MINOR(inode->i_rdev);
    pr_info("minor access = %d\n", minor_n);
    pcdev_data = container_of(inode->i_cdev, struct pcdev_private_data, pcd_cdev);
    fptr->private_data = pcdev_data;

    pr_info("opened successfully\n");
    return 0;
}

int vd_close(struct inode *inode, struct file *fptr)
{
    pr_info("closed successfully\n");
    return 0;
}

ssize_t vd_read(struct file *fptr, char __user *buff, size_t count, loff_t *f_pos)
{
    struct pcdev_private_data *data = (struct pcdev_private_data *)fptr->private_data;
    pr_info("read for %zu bytes \n", count);
    pr_info("current file position = %lld\n", *f_pos);

    if (*f_pos >= data->pdata.size) {
        pr_info("Wrong position for read\n");
        return 0;
    }
    if (*f_pos + count > data->pdata.size) {
        count = data->pdata.size - *f_pos;
    }

    if (copy_to_user(buff, data->buffer + *f_pos, count)) {
        pr_info("copy_to_user error\n");
        return -EFAULT;
    }

    *f_pos += count;

    pr_info("%zu bytes read\n", count);
    pr_info("new file position = %lld\n", *f_pos);

    return count;
}

ssize_t vd_write(struct file *fptr, const char __user *buff, size_t count, loff_t *f_pos)
{
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)fptr->private_data;
    u32 max_size = pcdev_data->pdata.size;

    pr_info("write for %zu bytes \n", count);
    pr_info("current file position = %lld\n", *f_pos);

    if ((*f_pos + count) > max_size) {
        count = max_size - *f_pos;
    }
    if (!count) {
    pr_err("not enough memory for write\n");
        return -ENOMEM;
    }
    if (copy_from_user(pcdev_data->buffer + (*f_pos), buff, count)) {
        return -EFAULT;
    }

    *f_pos += count;
    pr_info("%zu bytes written\n", count);
    pr_info("new file position = %lld\n", *f_pos);

    return count;
}

loff_t vd_lseek(struct file *fptr, loff_t offset, int whence)
{
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)fptr->private_data;
    u32 max_size = pcdev_data->pdata.size;
    loff_t temp;

    pr_info("received lseek request\n");
    pr_info("current position = %lld\n", fptr->f_pos);

    switch (whence) {
        case SEEK_SET:
            if ((offset > max_size) || (offset < 0))
                return -EINVAL;
            fptr->f_pos = offset;
            break;
        case SEEK_CUR:
            temp = fptr->f_pos + offset;
            if ((temp > max_size) || (temp < 0))
                return -EINVAL;
            fptr->f_pos = temp;
            break;
        case SEEK_END:
            temp = max_size + offset;
            if ((temp > max_size) || (temp < 0))
                return -EINVAL;
            fptr->f_pos = temp;
            break;
        default:
            return -EINVAL;
    }

    pr_info("new position = %lld\n", fptr->f_pos);
    return fptr->f_pos;
}

int vd_mmap(struct file *filp, struct vm_area_struct *vma)
{
    struct pcdev_private_data *data = filp->private_data;
    size_t size = vma->vm_end - vma->vm_start;
    unsigned long offset = vma->vm_pgoff;
    struct page *page;

    pr_info("mmap requested\n");

    page = virt_to_page(data->buffer + offset);
    if (remap_pfn_range(vma, vma->vm_start, page_to_pfn(page), size,
                          vma->vm_page_prot)) {
        pr_info("Remapping error\n");
        return -EAGAIN;
    }

    pr_info("mmap successfull\n");

    return 0;
}
