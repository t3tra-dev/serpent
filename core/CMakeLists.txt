add_library(serpent_core SHARED
    src/ReaderFactory.cpp
    src/AbiFactory.cpp
)

target_include_directories(serpent_core 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
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

# Installation settings (optional)
install(TARGETS serpent_core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
