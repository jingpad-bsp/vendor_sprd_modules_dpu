LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := dpu.$(TARGET_BOARD_PLATFORM)

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := liblog libutils libcutils libhardware libui libsync libdl

LOCAL_STATIC_LIBRARIES += libadf

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../external/kernel-headers \
                    $(LOCAL_PATH)/../libmemion \
                    $(TOP)/system/core/libion/kernel-headers \
                    $(LOCAL_PATH)/../


ifeq ($(strip $(SPRD_TARGET_USES_HWC2)),true)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../hwcomposer/v2.x/
else
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../hwcomposer/v1.x/
endif

LOCAL_C_INCLUDES += $(GPU_GRALLOC_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/vendor/sprd/external/drivers/gpu
LOCAL_CFLAGS := -DLOG_TAG=\"DPUModule\"
LOCAL_CFLAGS += -DTARGET_GPU_PLATFORM=$(TARGET_GPU_PLATFORM)

LOCAL_SRC_FILES := DpuModule.cpp
ifeq ($(strip $(TARGET_SUPPORT_ADF_DISPLAY)),true)
LOCAL_CFLAGS += -DUSE_ADF_DISPLAY
LOCAL_C_INCLUDES += $(TOP)/system/core/adf/libadf/include/  \
                    $(TOP)/system/core/adf/libadfhwc/include/
LOCAL_SRC_FILES += DpuDeviceAdf.cpp
else
LOCAL_SRC_FILES += DpuDeviceDrm.cpp
LOCAL_SHARED_LIBRARIES += libdrm
LOCAL_C_INCLUDES += $(TOP)/external/libdrm \
                    $(TOP)/external/libdrm/include/drm
endif
include $(call all-makefiles-under,$(LOCAL_PATH))

LOCAL_SRC_FILES += $(IPCORE_SOURCES)
$(info "ipcores: $(LOCAL_SRC_FILES))

include $(BUILD_SHARED_LIBRARY)
