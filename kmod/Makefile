obj-m += jc_kmod.o
CFLAGS_jc_kmod.o := -mhard-float
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
test:
	dmesg -C
	insmod jc_kmod.ko
	rmmod jc_kmod
	dmesg
