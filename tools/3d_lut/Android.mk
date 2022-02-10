LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	main.cpp \
	PictureData.cpp \
	BufferConsumer.cpp \
	RawDataPicture.cpp \
	three_d_lut.cpp \
	Feeder.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libui \
	libgui \
	liblog \
	libbinder \
	libnativewindow

LOCAL_STATIC_LIBRARIES += libframeout

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libframe
LOCAL_C_INCLUDES += $(GPU_GRALLOC_INCLUDES)


LOCAL_CFLAGS := -DLOG_TAG=\"dpu_test\"
LOCAL_CFLAGS += -DTARGET_GPU_PLATFORM=$(TARGET_GPU_PLATFORM)

LOCAL_MODULE:= 3d_lut

LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)
