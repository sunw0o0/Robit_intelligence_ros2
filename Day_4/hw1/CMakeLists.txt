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
# [추가] get_state 서비스 타입(GetState)이 정의된 패키지를 찾아오라는 의미
# 이게 없으면 "lifecycle_msgs/srv/get_state.hpp를 못 찾는다"는 에러가 남
find_package(lifecycle_msgs REQUIRED)

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
# ↑ fake_imu는 get_state 서비스를 "직접 호출"하지 않고 그냥 LifecycleNode를
#   상속받은 것만으로 자동 제공되는 거라서, 여기에는 lifecycle_msgs를 안 넣어도 됨

add_executable(imu_watchdog src/imu_watchdog.cpp)
ament_target_dependencies(imu_watchdog
  rclcpp
  rclcpp_lifecycle
  sensor_msgs
  lifecycle_msgs   # [추가] imu_watchdog는 GetState 서비스 "클라이언트"를 직접 쓰니까 필요함
)

add_executable(imu_watchdog_wall src/imu_watchdog_wall.cpp)
ament_target_dependencies(imu_watchdog_wall
  rclcpp
  rclcpp_lifecycle
  sensor_msgs
  lifecycle_msgs   # [추가] 이 파일도 같은 기능을 넣을 거라면 마찬가지로 필요
)

install(TARGETS
  time_node
  fake_imu
  imu_watchdog
  imu_watchdog_wall
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()