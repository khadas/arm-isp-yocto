obj-m		+= imx290/
obj-m		+= imx415/
obj-m		+= ov13b10/
obj-m		+= ov08a10/
obj-m		+= ov9285/
obj-m		+= ov5640/

KBUILD_CFLAGS_MODULE += $(GKI_EXT_MODULE_PREDEFINE)

ifeq ($(O),)
out_dir := .
else
out_dir := $(O)
endif
-include $(out_dir)/include/config/auto.conf

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules $(GKI_EXT_MODULE_CONFIG)

modules_install:
	-include $(OUT_DIR)/include/config/auto.conf
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules_install
	$(Q)mkdir -p ${out_dir}/../vendor_lib/modules
	$(Q)if [ -z "$(CONFIG_AMLOGIC_KERNEL_VERSION)" ]; then \
		cd ${out_dir}/$(M)/; find -name "*.ko" -exec cp {} ${out_dir}/../vendor_lib/modules/ \; ; \
	else \
		find $(INSTALL_MOD_PATH)/lib/modules/*/$(INSTALL_MOD_DIR) -name "*.ko" -exec cp {} ${out_dir}/../vendor_lib/modules/ \; ; \
	fi;

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) clean
