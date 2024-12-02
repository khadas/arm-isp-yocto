LOCAL_ARCH := $(ARCH)
LOCAL_KDIR := $(KDIR)
LOCAL_CROSS_COMPILE := $(CROSS_COMPILE)
LOCAL_PATH := $(shell dirname $(lastword $(MAKEFILE_LIST)))

obj-m += control.o

DRV_SRC := $(wildcard $(LOCAL_PATH)/*.c) \
			$(wildcard $(LOCAL_PATH)/src/*.c)

M_DRV_SRC := $(DRV_SRC:$(LOCAL_PATH)/%=%)
DRV_SRC_OBJ := $(M_DRV_SRC:.c=.o)

ccflags-y := -I$(LOCAL_PATH)/ -I$(LOCAL_PATH)/include -Wno-implicit-fallthrough

ccflags-y += -Wno-unused-function
ccflags-y += -Wno-unused-variable
ccflags-y += -Wno-declaration-after-statement

control-objs := $(DRV_SRC_OBJ)

ifeq ($(O),)
out_dir := .
else
out_dir := $(O)
endif
-include $(out_dir)/include/config/auto.conf

all:
	@$(MAKE) -C $(KERNEL_SRC) M=$(M)  modules
modules_install:
	-include $(OUT_DIR)/include/config/auto.conf
	$(MAKE) INSTALL_MOD_STRIP=1 M=$(M) -C $(KERNEL_SRC) modules_install
	$(Q)mkdir -p $(out_dir)/../vendor_lib
	$(Q)if [ -z "$(CONFIG_AMLOGIC_KERNEL_VERSION)" ]; then \
		cp $(out_dir)/$(M)/*.ko ${out_dir}/../vendor_lib/modules/ ; \
	else \
		find $(INSTALL_MOD_PATH)/lib/modules/*/$(INSTALL_MOD_DIR) -name "*.ko" -exec cp {} $(out_dir)/../vendor_lib/modules/ \; ; \
	fi

clean:
	@$(MAKE) -C $(KERNEL_SRC) M=$(M) clean






