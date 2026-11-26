cmake_minimum_required(VERSION 3.16)
project(miniqupa_manual_controller LANGUAGES CXX)
find_package(ARGoS REQUIRED COMPONENTS genericrobot)
set(MINIQUPA_PREFIX "" CACHE PATH "Installation prefix of the MiniQupa plugin")
find_library(MINIQUPA_LIBRARY NAMES argos3plugin_simulator_miniqupa
  PATHS "${MINIQUPA_PREFIX}/lib/argos3" NO_DEFAULT_PATH REQUIRED)
add_library(miniqupa_manual_controller SHARED controller.cpp)
target_compile_features(miniqupa_manual_controller PRIVATE cxx_std_17)
target_include_directories(miniqupa_manual_controller PRIVATE
  "${MINIQUPA_PREFIX}/include" ${ARGOS_INCLUDE_DIRS})
target_link_directories(miniqupa_manual_controller PRIVATE ${ARGOS_LIBRARY_DIR})
target_link_libraries(miniqupa_manual_controller PRIVATE "${MINIQUPA_LIBRARY}"
  argos3core_simulator argos3plugin_simulator_genericrobot ${ARGOS_LDFLAGS})
