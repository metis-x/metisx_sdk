cmake_minimum_required (VERSION 3.11)

project (app_echo_test)



add_library (${PROJECT_NAME} STATIC
    echo_test.cpp
)


target_include_directories(${PROJECT_NAME} PUBLIC .)

link_metisx_lib()