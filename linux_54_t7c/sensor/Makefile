obj-m		+= imx290/
obj-m		+= imx415/
obj-m		+= ov13b10/
obj-m		+= ov08a10/
obj-m		+= ov9285/
obj-m		+= ov5640/

KBUILD_CFLAGS_MODULE += $(GKI_EXT_MODULE_PREDEFINE)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules $(GKI_EXT_MODULE_CONFIG)

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules_install
	mkdir -p ${OUT_DIR}/../vendor_lib
	cd ${OUT_DIR}/$(M)/; find -name "*.ko" -exec cp {} ${OUT_DIR}/../vendor_lib/modules/ \;

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) clean
