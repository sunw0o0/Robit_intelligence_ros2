#include <chrono>   
#include <functional>  
#include <memory>    
#include <string>  

#include "rclcpp/rclcpp.hpp"             // ROS 2 C++ 기본 라이브러리 헤더
#include "my_cpp_pkg/msg/my_msg.hpp"     // 생성한 커스텀 메시지 헤더 (MyMsg.msg)

using namespace std::chrono_literals;    // 500ms 처럼 시간 단위를 직관적으로 쓰기 위한 네임스페이스

// rclcpp::Node 클래스를 상속받아 커스텀 퍼블리셔 노드 정의
class CustomPublisher : public rclcpp::Node {
public:
  CustomPublisher() 
  : Node("custom_publisher_node") {  // 노드 이름을 "custom_publisher_node"로 지정
    
    // "custom_topic" 이름으로 MyMsg 타입의 메시지를 퍼블리시하는 객체 생성 (큐 크기: 10)
    publisher_ = this->create_publisher<my_cpp_pkg::msg::MyMsg>(
      "custom_topic", 
      10
    );
    
    // 500ms(0.5초) 주기로 timer_callback 함수를 자동 호출하는 타이머 생성
    timer_ = this->create_wall_timer(
      500ms, 
      std::bind(&CustomPublisher::timer_callback, this)
    );
  }

private:
  // 타이머 주기마다 호출되어 데이터를 만드는 콜백 함수
  void timer_callback() {
    // 커스텀 메시지 객체 생성
    auto message = my_cpp_pkg::msg::MyMsg();
    
    message.id = count_++;                     // id 값을 1씩 증가시키며 저장
    message.label = "Custom Message Data";      // label 문자열 지정

    // 터미널 화면에 퍼블리시할 데이터 로그 출력
    RCLCPP_INFO(
      this->get_logger(), 
      "Publishing -> id: %d, label: '%s'", 
      message.id, 
      message.label.c_str()
    );
    
    // 퍼블리셔를 통해 실제로 메시지 전송(발행)
    publisher_->publish(message);
  }

  // 멤버 변수 선언
  rclcpp::TimerBase::SharedPtr timer_;                             // 타이머 객체 포인터
  rclcpp::Publisher<my_cpp_pkg::msg::MyMsg>::SharedPtr publisher_; // 퍼블리셔 객체 포인터
  int32_t count_ = 0;                                               // id 카운트 변수
};

// 메인 함수 (프로그램 시작점)
int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);                                // ROS 2 C++ 시스템 초기화
  rclcpp::spin(std::make_shared<CustomPublisher>());       // 노드를 실행하고 이벤트(타이머) 대기
  rclcpp::shutdown();                                      // 종료 시 ROS 2 시스템 자원 해제
  return 0;
}
