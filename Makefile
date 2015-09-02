ifneq ($(KERNELRELEASE),)
	obj-m := first-jeudi.o
	first-jeudi-objs = main.o perceptron.o
else

	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

endif
