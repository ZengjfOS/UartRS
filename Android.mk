LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := uartRs 
#LOCAL_STATIC_LIBRARIES :=libpthread
#LOCAL_SHARED_LIBRARIES :=libpthread
LOCAL_STATIC_LIBRARIES :=libpthread
LOCAL_SRC_FILES := uartRS.c
#LOCAL_LDLIBS    += -lpthread 

include $(BUILD_EXECUTABLE)
