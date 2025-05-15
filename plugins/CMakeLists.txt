add_subdirectory(abi_cp310)

# Build Reader plugin for each OS
if(UNIX AND NOT APPLE)
    add_subdirectory(reader_linux)
elseif(APPLE)
    add_subdirectory(reader_macos)
elseif(WIN32)
    # For Windows, to be added in the future
    # add_subdirectory(reader_windows)
endif()
