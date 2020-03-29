cmake_minimum_required(VERSION 3.0.0)
project(itunes-visualizer VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
include(ExternalProject)

enable_testing()

# sidenote: can include static libs without `add_library`
# just add it thats all you do

add_library(timing src/timing/timing.cpp)
add_library(DFT src/audiohandlers/DFT.cpp)
add_library(vorbis src/audiohandlers/stb_vorbis.c
                   src/audiohandlers/VorbisManager.cpp)


target_include_directories(timing PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(DFT PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(vorbis PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include 
                                          ${CMAKE_CURRENT_SOURCE_DIR}/deps/portaudio/include)

set_target_properties(vorbis 
                      PROPERTIES LINKER_LANGUAGE CXX)

# add_executable(itunes-visualizer src/main.cpp)

### BUILD TESTS ###
# from gtest docs

configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
# if (CMAKE_VERSION VERSION_LESS 2.8.11)
#   include_directories("${gtest_SOURCE_DIR}/include")
# endif()

# Now simply link against gtest or gtest_main as needed. Eg
set(test_dir ${CMAKE_CURRENT_SOURCE_DIR}/tests)

FILE(GLOB TESTFILES ${test_dir}/*test.cpp)

set(timingtest_deps timing)
set(DFTtest_deps DFT)
set(SPSCtest_deps )
set(SPSCPerChanneltest_deps )

foreach(TESTFILE ${TESTFILES})
  get_filename_component(TESTNAME ${TESTFILE} NAME_WE)
  add_executable(${TESTNAME} ${TESTFILE})
  target_include_directories(${TESTNAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_link_libraries(${TESTNAME} gtest_main ${${TESTNAME}_deps})
  add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
endforeach(TESTFILE)

set(exp_dir ${CMAKE_CURRENT_SOURCE_DIR}/experiments)

set(deps_dir ${CMAKE_CURRENT_SOURCE_DIR}/deps)

add_subdirectory(${deps_dir}/portaudio)

add_executable(portaudio_test ${exp_dir}/portaudioTest.cpp
                              ${CMAKE_CURRENT_SOURCE_DIR}/src/audiohandlers/stb_vorbis.c)
target_include_directories(portaudio_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(portaudio_test portaudio DFT)

target_link_libraries(vorbis portaudio)

### FINAL CONFIGS ###

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
