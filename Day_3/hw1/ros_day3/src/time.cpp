#include <chrono>
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

class TimeExampleNode : public rclcpp::Node
{
public:
  TimeExampleNode() : Node("time_example_node")
  {
    // 1. 현재 시간 (RCL_ROS_TIME)
    const rclcpp::Time start = this->now();

    // 2. Duration 생성 방법들
    const rclcpp::Duration timeout = rclcpp::Duration::from_seconds(0.5);
    const rclcpp::Duration period{100ms};
    const rclcpp::Duration one_and_half(1, 500'000'000);

    // 3. 연산
    const rclcpp::Time deadline = start + timeout;
    const rclcpp::Duration elapsed = this->now() - start;
    const bool expired = this->now() > deadline;

    // 4. 단위 변환 및 로그 출력
    RCLCPP_INFO(this->get_logger(), "시작 시각: %.2f초", start.seconds());
    RCLCPP_INFO(this->get_logger(), "마감 시각: %.2f초", deadline.seconds());
    RCLCPP_INFO(this->get_logger(), "경과 시간(초): %.6f초", elapsed.seconds());
    RCLCPP_INFO(this->get_logger(), "경과 시간(나노초): %ld ns", elapsed.nanoseconds());
    RCLCPP_INFO(this->get_logger(), "마감 시간 초과 여부: %s", expired ? "True" : "False");

    // 5. 메시지 필드용 변환
    builtin_interfaces::msg::Time stamp_msg = start;
    RCLCPP_INFO(this->get_logger(), "stamp_msg 변환 성공 (sec: %d, nanosec: %u)", stamp_msg.sec, stamp_msg.nanosec);
  }
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TimeExampleNode>());
  rclcpp::shutdown();
  return 0;
}