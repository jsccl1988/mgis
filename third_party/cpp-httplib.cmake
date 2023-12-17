if(TARGET mgis-third_party-cpp-httplib)
    return()
endif()

add_library(
    mgis-third_party-cpp-httplib INTERFACE
)

target_include_directories(
    mgis-third_party-cpp-httplib SYSTEM
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/cpp-httplib
)
