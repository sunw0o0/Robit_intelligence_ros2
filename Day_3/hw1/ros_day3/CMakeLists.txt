cmake_minimum_required(VERSION 3.8)
project(ros_day3)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)

add_executable(time_node src/time.cpp)
ament_target_dependencies(time_node
  rclcpp
  std_msgs
)

add_executable(fake_imu src/fake_imu.cpp)
ament_target_dependencies(fake_imu
  rclcpp
  rclcpp_lifecycle
  sensor_msgs
)

add_executable(imu_watchdog src/imu_watchdog.cpp)
ament_target_dependencies(imu_watchdog
  rclcpp
  rclcpp_lifecycle
  sensor_msgs
)

install(TARGETS
  time_node
  fake_imu
  imu_watchdog
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
