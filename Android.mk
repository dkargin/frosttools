LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := frosttools

LOCAL_CFLAGS    := -Werror
LOCAL_SRC_FILES += network.cpp
LOCAL_SRC_FILES += connection.cpp

include $(BUILD_STATIC_LIBRARY)
