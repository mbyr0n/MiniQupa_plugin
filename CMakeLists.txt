cmake_minimum_required(VERSION 3.16)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  project(argos3_miniqupa LANGUAGES CXX)
endif()

# Works both inside the ARGoS source tree and as an external plugin build.
if(NOT TARGET argos3core_simulator AND NOT ARGOS_FOUND)
  find_package(ARGoS REQUIRED COMPONENTS entities genericrobot dynamics2d media OPTIONAL_COMPONENTS qtopengl)
endif()
if(DEFINED ARGOS_BUILD_FOR AND NOT ARGOS_BUILD_FOR STREQUAL "simulator")
  message(FATAL_ERROR "MiniQupa currently implements the simulator target only")
endif()
find_package(Python3 REQUIRED COMPONENTS Interpreter)
option(MINIQUPA_WITH_QTOPENGL "Build MiniQupa graphical visualization when available" ON)
set(MINIQUPA_TARGET argos3plugin_simulator_miniqupa)

set(MINIQUPA_CI
  control_interface/ci_miniqupa_ir_sensor.cpp
  control_interface/ci_miniqupa_camera_sensor.cpp
  control_interface/ci_miniqupa_floor_color_sensor.cpp
  control_interface/ci_miniqupa_encoder_sensor.cpp)
set(MINIQUPA_SIMULATOR
  simulator/miniqupa_entity.cpp
  simulator/dynamics2d_miniqupa.cpp
  simulator/miniqupa_ir_default_sensor.cpp
  simulator/miniqupa_camera_default_sensor.cpp
  simulator/miniqupa_floor_color_default_sensor.cpp
  simulator/miniqupa_encoder_default_sensor.cpp
  simulator/miniqupa_diffuser_default_actuator.cpp
  simulator/miniqupa_model_path.cpp)

set(MINIQUPA_GENERATED)
foreach(kind IN ITEMS ir floor_color tag)
  set(header "${CMAKE_CURRENT_BINARY_DIR}/generated/miniqupa_${kind}_geometry.h")
  add_custom_command(OUTPUT "${header}"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_${kind}_geometry.py"
      "${CMAKE_CURRENT_SOURCE_DIR}/config/miniqupa_geometry.json" "${header}"
    DEPENDS "cmake/generate_${kind}_geometry.py" config/miniqupa_geometry.json VERBATIM)
  list(APPEND MINIQUPA_GENERATED "${header}")
endforeach()

add_library(${MINIQUPA_TARGET} SHARED ${MINIQUPA_CI} ${MINIQUPA_SIMULATOR} ${MINIQUPA_GENERATED})
target_compile_features(${MINIQUPA_TARGET} PUBLIC cxx_std_17)
target_compile_options(${MINIQUPA_TARGET} PRIVATE -Wall -Wextra -Wpedantic)
# Standard <argos3/plugins/robots/miniqupa/...> includes also work externally.
set(MINIQUPA_BUILD_INCLUDE "${CMAKE_CURRENT_BINARY_DIR}/include")
file(MAKE_DIRECTORY "${MINIQUPA_BUILD_INCLUDE}/argos3/plugins/robots")
file(CREATE_LINK "${CMAKE_CURRENT_SOURCE_DIR}" "${MINIQUPA_BUILD_INCLUDE}/argos3/plugins/robots/miniqupa" SYMBOLIC)
target_include_directories(${MINIQUPA_TARGET} PUBLIC
  $<BUILD_INTERFACE:${MINIQUPA_BUILD_INCLUDE}> $<INSTALL_INTERFACE:include>)
if(ARGOS_INCLUDE_DIRS)
  target_include_directories(${MINIQUPA_TARGET} SYSTEM PUBLIC ${ARGOS_INCLUDE_DIRS})
endif()
if(ARGOS_LIBRARY_DIR)
  target_link_directories(${MINIQUPA_TARGET} PUBLIC "${ARGOS_LIBRARY_DIR}")
endif()
target_include_directories(${MINIQUPA_TARGET} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_compile_definitions(${MINIQUPA_TARGET} PRIVATE
  MINIQUPA_SOURCE_MODEL_DIR="${CMAKE_CURRENT_SOURCE_DIR}/simulator/models")
target_link_libraries(${MINIQUPA_TARGET} PUBLIC argos3core_simulator argos3plugin_simulator_genericrobot
  PRIVATE argos3plugin_simulator_entities argos3plugin_simulator_media
    argos3plugin_simulator_dynamics2d ${ARGOS_LDFLAGS} ${CMAKE_DL_LIBS})
if(MINIQUPA_WITH_QTOPENGL AND ARGOS_QTOPENGL_FOUND AND (TARGET argos3plugin_simulator_qtopengl OR ARGOS_QTOPENGL_LIBRARY))
  target_sources(${MINIQUPA_TARGET} PRIVATE simulator/qtopengl_miniqupa.cpp)
  target_include_directories(${MINIQUPA_TARGET} PRIVATE ${ARGOS_QTOPENGL_INCLUDE_DIRS})
  target_link_libraries(${MINIQUPA_TARGET} PRIVATE argos3plugin_simulator_qtopengl ${ARGOS_QTOPENGL_LIBRARIES})
endif()

set(ARGOS_PLUGIN_PATH "${ARGOS_PLUGIN_PATH}:${CMAKE_CURRENT_BINARY_DIR}" CACHE INTERNAL "ARGoS plugin path")
install(TARGETS ${MINIQUPA_TARGET} LIBRARY DESTINATION lib/argos3)
install(DIRECTORY control_interface/ DESTINATION include/argos3/plugins/robots/miniqupa/control_interface FILES_MATCHING PATTERN "*.h")
install(DIRECTORY simulator/ DESTINATION include/argos3/plugins/robots/miniqupa/simulator FILES_MATCHING PATTERN "*.h" PATTERN "models" EXCLUDE)
install(DIRECTORY simulator/models/ DESTINATION share/argos3/robots/miniqupa/models)
