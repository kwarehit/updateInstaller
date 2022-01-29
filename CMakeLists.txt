cmake_minimum_required(VERSION 3.15...3.21)

project(updateInstaller LANGUAGES CXX)

include(CMakePrintHelpers)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "${PROJECT_NAME} be installed to ${CMAKE_INSTALL_PREFIX}")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build Type is ${CMAKE_BUILD_TYPE}")

if(NOT BUILD_SHARED_LIBS)
  set(BUILD_SHARED_LIBS OFF CACHE STRING "Lib type" FORCE)
endif()

message(STATUS "Build Shared is ${BUILD_SHARED_LIBS}")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(STATUS "Target is 64 bits")
else()
  message(STATUS "Target is 32 bits")
endif()

message(STATUS "Architecture detected is ${CMAKE_HOST_SYSTEM_PROCESSOR}")

include(GNUInstallDirs)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

cmake_print_variables(
  CMAKE_ARCHIVE_OUTPUT_DIRECTORY
  CMAKE_LIBRARY_OUTPUT_DIRECTORY
  CMAKE_RUNTIME_OUTPUT_DIRECTORY
)

set(INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR} CACHE PATH "Installation directory for libraries")
set(INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR} CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE PATH "Installation directory for header files")
if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKEDIR cmake)
else()
  set(DEF_INSTALL_CMAKEDIR share/cmake/${PROJECT_NAME})
endif()
set(INSTALL_CMAKEDIR ${DEF_INSTALL_CMAKEDIR} CACHE PATH "Installation directory for CMake files")


foreach(p LIB BIN INCLUDE CMAKE)
  cmake_path(CONVERT ${CMAKE_INSTALL_PREFIX}/${INSTALL_${p}DIR} TO_CMAKE_PATH_LIST _path NORMALIZE)
  message(STATUS "Installing ${p} components to ${_path}")
  unset(_path)
endforeach()

add_subdirectory(third_party)

#set(Boost_USE_STATIC_LIBS        ON)

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} $ENV{BOOST_ROOT}/stage/msvc142/cmake)
cmake_path(CONVERT ${CMAKE_PREFIX_PATH} TO_CMAKE_PATH_LIST CMAKE_PREFIX_PATH NORMALIZE)

find_package(Boost 
  REQUIRED
    filesystem 
    container 
  CONFIG
)

file(GLOB src_files
  ${CMAKE_SOURCE_DIR}/src/*.h  
  ${CMAKE_SOURCE_DIR}/src/*.cpp
)

cmake_print_variables(src_files)

set(UpdaterTarget ${PROJECT_NAME})

add_executable(UpdaterTarget "")

set_target_properties(UpdaterTarget
  PROPERTIES
    OUTPUT_NAME updater
)

target_sources(UpdaterTarget PRIVATE ${src_files})

set(msvc_options)
if(MSVC)
    set(msvc_options "/permissive-" "/W3" "/WX" "/wd4267" "/wd4996")
endif()

target_compile_options(UpdaterTarget 
  PRIVATE 
    ${msvc_options}
)

target_link_libraries(UpdaterTarget 
  PRIVATE 
    Boost::filesystem 
    Boost::container 
    fmt::fmt
)

if(BUILD_SHARED_LIBS)
  install(
    TARGETS 
       UpdaterTarget
       fmt
    ARCHIVE
      DESTINATION ${INSTALL_LIBDIR}
      COMPONENT lib
    RUNTIME
      DESTINATION ${INSTALL_BINDIR}
      COMPONENT bin
    LIBRARY
      DESTINATION ${INSTALL_LIBDIR}
      COMPONENT lib
  )
else()
  install(
    TARGETS 
        UpdaterTarget
    RUNTIME
        DESTINATION ${INSTALL_BINDIR}
        COMPONENT bin
  )
endif()




