LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:=  \
	devmcpy.c  \
	devmem.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../external/kernel-headers
LOCAL_CFLAGS := -DLOG_TAG=\"devmem\"
LOCAL_MODULE:= devcpy 
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:=  \
	devmd.c    \
	devmem.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../external/kernel-headers
LOCAL_CFLAGS := -DLOG_TAG=\"devmem\"
LOCAL_MODULE:= devmd 
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:=  \
	devmem.c   \
	devmload.c \

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../external/kernel-headers
LOCAL_CFLAGS := -DLOG_TAG=\"devmem\"
LOCAL_MODULE:= devload 
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:=  \
	devmem.c   \
	devmsave.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../external/kernel-headers
LOCAL_CFLAGS := -DLOG_TAG=\"devmem\"
LOCAL_MODULE:= devsave 
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=  \
	devmem.c   \
	devmset.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../external/kernel-headers
LOCAL_CFLAGS := -DLOG_TAG=\"devmem\"
LOCAL_MODULE:= devmset 
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)
