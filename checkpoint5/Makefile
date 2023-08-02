# Makefile for netfilter_module

# Set the name of the kernel module
obj-m += packet_filter.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

# Specify the target module





