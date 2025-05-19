find_package(Boost REQUIRED)

add_library(serpent_core SHARED
    src/ReaderFactory.cpp
    src/AbiFactory.cpp
)

target_include_directories(serpent_core 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${Boost_INCLUDE_DIRS}
)

# Link with libdl on Linux since dlopen is used
if(UNIX AND NOT APPLE)
    target_link_libraries(serpent_core PRIVATE dl)
elseif(APPLE)
    target_link_libraries(serpent_core PRIVATE dl)
endif()

# Include headers from subdirectories
target_include_directories(serpent_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Serpent Domain Library
add_library(serpent_domain STATIC
    src/domain/ObjectGraph.cpp
    src/domain/Snapshot.cpp
    src/domain/SnapshotManager.cpp
    src/domain/DiffEngine.cpp
    # Add other .cpp files ...
)

target_include_directories(serpent_domain PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${MSGPACK_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(serpent_domain PUBLIC
    serpent_core
    robin-hood
    ${MSGPACK_LIBRARIES}
    Boost::boost # Link with Boost
    libzstd_static # Corrected zstd target name
)

# Installation settings (optional)
install(TARGETS serpent_core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
