if(TARGET mgis-third_party-protozero)
    return()
endif()

add_library(
    mgis-third_party-protozero INTERFACE
)

target_include_directories(
    mgis-third_party-protozero SYSTEM
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/protozero/include
)

set_target_properties(
    mgis-third_party-protozero
    PROPERTIES
        INTERFACE_MAPBOX_NAME "protozero"
        INTERFACE_MAPBOX_URL "https://github.com/mapbox/protozero"
        INTERFACE_MAPBOX_AUTHOR "Mapbox"
        INTERFACE_MAPBOX_LICENSE ${CMAKE_CURRENT_LIST_DIR}/protozero/LICENSE.md
)
