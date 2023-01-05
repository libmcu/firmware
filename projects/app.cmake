# SPDX-License-Identifier: Apache-2.0

include(${BASEDIR}/projects/sources.cmake)

add_library(fpl_app OBJECT ${APP_SRCS})
target_include_directories(fpl_app PUBLIC ${APP_INCS})
target_compile_definitions(fpl_app PUBLIC ${APP_DEFS})
target_compile_options(fpl_app PRIVATE -finstrument-functions)

add_subdirectory(external/pble)
add_subdirectory(external/pwifi)

target_link_libraries(fpl_app pble)
target_link_libraries(fpl_app pwifi)
