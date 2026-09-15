cmake_minimum_required(VERSION 3.8)
project(my_cpp_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(my_publisher src/my_publisher.cpp)
ament_target_dependencies(my_publisher rclcpp std_msgs)

add_executable(my_subscriber src/my_subscriber.cpp)
ament_target_dependencies(my_subscriber rclcpp std_msgs)

install(TARGETS
  my_publisher
  my_subscriber
  DESTINATION lib/${PROJECT_NAME}
)

# launch 폴더 설치 규칙 추가
install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
