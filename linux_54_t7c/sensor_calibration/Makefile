TUNING_SUBDIR = $(shell ls -d */)
TUNING_SRC = $(shell find $(TUNING_SUBDIR) -name '*.c')
LIBTUNING_OBJ = $(TUNING_SRC: %.c = %.o)

LOCAL_PATH := $(shell dirname $(lastword $(MAKEFILE_LIST)))

ISP_ROOT_DIR := $(LOCAL_PATH)/..
LOCAL_INCLUDES := -I$(ISP_ROOT_DIR)/include/

CFLAGS += -I ./include/
CFLAGS += -I ./kernel-headers/
CFLAGS += $(LOCAL_INCLUDES)

LIBTUNING = libtuning.so


.PHONY: clean

# rules
all: $(LIBTUNING)

%.o: %.c
	$(CC) -c -fPIC  $(CFLAGS) $^ -o $@

$(LIBTUNING): $(LIBTUNING_OBJ)
	$(CC) -shared  -Wl,-soname,$(LIBTUNING) -fPIC $(CFLAGS) $^ -o $(LIBTUNING)

clean:
	rm -f $(LIBTUNING_OBJ) $(LIBTUNING)

