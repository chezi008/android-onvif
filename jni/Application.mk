#----------------------------------------------------
# Author: momo0853@live.com
# Time  : 2016年 04月 20日 星期三 17:43:07 CST
#----------------------------------------------------

# This is an automatically generated file, in order to reduce repetitive work.
# Android.mk: For more details, please see "http://developer.android.com/ndk/guides/android_mk.html".
# Application.mk: For more details, please see "http://developer.android.com/ndk/guides/application_mk.html".
# Can identify the suffix for .c .cc .cpp and .a .so.

NDK_TOOLCHAIN_VERSION :=#(4.6 4.8 4.9)
APP_BUILD_SCRIPT := Android.mk
APP_ABI          := armeabi-v7a#(32_bit(armeabi armeabi-v7a x86 mips), 64_bit(arm64-v8a x86_64 mips64))
APP_PLATFORM     := android-21# "5.0" (3~21)
APP_STL          := c++_static#(system stlport_static stlport_shared gnustl_static gnustl_shared \
                                   gabi++_static gabi++_shared c++_static c++_shared)
APP_OPTIM        := release#(release debug)
APP_PIE          := true

APP_MODULES    :=onvif
APP_CFLAGS     :=
APP_CPPFLAGS   := -fexceptions -frtti #允许异常功能，及运行时类型识别  
APP_CPPFLAGS +=-std=c++11 #允许使用c++11的函数等功能  
APP_CPPFLAGS +=-fpermissive  #此项有效时表示宽松的编译形式，比如没有用到的代码中有错误也可以通过编
APP_ASMFLAGS   :=
APP_CONLYFLAGS :=
APP_SHORT_COMMANDS :=
APP_THIN_ARCHIVE   := false #(false true)   merit :generate a thin archive; \
                                          drawback:such libraries cannot be moved to a different location
APP_PROJECT_PATH   :=
APP_LDFLAGS :=

