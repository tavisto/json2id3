cmake_minimum_required(VERSION 2.6)

project(json2id3)

set(VERSION 0.1.0)
set(AUTHOR "Tavis AItken <tavisto@tavisto.net>") 
set(PACKAGE_DESCRIPTION "Utility to embed id3 tags into media from a json config file")


set(CPACK_PACKAGE_CONTACT ${AUTHOR})
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_PACKAGE_DESCRIPTION ${PACKAGE_DESCRIPTION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PACKAGE_DESCRIPTION})
set(CPACK_DEBIAN_PACKAGE_SECTION "Development")
include(CPack)

set(CMAKE_CXX_FLAGS "-g -Wall")

#enable_testing()


set(WITH_TAGLIB ON CACHE BOOL "build with TagLib")
set(WITH_LIBJSON ON CACHE BOOL "build with libjson")
include (CheckCXXSourceCompiles)
### Check for taglib
set(TAGLIB_LIBRARIES)
set(TAGLIB_CFLAGS)
if(WITH_TAGLIB)
  if (WIN32)
  find_library(TAGLIB_LIBRARIES NAMES tag)
  find_path(TAGLIB_INCLUDE_DIR taglib.h PATHS $ENV{INCLUDE}/taglib)
  if (TAGLIB_LIBRARIES AND TAGLIB_INCLUDE_DIR)
    set(TAGLIB_CFLAGS -I${TAGLIB_INCLUDE_DIR} -DTAGLIB_STATIC)
    set(HAVE_TAGLIB 1)
  endif (TAGLIB_LIBRARIES AND TAGLIB_INCLUDE_DIR)
  else (WIN32)
  find_program(TAGLIBCONFIG_EXECUTABLE NAMES taglib-config PATHS /usr/bin /usr/local/bin ${BIN_INSTALL_DIR})
  if(TAGLIBCONFIG_EXECUTABLE)
    exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --libs RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_LIBRARIES)
    exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --cflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_CFLAGS)
    exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --version RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_VERSION)
    if(TAGLIB_LIBRARIES AND TAGLIB_CFLAGS AND TAGLIB_VERSION)
      if(NOT ${TAGLIB_VERSION} VERSION_LESS 1.4)
        set(HAVE_TAGLIB 1)
        message(STATUS "TagLib found: ${TAGLIB_LIBRARIES} ${TAGLIB_CFLAGS}")
      endif(NOT ${TAGLIB_VERSION} VERSION_LESS 1.4)
    endif(TAGLIB_LIBRARIES AND TAGLIB_CFLAGS AND TAGLIB_VERSION)
  endif(TAGLIBCONFIG_EXECUTABLE)
  if (NOT HAVE_TAGLIB)
    message(FATAL_ERROR "Could not find Taglib")
  endif (NOT HAVE_TAGLIB)
  endif (WIN32)
endif(WITH_TAGLIB)
if (TAGLIB_LIBRARIES AND TAGLIB_CFLAGS)
  set(_CMAKE_REQUIRED_LIBRARIES_TMP ${CMAKE_REQUIRED_LIBRARIES})
  set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${TAGLIB_LIBRARIES})
  set(_CMAKE_REQUIRED_DEFINITIONS_TMP ${CMAKE_REQUIRED_DEFINITIONS})
  set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS} ${TAGLIB_CFLAGS})
  CHECK_CXX_SOURCE_COMPILES("#include <mp4file.h>\nint main() {\n  TagLib::MP4::Item item;\n  item.toUInt();\n  return 0;\n}\n" HAVE_TAGLIB_MP4_UINTTYPES)
  set(CMAKE_REQUIRED_LIBRARIES ${_CMAKE_REQUIRED_LIBRARIES_TMP})
  set(CMAKE_REQUIRED_DEFINITIONS ${_CMAKE_REQUIRED_DEFINITIONS_TMP})
endif (TAGLIB_LIBRARIES AND TAGLIB_CFLAGS)

set(JSON_LIBRARIES)
if(WITH_LIBJSON)
    find_library(LIBJSON_LIBRARIES NAMES tag
        PATHS
        /usr/lib
        /usr/local/lib
        /home/taitken/cpp/libjson/src/
        )
    find_path(LIBJSON_INCLUDE_DIR json.h PATHS 
        /usr/include/json
        /usr/local/include/json
        $ENV{INCLUDE}/json
        )

  if (LIBJSON_LIBRARIES AND LIBJSON_INCLUDE_DIR)
      set(LIBJSON_CFLAGS -I${LIBJSON_INCLUDE_DIR})
      message(STATUS "libjson found: ${LIBJSON_LIBRARIES} ${LIBJSON_CFLAGE}")
      set(HAVE_LIBJSON 1)
      set(LIBJSON_FOUND TRUE)
  else (LIBJSON_LIBRARIES AND LIBJSON_INCLUDE_DIR)
      message(STATUS "Could not find jsonlib")
  endif (LIBJSON_LIBRARIES AND LIBJSON_INCLUDE_DIR)
endif(WITH_LIBJSON)


add_definitions(${TAGLIB_CFLAGS} ${LIBJSON_CFLAGS})


#add_subdirectory(doc)
#add_subdirectory(example)
#add_subdirectory(include)
add_subdirectory(src)
#add_subdirectory(test)
