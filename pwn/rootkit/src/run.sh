#!/bin/bash

qemu-system-mips \
	-m 64M \
	-M malta \
	-nographic \
	-kernel vmlinux \
	-append 'root=/dev/ram rw console=ttyS0 loglevel=2 oops=panic panic=1 rdinit=/init quiet' \
	-monitor /dev/null \
	-initrd rootfs.img \
	-net nic,macaddr=52:54:00:fa:ce:73 \
	-net user
