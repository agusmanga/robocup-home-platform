cmake_minimum_required(VERSION 3.8)
project(robocup_hardware)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(gz_ros2_control REQUIRED)
find_package(ignition-gazebo6 REQUIRED)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()


add_library(
  gazebo_hardware
  SHARED
  src/gazebo-hardware.cpp
)

target_include_directories(
  gazebo_hardware
  PUBLIC
  include
)


ament_target_dependencies(
  gazebo_hardware
  rclcpp
  hardware_interface
  pluginlib
  gz_ros2_control
)
target_link_libraries(
  gazebo_hardware
  ignition-gazebo6::core
)

pluginlib_export_plugin_description_file(
  gz_ros2_control
  gazebo_hardware_plugin.xml
)

install(
  TARGETS gazebo_hardware
  DESTINATION lib
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  FILES gazebo_hardware_plugin.xml
  DESTINATION share/${PROJECT_NAME}
)


ament_package()
