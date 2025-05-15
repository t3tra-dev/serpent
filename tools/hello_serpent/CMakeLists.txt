add_executable(hello_serpent main.cpp)

target_link_libraries(hello_serpent 
    PRIVATE serpent_core
)

target_include_directories(hello_serpent
    PRIVATE ${CMAKE_SOURCE_DIR}/core/include
)

# Set the path to search for libraries
if(APPLE)
    set_target_properties(hello_serpent PROPERTIES
        INSTALL_RPATH "@executable_path/../lib"
    )
elseif(UNIX AND NOT APPLE)
    set_target_properties(hello_serpent PROPERTIES
        INSTALL_RPATH "$ORIGIN/../lib"
    )
endif()

install(TARGETS hello_serpent
    RUNTIME DESTINATION bin
)