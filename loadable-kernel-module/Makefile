# Defines a module to be built
obj-m += lkm_example.o

# The -C option switches the directory to the kernel directory
# before performing any make tasks
# The M=$(PWD) variable assignment tells the make command where
# the actual project files exist
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load_module:
	# Clear the kernel log without echo
	sudo dmesg --clear
	# Insert the module
	sudo insmod lkm_example.ko
	# Display and then clear the kernel log
	dmesg

unload_module:
	# Clear the kernel log without echo
	sudo dmesg --read-clear
	# Delete the module
	sudo rmmod lkm_example.ko
	# Display and then clear the kernel log
	dmesg
	
test:
	# Clear the kernel log without echo
	sudo dmesg --clear
	# Insert the module
	sudo insmod lkm_example.ko
	# Delete the module
	sudo rmmod lkm_example.ko
	# Display the kernel log
	dmesg