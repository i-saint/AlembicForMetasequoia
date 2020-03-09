# * ALEMBIC_INCLUDE_DIR
# * ALEMBIC_LIBRARY

find_path(ALEMBIC_INCLUDE_DIR
    NAMES
        "Alembic/Abc/All.h"
    PATHS
        "/usr/include"
        "/usr/local/include"
)
mark_as_advanced(ALEMBIC_INCLUDE_DIR)

find_file(ALEMBIC_LIBRARY libAlembic.a PATH_SUFFIXES lib)
mark_as_advanced(ALEMBIC_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("ALEMBIC"
    DEFAULT_MSG
    ALEMBIC_INCLUDE_DIR
	ALEMBIC_LIBRARY
)
