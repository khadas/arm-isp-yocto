obj-m += lt6911c_auto.o

M_PATH := $(shell dirname $(lastword $(MAKEFILE_LIST)))

DRV_SRC := $(wildcard $(M_PATH)/*.c)

M_DRV_SRC := $(DRV_SRC:$(M_PATH)/%=%)

DRV_SRC_OBJ := $(M_DRV_SRC:.c=.o)

ccflags-y += -DCONFIG_G12B
ccflags-y += -DCONFIG_MEDIA_CONTROLLER
ccflags-y += -DCONFIG_VIDEO_V4L2_SUBDEV_API
ccflags-y += -Wno-unused-function
ccflags-y += -Wno-unused-variable
lt6911c_auto-objs += $(DRV_SRC_OBJ)


all:
	$(MAKE) -C $(KERNEL_SRC) M=$(M)  modules

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) clean

modules_install:
	@echo "$(MAKE) INSTALL_MOD_STRIP=1 M=$(M) -C $(KERNEL_SRC) modules_install"
	$(MAKE) INSTALL_MOD_STRIP=1 M=$(M) -C $(KERNEL_SRC) modules_install
	mkdir -p ${OUT_DIR}/../vendor_lib
	cp $(OUT_DIR)/$(M)/*.ko ${OUT_DIR}/../vendor_lib/modules/