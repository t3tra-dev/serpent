cmake_minimum_required(VERSION 3.15)
project(serpent VERSION 0.1.0 LANGUAGES CXX)

cmake_policy(SET CMP0167 NEW)

# Debug flags
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# For shared libraries of plugins
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Compilation options
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif(MSVC)
    add_compile_options(/W4)
endif()

# --- PkgConfig and External Dependencies ---
find_package(PkgConfig REQUIRED)

# Find msgpack-c using pkg-config
set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/opt/homebrew/lib/pkgconfig:/opt/homebrew/Cellar/msgpack/6.1.0/lib/pkgconfig")
pkg_check_modules(MSGPACK REQUIRED msgpack-c)

# If not found, set manually
if(NOT MSGPACK_FOUND)
    message(STATUS "msgpack-c not found via pkg-config, setting manually")
    set(MSGPACK_INCLUDE_DIRS "/opt/homebrew/Cellar/msgpack/6.1.0/include")
    set(MSGPACK_LIBRARIES "/opt/homebrew/Cellar/msgpack/6.1.0/lib/libmsgpack-c.dylib")
    set(MSGPACK_FOUND TRUE)
endif()

# robin-hood-hashing is header-only, using submodule
add_library(robin-hood INTERFACE)
target_include_directories(robin-hood INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/external/robin-hood-hashing/src/include # Corrected path
)

# External dependencies (submodules)
add_subdirectory(external/zstd/build/cmake) # Corrected path for zstd

# Core abstraction layer library
add_subdirectory(core)

# Plugins
add_subdirectory(plugins)

# Tools
add_subdirectory(tools)

# --- Unit Testing with Google Test ---
# Check if testing is enabled (e.g., by a CMake option or by default)
option(SERPENT_BUILD_TESTS "Build unit tests" ON)

if(SERPENT_BUILD_TESTS)
    enable_testing() # Enables CTest
    add_subdirectory(tests)
endif()
