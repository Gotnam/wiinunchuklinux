obj-m += wiinunchuk.o

ARCH := arm64
CROSS_COMPILE := aarch64-linux-gnu-
MAKE_ARGS := ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)

PWD := $(shell pwd)
KDIR := ~/Documents/LinuxProjects/kernel-headers/linux-headers-custom

all:
	$(MAKE) $(MAKE_ARGS) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) $(MAKE_ARGS) -C $(KDIR) M=$(PWD) clean

dts:	wiinunchuk.dts
	dtc
