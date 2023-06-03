
LOCAL_PATH := $(shell pwd)
define module_clean
    make -C $(LOCAL_PATH)/$(1) clean
endef

.PHONY: clean media-v4l2_clean sensor_calibration_clean v4l2_test_media_clean media2videoService_clean lens_clean

clean:
	$(call module_clean,media-v4l2)
	$(call module_clean,sensor_calibration)
	$(call module_clean,v4l2_test_media)
	$(call module_clean,media2videoService)
	$(call module_clean,lens)
	@rm -rf $(LOCAL_PATH)/v4l2_test_media/v4l2_test_raw.o
	@rm -rf $(LOCAL_PATH)/media2videoService/media2videoService.o

media-v4l2_clean:
	$(call module_clean,media-v4l2)

sensor_calibration_clean:
	$(call module_clean,sensor_calibration)

v4l2_test_media_clean:
	$(call module_clean,v4l2_test_media)
	@rm -rf $(LOCAL_PATH)/v4l2_test_media/v4l2_test_raw.o
media2videoService_clean:
	$(call module_clean,media2videoService)
	@rm -rf $(LOCAL_PATH)/media2videoService/media2videoService.o
lens_clean:
	$(call module_clean,lens)