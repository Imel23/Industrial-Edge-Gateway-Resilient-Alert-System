obj-m += industrial_button.o

# Chemins par défaut (peuvent être surchargés par l'environnement Yocto)
KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean

# Cible pour vérifier le style du code avec le script officiel du noyau Linux
check:
	$(KERNEL_SRC)/scripts/checkpatch.pl --no-tree -f industrial_button.c
