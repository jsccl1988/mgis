if(TARGET mgis-third_party-boost)
    return()
endif()

add_library(
    mgis-third_party-boost INTERFACE
)

target_include_directories(
    mgis-third_party-boost SYSTEM
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/boost/include
)

set_target_properties(
    mgis-third_party-boost
    PROPERTIES
        INTERFACE_MAPBOX_NAME "Boost C++ Libraries"
        INTERFACE_MAPBOX_URL "https://www.boost.org"
        INTERFACE_MAPBOX_AUTHOR "Boost authors"
        INTERFACE_MAPBOX_LICENSE ${CMAKE_CURRENT_LIST_DIR}/boost/LICENSE_1_0.txt
)
