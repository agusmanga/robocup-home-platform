cmake_minimum_required(VERSION 3.8)
project(robocup_hardware)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Dependencias
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(gz_ros2_control REQUIRED)
find_package(ignition-gazebo6 REQUIRED)


add_library(gazebo_hardware SHARED
  src/gazebo-hardware.cpp
)

add_library(odrive_ros2_control_plugin SHARED
  src/odrive_hardware_interface.cpp
  odrive_base/src/epoll_event_loop.cpp
  odrive_base/src/socket_can.cpp
)

target_include_directories(gazebo_hardware PUBLIC
  include
)

target_include_directories(odrive_ros2_control_plugin PUBLIC
  include
  odrive_base/include
)

ament_target_dependencies(gazebo_hardware
  rclcpp
  hardware_interface
  pluginlib
  gz_ros2_control
)

ament_target_dependencies(odrive_ros2_control_plugin
  rclcpp
  hardware_interface
  pluginlib
)

target_link_libraries(gazebo_hardware
  ignition-gazebo6::core
)

pluginlib_export_plugin_description_file(
  gz_ros2_control
  gazebo_hardware_plugin.xml
)

pluginlib_export_plugin_description_file(
  hardware_interface
  odrive_hardware_interface.xml
)

install(
  TARGETS
    gazebo_hardware
    odrive_ros2_control_plugin
  DESTINATION lib
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  FILES
    gazebo_hardware_plugin.xml
    odrive_hardware_interface.xml
  DESTINATION share/${PROJECT_NAME}
)


ament_package()