#include <linux/module.h>
#include <linux/platform_device.h>

#undef pr_fmt
#define pr_fmt(fmt) "%s: %s: " fmt, KBUILD_MODNAME, __func__
struct pcdev_platform_data {
    int size;
};
#define RDWR    0x11
#define RDONLY  0x01
#define WRONLY  0x10

void pcdev_release(struct device *dev) {
    pr_info("Device released\n");
}

struct pcdev_platform_data pcdev_pltf_data[4] = {
    [0] = {
        .size = 4096,
    },
    [1] = {
        .size = 4096,
    },
    [2] = {
        .size = 4096,
    },
    [3] = {
        .size = 4096,
    }
};

struct platform_device platform_pcdev_1 = {
    .name = "vdev-A1x",
    .id   = 0,
    .dev  = {
        .platform_data = &pcdev_pltf_data[0],
        .release       = pcdev_release
    }
};

struct platform_device platform_pcdev_2 = {
    .name = "vdev-B1x",
    .id   = 1,
    .dev = {
        .platform_data = &pcdev_pltf_data[1],
        .release       = pcdev_release
    }
};

struct platform_device platform_pcdev_3 = {
    .name = "vdev-C1x",
    .id   = 2,
    .dev = {
        .platform_data = &pcdev_pltf_data[2],
        .release       = pcdev_release
    }
};

struct platform_device platform_pcdev_4 = {
    .name = "vdev-D1x",
    .id   = 3,
    .dev = {
        .platform_data = &pcdev_pltf_data[3],
        .release       = pcdev_release
    }
};

struct platform_device *platform_pcdevs[] = {
    &platform_pcdev_1,
    &platform_pcdev_2,
    &platform_pcdev_3,
    &platform_pcdev_4
};

static int __init pcdev_platform_init(void) {
    platform_add_devices(platform_pcdevs, ARRAY_SIZE(platform_pcdevs));
    pr_info("Devices initialized\n");
    return 0;
}

static void __exit pcdev_platform_exit(void) {
    platform_device_unregister(&platform_pcdev_1);
    platform_device_unregister(&platform_pcdev_2);
    platform_device_unregister(&platform_pcdev_3);
    platform_device_unregister(&platform_pcdev_4);

    pr_info("Devices unloaded\n");
    return;
}

module_init(pcdev_platform_init);
module_exit(pcdev_platform_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ilya Dubovskiy");
MODULE_DESCRIPTION("Emulates devices for vd_driver");
