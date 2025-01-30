obj-m += wiinunchuk.o

ARCH := arm64
CROSS_COMPILE := aarch64-linux-gnu-
MAKE_ARGS := ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)

PWD := $(shell pwd)
KDIR := ~/Documents/LinuxProjects/kernel-headers/linux-headers-custom

all: wiinunchuk.dtbo
	$(MAKE) $(MAKE_ARGS) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) $(MAKE_ARGS) -C $(KDIR) M=$(PWD) clean
	rm wiinunchuk.dtbo > /dev/null 2> /dev/null || true
wiinunchuk.dtbo:	wiinunchuk.dts
	dtc -@ -Hepapr -I dts -O dtb -o wiinunchuk.dtbo wiinunchuk.dts
