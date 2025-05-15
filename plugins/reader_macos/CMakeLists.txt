add_library(serpent_reader_macos MODULE
    MacOSReader.cpp
)

target_include_directories(serpent_reader_macos
    PRIVATE
        ${CMAKE_SOURCE_DIR}/core/include
)

set_target_properties(serpent_reader_macos PROPERTIES 
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
