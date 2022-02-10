LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	main.cpp

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

ifeq ($(strip $(TARGET_GPU_PLATFORM)),midgard)
LOCAL_CPPFLAGS += -DGRALLOC_MIDGARD
else ifeq ($(strip $(TARGET_GPU_PLATFORM)),utgard)
LOCAL_CPPFLAGS += -DGRALLOC_UTGARD
else ifeq ($(strip $(TARGET_GPU_PLATFORM)),soft)
LOCAL_CPPFLAGS += -DGRALLOC_MIDGARD
endif

LOCAL_CFLAGS := -DLOG_TAG=\"sprd_cabc\"
LOCAL_CFLAGS += -DTARGET_GPU_PLATFORM=$(TARGET_GPU_PLATFORM)

LOCAL_MODULE:= test-dpu-cabc

LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)
