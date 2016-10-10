LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    apply-permissions.c

LOCAL_C_INCLUDES += system/core/include/private/

LOCAL_SHARED_LIBRARIES :=

LOCAL_CFLAGS := -std=c99

LOCAL_MODULE:= apply-permissions


include $(BUILD_EXECUTABLE)
