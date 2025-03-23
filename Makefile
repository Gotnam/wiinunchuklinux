obj-m += wiinunchuk.o

ARCH := arm64
CROSS_COMPILE := aarch64-linux-gnu-
MAKE_ARGS := ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)

PWD := $(shell pwd)
KDIR := /Insert/Kernel/build/directory/here
OVERLAY := /boot/firmware/overlays

.PHONY: clean all install

install: wiinunchuk.dtbo wiinunchuk.ko
	xz -zk wiinunchuk.ko
	cp wiinunchuk.dtbo $(OVERLAY)
	cp wiinunchuk.ko.xz $(KDIR)/../kernel/drivers/i2c
all: wiinunchuk.dtbo
	$(MAKE) $(MAKE_ARGS) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) $(MAKE_ARGS) -C $(KDIR) M=$(PWD) clean
	rm wiinunchuk.dtbo > /dev/null 2> /dev/null || true
wiinunchuk.dtbo:	wiinunchuk.dts
	dtc -@ -Hepapr -I dts -O dtb -o wiinunchuk.dtbo wiinunchuk.dts


