
# CROSS_COMPILE := /opt/gcc-linaro-7.2.1-2017.11-rc1_arm-linux-gnueabihf/bin/arm-linux-gnueabihf
CROSS_COMPILE := /mnt/fileroot2/junwei.ma/project/buildroot/C3_buildroot_MBP_20220629/toolchain/gcc/linux-x86/arm/gcc-arm-10.3-2021.07-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf
CC := $(CROSS_COMPILE)-gcc


TARGET := v4l2_test_media

SRC_FILE := $(wildcard ./*.c)
SRC_OBJ := $(patsubst %.c,%.o,$(SRC_FILE))

CFLAGS := -Wall -Werror -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast

CFLAGS += -pthread

.PHONY:clean

all: $(TARGET)

$(TARGET): $(SRC_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf ./*.o $(TARGET)
