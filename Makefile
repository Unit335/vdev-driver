obj-m := vd_drv.o vd_devices.o
vd_drv-objs += vd_driver.o vd_syscalls.o
HOST_KERN_DIR = /lib/modules/$(shell uname -r)/build/

host:
	make -C $(HOST_KERN_DIR) M=$(PWD) modules 
clean:
	make -C $(HOST_KERN_DIR) M=$(PWD) clean 
help:
	make -C $(HOST_KERN_DIR) M=$(PWD) help 

enable:
	sudo insmod vd_devices.ko 
	sudo insmod vd_drv.ko
disable:
	sudo rmmod vd_devices
	sudo rmmod vd_drv
test: test.c
	gcc test.c -o test
