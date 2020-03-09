find_path(OPENEXR_INCLUDE_DIR
    NAMES
        OpenEXR/half.h
    PATHS
        "/usr/include"
        "/usr/local/include"
)

if(OPENEXR_INCLUDE_DIR AND EXISTS "${OPENEXR_INCLUDE_DIR}/OpenEXR/OpenEXRConfig.h")
    file(STRINGS
        ${OPENEXR_INCLUDE_DIR}/OpenEXR/OpenEXRConfig.h
        TMP
        REGEX "#define OPENEXR_VERSION_STRING.*$")
    string(REGEX MATCHALL "[0-9.]+" OPENEXR_VERSION ${TMP})
    mark_as_advanced(OPENEXR_INCLUDE_DIR)
endif()

foreach(OPENEXR_LIB
    Half Iex IexMath Imath IlmImf IlmThread
)
    find_library(OPENEXR_${OPENEXR_LIB}_LIBRARY
            "${OPENEXR_LIB}-2_4"
        HINTS
            "${OPENEXR_LOCATION}"
            "$ENV{OPENEXR_LOCATION}"
            "${OPENEXR_BASE_DIR}"
        PATH_SUFFIXES
            lib/
        DOC
            "OPENEXR's ${OPENEXR_LIB} library path"
    )
    mark_as_advanced(OPENEXR_${OPENEXR_LIB}_LIBRARY)

    if(OPENEXR_${OPENEXR_LIB}_LIBRARY)
        list(APPEND OPENEXR_LIBRARIES ${OPENEXR_${OPENEXR_LIB}_LIBRARY})
    endif()
endforeach(OPENEXR_LIB)

list(APPEND OPENEXR_INCLUDE_DIRS ${OPENEXR_INCLUDE_DIR})
list(APPEND OPENEXR_INCLUDE_DIRS ${OPENEXR_INCLUDE_DIR}/OpenEXR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenEXR
    REQUIRED_VARS
        OPENEXR_INCLUDE_DIRS
        OPENEXR_Half_LIBRARY
    VERSION_VAR
        OPENEXR_VERSION
)
