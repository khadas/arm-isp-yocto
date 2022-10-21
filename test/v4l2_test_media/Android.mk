LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE:= false
LOCAL_ARM_MODE := arm
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES += $(LOCAL_PATH)/external $(LOCAL_PATH)/include $(LOCAL_PATH)/sensor/

LOCAL_SRC_FILES:= v4l2_test_raw.c media-v4l2/libmediactl.c  media-v4l2/libv4l2subdev.c  media-v4l2/libv4l2videodev.c sensor/imx290/imx290_config.c

LOCAL_CFLAGS += -DANDROID  -Wno-unused-parameter -Wno-unused-function -Wno-missing-field-initializers
LOCAL_STATIC_LIBRARIES :=libcutils
LOCAL_SHARED_LIBRARIES :=libispaml

LOCAL_CFLAGS += -g
LOCAL_CPPFLAGS := -g

LOCAL_MODULE := v4l2_test_raw
include $(BUILD_EXECUTABLE)
