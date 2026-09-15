cmake_minimum_required(VERSION 3.8)
project(my_cpp_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

# msg 및 srv 인터페이스 빌드
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/MyMsg.msg"
  "srv/Calculator.srv"
)

# 1. Custom Publisher (과제 3번용)
add_executable(custom_publisher src/custom_publisher.cpp)
ament_target_dependencies(custom_publisher rclcpp)
rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")
target_link_libraries(custom_publisher "${cpp_typesupport_target}")

# 2. Calculator Server (과제 1번용)
add_executable(calculator_server src/calculator_server.cpp)
ament_target_dependencies(calculator_server rclcpp)
target_link_libraries(calculator_server "${cpp_typesupport_target}")

install(TARGETS
  custom_publisher
  calculator_server
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
