# - Find the raspberry pi includes and library
#
# This module defines
#  RASPBERRY_INCLUDE_DIR, where to find raspberry brcm header files
#  RASPBERRY_LIBRARIES, the libraries to link against to use raspberry brcm
#  RASPBERRY_FOUND, If false, do not try to use raspberry brcm.


find_path(RASPBERRY_INCLUDE_DIR bcm_host.h HINTS /opt/vc/include)

set(RPI_HINT "/opt/vc/lib")
find_library(openmaxil_LIBRARY openmaxil HINTS ${RPI_HINT})
find_library(mmal_core_LIBRARY mmal_core HINTS ${RPI_HINT})
find_library(mmal_components_LIBRARY mmal_components HINTS ${RPI_HINT})
find_library(mmal_util_LIBRARY mmal_util HINTS ${RPI_HINT})
find_library(mmal_LIBRARY mmal HINTS ${RPI_HINT})
find_library(openmaxil_LIBRARY openmaxil HINTS ${RPI_HINT})
find_library(bcm_host_LIBRARY bcm_host HINTS ${RPI_HINT})
find_library(vcos_LIBRARY vcos HINTS ${RPI_HINT})
set(RASPBERRY_LIBRARIES ${openmaxil_LIBRARY})
list(APPEND RASPBERRY_LIBRARIES ${bcm_host_LIBRARY})
list(APPEND RASPBERRY_LIBRARIES ${vcos_LIBRARY})
list(APPEND RASPBERRY_LIBRARIES ${mmal_core_LIBRARY})
list(APPEND RASPBERRY_LIBRARIES ${mmal_components_LIBRARY})
list(APPEND RASPBERRY_LIBRARIES ${mmal_util_LIBRARY})
list(APPEND RASPBERRY_LIBRARIES ${mmal_LIBRARY})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Raspberry  DEFAULT_MSG
        RASPBERRY_LIBRARIES RASPBERRY_INCLUDE_DIR)

mark_as_advanced(RASPBERRY_INCLUDE_DIR RASPBERRY_LIBRARIES)