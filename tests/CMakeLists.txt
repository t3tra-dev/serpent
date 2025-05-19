cmake_minimum_required(VERSION 3.14)
project(SerpentTests)

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG main
)
# For Windows: Prevent overriding /MT and /MD settings (not strictly necessary for macOS/Linux)
set(gtest_force_shared_crt ON CACHE BOOL \"\" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Test executable for ObjectGraph build tests
add_executable(ObjectGraph_build_test
    domain/ObjectGraph_build_test.cpp
    # Add other test source files here if they are part of this target
)

# Link against GoogleTest and our libraries
target_link_libraries(ObjectGraph_build_test PRIVATE
    gtest_main # Includes GTest and Gmock, and provides main()
    gmock
    serpent_domain
    serpent_core
    ${MSGPACK_LIBRARIES} # From top-level CMakeLists.txt
    # robin-hood is header only and should be available via serpent_domain
)

# Add library search paths
target_link_directories(ObjectGraph_build_test PRIVATE ${MSGPACK_LIBRARY_DIRS}) # From top-level

# Include directories for our test files to find mock headers and library headers
target_include_directories(ObjectGraph_build_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/core/include
    ${MSGPACK_INCLUDE_DIRS} # From top-level
)

# Test executable for Snapshot tests
add_executable(Snapshot_test
    domain/Snapshot_test.cpp
)

target_link_libraries(Snapshot_test PRIVATE
    gtest_main
    gmock
    serpent_domain
    serpent_core
    ${MSGPACK_LIBRARIES} # From top-level
)

# Add library search paths
target_link_directories(Snapshot_test PRIVATE ${MSGPACK_LIBRARY_DIRS}) # From top-level

target_include_directories(Snapshot_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/core/include
    ${MSGPACK_INCLUDE_DIRS} # From top-level
)

# Add test to CTest
include(GoogleTest)
gtest_discover_tests(ObjectGraph_build_test)
gtest_discover_tests(Snapshot_test)

