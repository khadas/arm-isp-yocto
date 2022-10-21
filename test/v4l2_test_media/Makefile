
# CROSS_COMPILE := /opt/gcc-linaro-7.2.1-2017.11-rc1_arm-linux-gnueabihf/bin/arm-linux-gnueabihf
#CROSS_COMPILE := /opt/gcc-linaro-7.3.1-2018.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf
#CC := $(CROSS_COMPILE)-gcc


TARGET := v4l2_test_media

SRC_FILE := $(wildcard ./*.c)
SRC_FILE += $(wildcard ./media-v4l2/*.c)
SRC_FILE += $(wildcard ./sensor/imx290/*.c)
SRC_OBJ := $(patsubst %.c,%.o,$(SRC_FILE))

#CFLAGS := -Wall -Werror -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast
CFLAGS := -Wall -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast
CFLAGS += -I./media-v4l2 -I./include -I./sensor -I./sensor/imx290 -I./external -pthread -fPIE
CFLAGS += -L./lib -lispaml -lm
CFLAGS += -fPIC -D_FORTIFY_SOURCE=2 -O3

#CFLAGS += -L./external -lH264VideoStreamer -lx264 -lcrypto -lssl

.PHONY:clean

all: $(TARGET)

$(TARGET): $(SRC_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf ./*.o ./media-v4l2/*.o ./sensor/imx290/*.o $(TARGET)
