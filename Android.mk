LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := SoundControl

LOCAL_CFLAGS    := -Werror
LOCAL_SRC_FILES += SoundControl.cpp
LOCAL_SRC_FILES += connection.cpp

LOCAL_SRC_FILES += OpenSLES-audio.cpp
LOCAL_LDLIBS    := -llog


include $(BUILD_SHARED_LIBRARY)
