MEDIA_API_LOCAL_PATH := $(shell dirname $(lastword $(MAKEFILE_LIST)))
MEDIA_API_SRC = $(shell find $(MEDIA_API_LOCAL_PATH) -name '*.c')
MEDIA_API_OBJ = $(MEDIA_API_SRC: %.c = %.o)


ISP_ROOT_DIR := $(MEDIA_API_LOCAL_PATH)/..
LOCAL_INCLUDES := -I$(ISP_ROOT_DIR)/include/

CFLAGS += -I ./include/
CFLAGS += -I ./kernel-headers/
CFLAGS += $(LOCAL_INCLUDES)

LIBMEDIA_API = libmediaAPI.so


.PHONY: clean

# rules
all: $(LIBMEDIA_API)

%.o: %.c
	$(CC) -c -fPIC  $(CFLAGS) $^ -o $@

$(LIBMEDIA_API): $(MEDIA_API_OBJ)
	$(CC) -shared  -Wl,-soname,$(LIBMEDIA_API) -fPIC $(CFLAGS) $^ -o $(LIBMEDIA_API)

clean:
	rm -f $(MEDIA_API_OBJ) $(LIBMEDIA_API)

