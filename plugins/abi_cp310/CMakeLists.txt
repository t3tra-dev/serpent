add_library(serpent_abi_cp310 MODULE
    Py310ABI.cpp
)

target_include_directories(serpent_abi_cp310
    PRIVATE
        ${CMAKE_SOURCE_DIR}/core/include
)

set_target_properties(serpent_abi_cp310 PROPERTIES 
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)