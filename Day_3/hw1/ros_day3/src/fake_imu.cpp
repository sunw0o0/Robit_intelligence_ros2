#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_lifecycle/lifecycle_publisher.hpp"
#include "sensor_msgs/msg/imu.hpp"

// 콜백 반환 타입 이름이 너무 길어서 'CallbackReturn'이라는 짧은 별명으로 사용
using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

//가짜 센서 노드 class
class FakeImuNode : public rclcpp_lifecycle::LifecycleNode
{
public:
  // 생성자: 프로그램이 처음 켜질 때 설정값을 등록함
  explicit FakeImuNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : LifecycleNode("fake_imu", options)
  {
    // 초당 데이터 송신 횟수 설정
    declare_parameter("rate_hz", 10.0);
    
    // 시간 일부러 늦추기 설정
    declare_parameter("stamp_offset_sec", 0.0);
  }

  // 준비 버튼을 눌렀을 때 실행되는 함수
  CallbackReturn on_configure(const rclcpp_lifecycle::State &) override
  {
    // 외부에서 입력한 설정값 가져오기
    rate_hz_ = get_parameter("rate_hz").as_double();
    stamp_offset_sec_ = get_parameter("stamp_offset_sec").as_double();

    // 입력받은 주기가 0 이하이면 잘못된 설정이므로 실패 처리
    if (rate_hz_ <= 0.0) 
    {
      RCLCPP_ERROR(get_logger(), "주기(rate_hz)는 0보다 커야 합니다");
      return CallbackReturn::FAILURE; // 대기 상태로 되돌아가서 다시 설정할 수 있게 함
    }

    pub_ = create_publisher<sensor_msgs::msg::Imu>("/imu", 10);
    
    RCLCPP_INFO(get_logger(), "초당 %.1f회 송신, %.2f초 지연 설정", 
                rate_hz_, stamp_offset_sec_);
    return CallbackReturn::SUCCESS; // 준비 성공 (Inactive 상태로 이동)
  }

  //시작 버튼을 눌렀을 때 시작되는 함수
  CallbackReturn on_activate(const rclcpp_lifecycle::State & state) override
  {
    // 스위치 작동
    LifecycleNode::on_activate(state);

    // 정해진 주기마다 publish_message 함수를 자동으로 실행하는 타이머 켜기
    timer_ = create_timer(
      std::chrono::duration<double>(1.0 / rate_hz_),
      [this]() { publish_message(); });

    RCLCPP_INFO(get_logger(), "센서 데이터 송신을 시작합니다.");
    return CallbackReturn::SUCCESS; // Active 상태로 이동
  }

  // 일시정지 버튼을 눌렀을 때 실행되는 함수
  CallbackReturn on_deactivate(const rclcpp_lifecycle::State & state) override
  {
    // 전원 OFF
    LifecycleNode::on_deactivate(state);

    // 타이머를 없애서 자동 실행 멈춤
    timer_.reset();

    RCLCPP_INFO(get_logger(), "센서 데이터 송신을 멈춥니다.");
    return CallbackReturn::SUCCESS; // Inactive 상태로 복귀
  }

  // 초기화 버튼을 눌렀을 때 실행되는 함수
  CallbackReturn on_cleanup(const rclcpp_lifecycle::State &) override
  {
    // 메모리 정리
    pub_.reset();

    RCLCPP_INFO(get_logger(), "on_configure에서 만든 데이터를 정리하고 대기 상태를 시작합니다.");
    return CallbackReturn::SUCCESS; // 대기 상태로 복귀 (Unconfigured)
  }

  // 프로그램 끝날 때까지 실행
  CallbackReturn on_shutdown(const rclcpp_lifecycle::State &) override
  {
    timer_.reset();
    pub_.reset();
    RCLCPP_INFO(get_logger(), "종료 완료: 노드를 완전히 끕니다.");
    return CallbackReturn::SUCCESS;
  }

  //비상이나 고장 시 실행
  CallbackReturn on_error(const rclcpp_lifecycle::State &) override
  {
    timer_.reset();
    pub_.reset();
    RCLCPP_ERROR(get_logger(), "비상 상황: 자원을 급하게 정리합니다.");
    return CallbackReturn::SUCCESS;
  }

private:
  //데이터를 옮겨줌(publish)
  void publish_message()
  {
    auto msg = sensor_msgs::msg::Imu();

    //지금 현재 시각 가져오기
    rclcpp::Time current_time = this->now();

    rclcpp::Duration offset = rclcpp::Duration::from_seconds(stamp_offset_sec_);

    // 현재 시각 - 늦출 시간 계산을 통해 과거 타임스탬프 만들기
    msg.header.stamp = current_time - offset;
    msg.header.frame_id = "imu_link";

    // 4. 데이터 보내기
    pub_->publish(msg);
  }

  double rate_hz_{10.0};         
  double stamp_offset_sec_{0.0};  

  rclcpp_lifecycle::LifecyclePublisher<sensor_msgs::msg::Imu>::SharedPtr pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};


int main(int argc, char ** argv)
{
  // ROS 2 시작 알림
  rclcpp::init(argc, argv);

  // 노드 만들기
  auto node = std::make_shared<FakeImuNode>();

  // 노드가 켜진 채로 외부 명령을 기다리게 유지하기
  rclcpp::spin(node->get_node_base_interface());

  // ROS 2 종료
  rclcpp::shutdown();
  return 0;
}