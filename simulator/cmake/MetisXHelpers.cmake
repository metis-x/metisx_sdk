include(FetchContent)

function(metisx_fetch_external name)
  FetchContent_Declare(
    ${name}
    ${ARGN}
  )
  FetchContent_MakeAvailable(${name})
endfunction()

function(link_metisx_lib)
  set(include_dir_list
    ${CMAKE_SOURCE_DIR}/include/sys
    ${CMAKE_SOURCE_DIR}/include/metisx_api
    ${CMAKE_SOURCE_DIR}/include/alveo_sim
  )

  set(lib_list
    sim
    mu
    micro
    host
  )

  target_include_directories(${PROJECT_NAME} PRIVATE ${include_dir_list})
  target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/lib/)
  target_link_libraries(${PROJECT_NAME} PRIVATE ${lib_list} -lfmt)
endfunction()