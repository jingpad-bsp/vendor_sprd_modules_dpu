LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_VENDOR_MODULE = true

#LOCAL_MULTILIB := both
#LOCAL_JAVA_LIBRARIES := telephony-common
#LOCAL_JNI_SHARED_LIBRARIES := libjni_validationtools
#LOCAL_JNI_SHARED_LIBRARIES += libatci

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := MyApp01

LOCAL_CERTIFICATE := platform

LOCAL_PROGUARD_ENABLED := disabled

#LOCAL_STATIC_JAVA_LIBRARIES := com.broadcom.bt
#LOCAL_STATIC_JAVA_LIBRARIES += fpextservicejar
#LOCAL_REQUIRED_MODULES := libCameraVerfication libopencv_java3
#LOCAL_JNI_SHARED_LIBRARIES += libCameraVerfication
#LOCAL_JNI_SHARED_LIBRARIES += libopencv_java3

include $(BUILD_PACKAGE)
