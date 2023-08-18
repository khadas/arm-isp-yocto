
obj-m += amlsens.o

M_PATH := $(shell dirname $(lastword $(MAKEFILE_LIST)))

ifeq ($(DRV_SRC_OBJ),)
	DRV_SRC := $(wildcard *.c imx290/*.c imx335/*.c imx378/*.c imx415/*.c ov08a10/*.c ov5640/*.c \
				ov13b10/*.c imx577/*.c ov16a1q/*c ov13855/*.c lt6911c/*.c)
	export DRV_SRC_OBJ := $(DRV_SRC:.c=.o)
endif

amlsens-objs := $(DRV_SRC_OBJ)

LOCAL_INCLUDES := -I$(M_PATH)/ -I$(M_PATH)/include/

EXTRA_CFLAGS += $(LOCAL_INCLUDES)

all:
	@$(MAKE) -C $(KERNEL_SRC) M=$(M)  modules

modules_install:
	@echo "$(MAKE) INSTALL_MOD_STRIP=1 M=$(M) -C $(KERNEL_SRC) modules_install"
	@$(MAKE) INSTALL_MOD_STRIP=1 M=$(M) -C $(KERNEL_SRC) modules_install
	mkdir -p ${OUT_DIR}/../vendor_lib
	cp $(OUT_DIR)/$(M)/*.ko ${OUT_DIR}/../vendor_lib/modules/

clean:
	@$(MAKE) -C $(KERNEL_SRC) M=$(M) clean
