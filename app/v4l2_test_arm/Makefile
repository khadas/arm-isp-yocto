#LOCAL_PATH := $(shell dirname $(lastword $(MAKEFILE_LIST)))
#CC=/opt/gcc-linaro-6.3.1-2017.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
#CROSS_COMPILE=/mnt/fileroot2/shuanshuan.jiang/arm_5.15_1/prebuilts/gcc/linux-x86/host/x86_64-arm-10.3-2021.07/bin/arm-none-linux-gnueabihf-
#CC=$(CROSS_COMPILE)gcc
#CROSS_COMPILE=/opt/gcc-linaro-6.3.1-2017.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
LOCAL_PATH := $(shell pwd)

OUT_DIR ?= .
$(info "OUT_DIR : $(OUT_DIR)")

CFLAGS := -I. -g -fPIE -Wall -pthread -Wno-unused-parameter
OFILE=v4l2_test

#_OBJ = v4l2_test.o capture.o renderer.o isp_metadata.o gdc.o

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

%.o: %.c
	$(CC) -c -o $(patsubst %,$(OUT_DIR)/%,$@) $< $(CFLAGS)

$(OFILE): $(OBJS)
	$(CC) -o $(OUT_DIR)/$@ $(patsubst %,$(OUT_DIR)/%,$^) $(CFLAGS) -pie

.PHONY: clean

clean:
	@rm -rf $(OUT_DIR)/$(OFILE)
	@rm -rf $(patsubst %,$(OUT_DIR)/%,$(_OBJ))

$(shell mkdir -p $(OUT_DIR))