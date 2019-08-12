cmake_minimum_required(VERSION 3.4.1)

include_directories(src/main/cpp/include)
include_directories(src/main/cpp/soundtouch/include)
include_directories(src/main/cpp/soundtouch/source)
#添加ffmpeg库文件
add_library(ffmpeg SHARED IMPORTED)
set_target_properties(ffmpeg
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libffmpeg.so)
add_library(
        GLNative
        SHARED
        src/main/cpp/GLNative.cpp
        src/main/cpp/GLAudio.cpp
        src/main/cpp/GLCallJava.cpp
        src/main/cpp/GLQueue.cpp
        src/main/cpp/GLStatus.cpp
        src/main/cpp/GLFFmpeg.cpp
        src/main/cpp/GLPCMBean.cpp
        src/main/cpp/GLBufferDeque.cpp

        #soundtouch
        src/main/cpp/soundtouch/source/AAFilter.cpp
        src/main/cpp/soundtouch/source/cpu_detect_x86.cpp
        src/main/cpp/soundtouch/source/FIFOSampleBuffer.cpp
        src/main/cpp/soundtouch/source/FIRFilter.cpp
        src/main/cpp/soundtouch/source/InterpolateCubic.cpp
        src/main/cpp/soundtouch/source/InterpolateLinear.cpp
        src/main/cpp/soundtouch/source/InterpolateShannon.cpp
        src/main/cpp/soundtouch/source/PeakFinder.cpp
        src/main/cpp/soundtouch/source/RateTransposer.cpp
        src/main/cpp/soundtouch/source/SoundTouch.cpp
        src/main/cpp/soundtouch/source/sse_optimized.cpp
        src/main/cpp/soundtouch/source/TDStretch.cpp)

target_link_libraries(
        GLNative
        ffmpeg
        OpenSLES
        log)