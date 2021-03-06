LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)
LOCAL_MODULE := libssl
LOCAL_SRC_FILES := $(LOCAL_PATH)/src/libs/armeabi-v7a/libssl_armeabi-v7a.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcrypto
LOCAL_SRC_FILES := $(LOCAL_PATH)/src/libs/armeabi-v7a/libcrypto_armeabi-v7a.a
include $(PREBUILT_STATIC_LIBRARY)



include $(CLEAR_VARS)
LOCAL_MODULE    := onvif
LOCAL_CFLAGS :=-fno-rtti -fexceptions -DWITH_OPENSSL -DWITH_DOM

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/src/include \
	$(LOCAL_PATH)
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)



LOCAL_SRC_FILES :=src/dom.cpp		\
        src/mecevp.cpp	\
        src/smdevp.cpp	\
        src/soapC.cpp	\
        src/soapClient.cpp\
        src/stdsoap2.cpp\
        src/threads.cpp	\
        src/wsaapi.cpp	\
        src/wsdd.cpp	\
        src/wsseapi.cpp \
        
# LOCAL_LDLIBS    := $(LOCAL_PATH)/src/libs/armeabi-v7a/libcrypto_1_1.so
# LOCAL_LDLIBS    += $(LOCAL_PATH)/src/libs/armeabi-v7a/libssl_1_1.so

# LOCAL_LDLIBS := $(LOCAL_PATH)/src/libs/armeabi-v7a/libssl_armeabi-v7a.a
# LOCAL_LDLIBS += $(LOCAL_PATH)/src/libs/armeabi-v7a/libcrypto_armeabi-v7a.a
LOCAL_LDLIBS += -lm -llog


# LOCAL_LDFLAGS := $(LOCAL_PATH)/src/libs/armeabi-v7a/libcrypto_armeabi-v7a.a
# LOCAL_LDFLAGS := $(LOCAL_PATH)/src/libs/armeabi-v7a/libssl_armeabi-v7a.a

LOCAL_CXXFLAGS := -fexceptions -Wno-write-strings
LOCAL_CPPFLAGS :=  -O2 -fexceptions -DWITH_OPENSSL -DWITH_DOM 
# LOCAL_CPPFLAGS := -O0 -g -fexceptions -DWITH_OPENSSL -DWITH_DOM
LOCAL_MODULE_TAGS := optional  

LOCAL_STATIC_LIBRARIES := libssl
LOCAL_STATIC_LIBRARIES += libcrypto

include $(BUILD_SHARED_LIBRARY)
