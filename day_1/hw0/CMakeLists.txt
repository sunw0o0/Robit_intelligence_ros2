find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

# Publisher 빌드 설정
add_executable
(
  my_publisher 
  src/my_publisher.cpp
)
ament_target_dependencies
(
  my_publisher 
  rclcpp 
  std_msgs
)

# Subscriber 빌드 설정
add_executable
(
  my_subscriber 
  src/my_subscriber.cpp
)
ament_target_dependencies
(
  my_subscriber 
  rclcpp 
  std_msgs
)

# 설치 경로 지정
install
(
  TARGETS
    my_publisher
    my_subscriber
  DESTINATION 
    lib/${PROJECT_NAME}
)
