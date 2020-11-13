obj-m += lkm_example.o

# -C flag point where the program go at the beggining
# constant M describe where module binaries are located
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load_module:
	# Insert the module
	sudo insmod lkm_example.ko
	# Display and then clear the kernel log
	sudo dmesg --read-clear

unload_module:
	# Delete the module
	sudo rmmod lkm_example.ko
	# Display and then clear the kernel log
	sudo dmesg --read-clear

test:
	# Clear the kernel log without echo
	sudo dmesg --clear
	# Insert the module
	sudo insmod lkm_example.ko
	# Delete the module
	sudo rmmod lkm_example.ko
	# Display the kernel log
	dmesg