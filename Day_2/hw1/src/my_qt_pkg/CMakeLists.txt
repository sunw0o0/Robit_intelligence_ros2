cmake_minimum_required(VERSION 3.8)
project(my_qt_pkg)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(turtlesim REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

include_directories(include)

add_executable(my_qt_pkg_node
  src/main.cpp
  src/main_window.cpp
  src/qnode.cpp
  include/my_qt_pkg/main_window.hpp
  include/my_qt_pkg/qnode.hpp
  ui/mainwindow.ui
)

ament_target_dependencies(my_qt_pkg_node
  rclcpp
  geometry_msgs
  turtlesim
)

target_link_libraries(my_qt_pkg_node
  Qt5::Widgets
)

install(TARGETS my_qt_pkg_node
  DESTINATION lib/${PROJECT_NAME})

ament_package()