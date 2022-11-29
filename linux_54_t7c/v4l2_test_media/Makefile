LOCAL_PATH := $(shell dirname $(lastword $(MAKEFILE_LIST)))

ISP_ROOT_DIR := $(LOCAL_PATH)/..
LOCAL_INCLUDES := -I$(ISP_ROOT_DIR)/include/
LOCAL_INCLUDES += -I$(ISP_ROOT_DIR)/sensor_calibration/imx290/


TARGET := v4l2_test_raw

SRC_OBJ := v4l2_test_raw.o

#CFLAGS := -Wall -Werror -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast
CFLAGS := -Wall -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast
CFLAGS += $(LOCAL_INCLUDES)
CFLAGS += -I./external -pthread -fPIE
CFLAGS += -L$(ISP_ROOT_DIR)/firmware -lispaml -L$(ISP_ROOT_DIR)/media-v4l2 -lmediaAPI -L$(ISP_ROOT_DIR)/sensor_calibration -ltuning -lm
CFLAGS += -fPIC -D_FORTIFY_SOURCE=2 -O3

#CFLAGS += -L./external -lH264VideoStreamer -lx264 -lcrypto -lssl

.PHONY:clean

all: $(TARGET)

$(TARGET): $(SRC_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf $(TARGET)
