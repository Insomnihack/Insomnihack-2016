obj-m += rootkit.o

MODULES_DIR=$(shell pwd)/../linux-4.5/

all:
	make -C $(MODULES_DIR) M=$(PWD) modules
	cp rootkit.ko fs/lib/modules/
	cd fs && find . |cpio -H newc -o | gzip > ../rootfs.img

clean:
	make -C $(MODULES_DIR) M=$(PWD) clean
