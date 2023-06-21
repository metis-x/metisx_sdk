cmake_minimum_required (VERSION 3.11)
project(metisx_alveo)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(MetisXHelpers)

add_compile_definitions(_SIM_)

metisx_fetch_external(
  fmt 
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 9.1.0
)

add_executable (${PROJECT_NAME} sim_main.cpp)

add_custom_target(${PROJECT_NAME}_mu ALL
  COMMAND python3 ${CMAKE_SOURCE_DIR}/tool/build.py ${CMAKE_SOURCE_DIR}/mu
)

target_compile_options(${PROJECT_NAME} PRIVATE -g -rdynamic)

add_subdirectory(app/echo_test)
add_subdirectory(app/sort_test)

link_metisx_lib()

# Link App
target_link_libraries(${PROJECT_NAME} PRIVATE app_echo_test)
target_link_libraries(${PROJECT_NAME} PRIVATE app_sort_test)
