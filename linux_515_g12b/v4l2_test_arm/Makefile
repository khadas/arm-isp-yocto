#LOCAL_PATH := $(shell dirname $(lastword $(MAKEFILE_LIST)))
#CC=/opt/gcc-linaro-6.3.1-2017.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
#CROSS_COMPILE=/mnt/fileroot2/shuanshuan.jiang/arm_5.15_1/prebuilts/gcc/linux-x86/host/x86_64-arm-10.3-2021.07/bin/arm-none-linux-gnueabihf-
#CC=$(CROSS_COMPILE)gcc
#CROSS_COMPILE=/opt/gcc-linaro-6.3.1-2017.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
LOCAL_PATH := $(shell pwd)
DIR_PATH := /mnt/fileroot2/shuanshuan.jiang/arm_5.15_1/common/common_drivers/customer

CFLAGS := -I. -g -fPIE -Wall -pthread -Wno-unused-parameter
ODIR=obj
OFILE=v4l2_test

#_OBJ = v4l2_test.o capture.o renderer.o isp_metadata.o gdc.o
_OBJ = v4l2_test.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))
$(shell  mkdir -p $(ODIR))

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(OFILE): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -pie

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(OFILE)
install:
	find $(LOCAL_PATH) -name "v4l2_test" | xargs -i cp {} $(DIR_PATH)
