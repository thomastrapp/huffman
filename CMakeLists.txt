#### HUFFMAN

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7 FATAL_ERROR)
INCLUDE(ExternalProject) # available since 2.8.7
#SET(CMAKE_VERBOSE_MAKEFILE on)

PROJECT(huffman)

ADD_DEFINITIONS("-std=c++11")

ADD_EXECUTABLE(huffman "${PROJECT_SOURCE_DIR}/src/main.cpp")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/src")
TARGET_LINK_LIBRARIES(huffman boost_program_options)


###### EXTERNAL DEPENDENCIES ##############################
# download and build my datas-and-algos project
# header files only, no libraries
SET_DIRECTORY_PROPERTIES(PROPERTIES EP_PREFIX ${CMAKE_BINARY_DIR}/thirdparty)
ExternalProject_Add(
    datas-and-algos
    GIT_REPOSITORY https://github.com/thomastrapp/datas-and-algos.git
    TIMEOUT 10

    UPDATE_COMMAND ""
    INSTALL_COMMAND ""

    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON)

# use datas-and-algo's src/ directory as an additional include directory
ExternalProject_Get_Property(datas-and-algos source_dir)
INCLUDE_DIRECTORIES("${source_dir}/src")

ADD_DEPENDENCIES(huffman datas-and-algos)
###########################################################


###### WARNINGS ###########################################
# show warnings in debug mode
IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
  # clang
  IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    ADD_DEFINITIONS("-Weverything")
  ENDIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

  # g++
  IF(CMAKE_COMPILER_IS_GNUCXX)
    ADD_DEFINITIONS("-Wconversion" "-Wextra" "-pedantic" "-Wall" "-Wextra"
      "-Wcast-align" "-Wcast-qual" "-Wctor-dtor-privacy"
      "-Wdisabled-optimization" "-Wformat=2" "-Winit-self" "-Wlogical-op"
      "-Wmissing-include-dirs" "-Woverloaded-virtual" "-Wredundant-decls"
      "-Wshadow" "-Wsign-promo" "-Wstrict-null-sentinel" "-Wstrict-overflow=5"
      "-Wswitch-default" "-Wundef" "-Wno-unused" "-Weffc++")
  ENDIF(CMAKE_COMPILER_IS_GNUCXX)
ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")
###########################################################

###### BUILD OPTIONS ######################################
OPTION(TCMALLOC "Link with libtcmalloc" OFF)
IF(TCMALLOC)
  TARGET_LINK_LIBRARIES(huffman tcmalloc)

  # -fno-omit-frame-pointer must be set if you configured gperftools with
  # --enable-frame-pointers
  #ADD_DEFINITIONS("-fno-omit-frame-pointer")

  # force gcc to use tcmalloc
  IF(CMAKE_COMPILER_IS_GNUCXX)
    ADD_DEFINITIONS(
      "-fno-builtin-malloc" "-fno-builtin-calloc"
      "-fno-builtin-realloc" "-fno-builtin-free")
  ENDIF(CMAKE_COMPILER_IS_GNUCXX)
ENDIF(TCMALLOC)

# force debug symbols (e.g. cmake -DCMAKE_BUILD_TYPE=Release -DSYMBOLS=ON)
OPTION(SYMBOLS "Add debugging symbols" OFF)
IF(SYMBOLS)
  ADD_DEFINITIONS("-g")
ENDIF(SYMBOLS)

# enable whole-program optimization on gcc
OPTION(WPO  "Use whole-program optimization on gcc" OFF)
IF(WPO)
  IF(CMAKE_COMPILER_IS_GNUCXX)
    ADD_DEFINITIONS("-fwhole-program")
    SET(CMAKE_EXE_LINKER_FLAGS "-fwhole-program")
  ELSE()
    MESSAGE(STATUS "compiler not g++, cannot use -fwhole-program")
  ENDIF()
ENDIF(WPO)

# gprof build
OPTION(GPROF "gprof build" OFF)
IF(GPROF)
  # -pg must be set when compiling and linking
  ADD_DEFINITIONS("-pg")
  SET(CMAKE_EXE_LINKER_FLAGS "-pg")
ENDIF(GPROF)
###########################################################

