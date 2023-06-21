cmake_minimum_required (VERSION 3.11)

project (app_sort_test)



add_library (${PROJECT_NAME} STATIC
    sort_test.cpp
)


target_include_directories(${PROJECT_NAME} PUBLIC .)

link_metisx_lib()