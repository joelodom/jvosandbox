LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_LDLIBS := -llog
LOCAL_MODULE    := KMLShim
LOCAL_SRC_FILES := shim.cc
include $(BUILD_SHARED_LIBRARY)