obj-m+=fifodev.o
 
all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	# $(CC) test-lkm-char.c -o test
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	# rm test

test:
	# Clear the kernel log without echo
	sudo dmesg --clear
	sudo insmod fifodev.ko
	lsmod | grep "fifodev"
	sudo rmmod fifodev
	dmesg