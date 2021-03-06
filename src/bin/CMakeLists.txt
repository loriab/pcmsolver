# Executables for testing separate from Boost unit test framework or for profiling purposes
# The executables will be created in ${PROJECT_BINARY_DIR}/src/bin

#add_executable(debug_wavcav debug_wavcav.cpp)
#target_link_libraries(debug_wavcav solver cavity wavcav pwl wem utils)

#add_executable(plot_green_spherical plot_green_spherical.cpp)
#target_link_libraries(plot_green_spherical utils ${external_libraries})

#add_executable(plot_green_spherical-CASE1 plot_green_spherical-CASE1.cpp)
#target_link_libraries(plot_green_spherical-CASE1 utils ${external_libraries})
#add_executable(plot_green_spherical-CASE2 plot_green_spherical-CASE2.cpp)
#target_link_libraries(plot_green_spherical-CASE2 utils ${external_libraries})
#add_executable(plot_green_spherical-CASE3 plot_green_spherical-CASE3.cpp)
#target_link_libraries(plot_green_spherical-CASE3 utils ${external_libraries})
#add_executable(plot_green_spherical-CASE4 plot_green_spherical-CASE4.cpp)
#target_link_libraries(plot_green_spherical-CASE4 utils ${external_libraries})

#add_executable(check_Coulomb_coefficient check_Coulomb_coefficient.cpp)
#target_link_libraries(check_Coulomb_coefficient utils ${external_libraries})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
link_directories(${PROJECT_BINARY_DIR}/lib)
add_executable(run_pcm run_pcm.cpp)
if(BUILD_CUSTOM_BOOST)
  add_dependencies(run_pcm custom_boost)
endif()
if(STATIC_LIBRARY_ONLY)
  target_link_libraries(run_pcm pcm-static ${ZLIB_LIBRARIES})
else()
  target_link_libraries(run_pcm pcm-shared)
endif()
unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
