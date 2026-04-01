obj-m += industrial_dht11.o

# Default kernel source path (overridden by Yocto build environment)
KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean

# Kernel coding style check
check:
	$(KERNEL_SRC)/scripts/checkpatch.pl --no-tree -f industrial_dht11.c
