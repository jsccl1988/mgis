if(TARGET mgis-third_party-polylabel)
    return()
endif()

add_library(
    mgis-third_party-polylabel INTERFACE
)

target_include_directories(
    mgis-third_party-polylabel SYSTEM
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/polylabel/include
)

set_target_properties(
    mgis-third_party-polylabel
    PROPERTIES
        INTERFACE_MAPBOX_NAME "polylabel"
        INTERFACE_MAPBOX_URL "https://github.com/mapbox/polylabel"
        INTERFACE_MAPBOX_AUTHOR "Mapbox"
        INTERFACE_MAPBOX_LICENSE ${CMAKE_CURRENT_LIST_DIR}/polylabel/LICENSE
)
