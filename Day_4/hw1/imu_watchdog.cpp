#include <chrono>
#include <memory>
#include <optional>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_lifecycle/lifecycle_publisher.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "lifecycle_msgs/srv/get_state.hpp"   // fake_imu의 상태(get_state)를 물어보기 위한 서비스 타입

// 콜백 반환 타입 짧은 별명
using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

// IMU 데이터가 살아있는지 감시하는 워치독 노드
class ImuWatchdogNode : public rclcpp_lifecycle::LifecycleNode
{
public:
  explicit ImuWatchdogNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : LifecycleNode("imu_watchdog", options)
  {
    // 이 시간(초) 동안 새 데이터가 없으면 STALE로 판단
    declare_parameter("timeout_sec", 1.0);

    // 워치독이 age를 검사하는 주기(Hz)
    declare_parameter("check_rate_hz", 5.0);
  }

  CallbackReturn on_configure(const rclcpp_lifecycle::State &) override
  {
    timeout_sec_ = get_parameter("timeout_sec").as_double();
    check_rate_hz_ = get_parameter("check_rate_hz").as_double();

    // 둘 중 하나라도 0 이하면 잘못된 설정이므로 실패 처리
    if (timeout_sec_ <= 0.0 || check_rate_hz_ <= 0.0)
    {
      RCLCPP_ERROR(
        get_logger(),
        "timeout_sec(%.2f)와 check_rate_hz(%.2f)는 모두 0보다 커야 합니다",
        timeout_sec_, check_rate_hz_);
      return CallbackReturn::FAILURE;
    }

    // 원본 IMU 구독 (필요한 데이터는 콜백에서 last_stamp_에 저장만 해둠)
    sub_ = create_subscription<sensor_msgs::msg::Imu>(
      "/imu", 10,
      [this](sensor_msgs::msg::Imu::SharedPtr msg) { imu_callback(msg); });

    // 신선한 데이터만 재발행할 퍼블리셔
    pub_ = create_publisher<sensor_msgs::msg::Imu>("/imu_checked", 10);

    // ── 여기부터 이번 과제에서 새로 추가하는 부분 ──
    // fake_imu 노드가 제공하는 "/fake_imu/get_state" 서비스를 부를 클라이언트를 만듦.
    // 클라이언트를 만든다고 바로 통신이 되는 건 아니고, 나중에 이 client_로
    // "요청을 보낼 준비"만 해두는 것. 실제 fake_imu 노드가 켜져 있어야 응답이 옴.
    fake_imu_state_client_ =
      create_client<lifecycle_msgs::srv::GetState>("/fake_imu/get_state");

    RCLCPP_INFO(
      get_logger(), "준비 완료: timeout %.2f초, 검사 주기 %.1fHz",
      timeout_sec_, check_rate_hz_);
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_activate(const rclcpp_lifecycle::State & state) override
  {
    LifecycleNode::on_activate(state);

    // 상태 초기화 (재활성화 시 이전 판정이 남아있지 않도록)
    last_stamp_.reset();
    was_stale_ = false;

    // check_rate_hz_ 주기로 age 검사
    timer_ = create_timer(
      std::chrono::duration<double>(1.0 / check_rate_hz_),
      [this]() { check_age(); });

    // ── 새로 추가 ──
    // 1초마다 fake_imu의 상태를 확인하는 타이머.
    // 이 타이머 콜백(check_fake_imu_state)이 실행되는 동안에도
    // executor(직원)는 한 명뿐이라는 걸 잊으면 안 됨 → 뒤에서 문제가 생기는 지점
    state_timer_ = create_timer(
      std::chrono::duration<double>(1.0),
      [this]() { check_fake_imu_state(); });

    RCLCPP_INFO(get_logger(), "시작 완료: IMU 감시를 시작합니다.");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_deactivate(const rclcpp_lifecycle::State & state) override
  {
    LifecycleNode::on_deactivate(state);
    timer_.reset();
    state_timer_.reset();  // 상태 확인 타이머도 같이 멈춤
    RCLCPP_INFO(get_logger(), "일시정지: IMU 감시를 멈춥니다.");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_cleanup(const rclcpp_lifecycle::State &) override
  {
    sub_.reset();
    pub_.reset();
    last_stamp_.reset();
    fake_imu_state_client_.reset();  // on_configure에서 만든 클라이언트를 여기서 해제 (표에 나온 규칙)
    RCLCPP_INFO(get_logger(), "청소 완료: 만든 자원을 싹 정리했습니다.");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_shutdown(const rclcpp_lifecycle::State &) override
  {
    timer_.reset();
    state_timer_.reset();
    sub_.reset();
    pub_.reset();
    fake_imu_state_client_.reset();
    RCLCPP_INFO(get_logger(), "종료 완료: 노드를 완전히 끕니다.");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_error(const rclcpp_lifecycle::State &) override
  {
    timer_.reset();
    state_timer_.reset();
    sub_.reset();
    pub_.reset();
    fake_imu_state_client_.reset();
    RCLCPP_ERROR(get_logger(), "비상 상황: 자원을 급하게 정리합니다.");
    return CallbackReturn::SUCCESS;
  }

private:
  // 원본 IMU 콜백: 마지막 수신 시각(헤더 stamp 기준)만 기록해둠
  void imu_callback(const sensor_msgs::msg::Imu::SharedPtr msg)
  {
    last_stamp_ = rclcpp::Time(msg->header.stamp);
    last_msg_ = *msg;
  }

  // 주기적으로 age를 계산해서 NO DATA / STALE / RECOVERED 판정
  void check_age()
  {
    rclcpp::Time now = this->now();

    // 아직 한 번도 데이터를 못 받은 경우
    if (!last_stamp_.has_value())
    {
      RCLCPP_WARN(get_logger(), "[NO DATA] 아직 /imu 데이터를 받지 못했습니다.");
      return;
    }

    double age_sec = (now - last_stamp_.value()).seconds();

    if (age_sec > timeout_sec_)
    {
      // 지금 막 STALE로 전환된 경우에만 로그를 찍고 싶다면 was_stale_ 체크 후 로그
      RCLCPP_WARN(
        get_logger(), "[STALE] 마지막 데이터로부터 %.2f초 경과 (허용: %.2f초)",
        age_sec, timeout_sec_);
      was_stale_ = true;
      // STALE 상태에서는 재발행하지 않음 (신선한 데이터만 내보냄)
      return;
    }

    // 신선한 데이터 → STALE에서 복귀했으면 RECOVERED 로그
    if (was_stale_)
    {
      RCLCPP_INFO(get_logger(), "[RECOVERED] 데이터 수신이 재개되었습니다 (age %.2f초)", age_sec);
      was_stale_ = false;
    }

    // 신선한 데이터만 /imu_checked로 재발행
    pub_->publish(last_msg_);
  }

  // ── 새로 추가된 함수: 2-1단계, "일부러 틀리게" 구현한 버전 ──
  // fake_imu 노드에게 "지금 상태가 뭐야?"라고 물어보고 로그로 출력하는 함수.
  // 여기서는 일부러 잘못된 방식(콜백 안에서 응답 완료를 기다리는 방식)으로 짬.
  // 수정 (2-2, 올바른 버전)
void check_fake_imu_state()
{
  if (!fake_imu_state_client_->service_is_ready())
  {
    RCLCPP_WARN(get_logger(), "[NO SERVICE] /fake_imu/get_state 서비스가 아직 준비되지 않았습니다.");
    return;
  }

  auto request = std::make_shared<lifecycle_msgs::srv::GetState::Request>();

  // async_send_request에 "응답 오면 실행할 함수"를 두 번째 인자로 넘김.
  // 이 줄을 실행하고 나면 check_fake_imu_state()는 곧바로 끝나버림(리턴).
  // → executor(직원)는 여기서 멈추지 않고 바로 다음 콜백으로 넘어갈 수 있음
  // → 나중에 응답이 도착하면, 그때 executor가 아래 람다 함수를 별도로 실행해줌
  fake_imu_state_client_->async_send_request(
    request,
    [this](rclcpp::Client<lifecycle_msgs::srv::GetState>::SharedFuture future)
    {
      auto response = future.get();
      RCLCPP_INFO(get_logger(), "fake_imu state: %s", response->current_state.label.c_str());
    });
}

  double timeout_sec_{1.0};
  double check_rate_hz_{5.0};

  std::optional<rclcpp::Time> last_stamp_;
  sensor_msgs::msg::Imu last_msg_;
  bool was_stale_{false};

  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr sub_;
  rclcpp_lifecycle::LifecyclePublisher<sensor_msgs::msg::Imu>::SharedPtr pub_;
  rclcpp::TimerBase::SharedPtr timer_;

  // ── 새로 추가된 멤버 변수들 ──
  rclcpp::TimerBase::SharedPtr state_timer_;  // 1초마다 fake_imu 상태를 확인하는 타이머
  rclcpp::Client<lifecycle_msgs::srv::GetState>::SharedPtr fake_imu_state_client_;  // get_state 서비스 클라이언트
};


int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<ImuWatchdogNode>();
  rclcpp::spin(node->get_node_base_interface());
  rclcpp::shutdown();
  return 0;
}