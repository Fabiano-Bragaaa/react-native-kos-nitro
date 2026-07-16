project(KosNitro)
cmake_minimum_required(VERSION 3.9.0)

set (PACKAGE_NAME KosNitro)
set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_STANDARD 20)

# Enable Raw Props parsing in react-native (for Nitro Views)
add_compile_options(-DRN_SERIALIZABLE_STATE=1)

# Define C++ library and add all sources
add_library(${PACKAGE_NAME} SHARED 
	src/main/cpp/cpp-adapter.cpp
	../cpp/HybridKosNitro.cpp
        
)

# Add Nitrogen specs :)
include(${CMAKE_SOURCE_DIR}/../nitrogen/generated/android/KosNitro+autolinking.cmake)

# Set up local includes
include_directories(
        "src/main/cpp"
        "../cpp"
        "../rust/kos-nitro/include"
)

# Import the prebuilt kos-nitro Rust static library
add_library(kos_nitro STATIC IMPORTED)
set_target_properties(kos_nitro PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libkos_nitro.a"
)

find_library(LOG_LIB log)

# Link all libraries together
target_link_libraries(
        ${PACKAGE_NAME}
        kos_nitro
        ${LOG_LIB}
        android                                   # <-- Android core
        dl
        m
)
