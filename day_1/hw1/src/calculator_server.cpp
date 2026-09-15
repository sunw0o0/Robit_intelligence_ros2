#include <memory> 
#include <string> 

#include "rclcpp/rclcpp.hpp"           
#include "my_cpp_pkg/srv/calculator.hpp" 

// rclcpp::Node를 상속받아 계산기 서비스 서버 노드 클래스 정의
class CalculatorServer : public rclcpp::Node 
{
public:
  CalculatorServer() 
  : Node("calculator_server_node") 
{  // 노드 이름을 "calculator_server_node"로 지정
    
    // "calculate" 서비스 서버 생성 (요청이 들어오면 handle_calculate 콜백 함수 호출)
    service_ = this->create_service<my_cpp_pkg::srv::Calculator>(
      "calculate",
      std::bind
      (
        &CalculatorServer::handle_calculate, 
        this, 
        std::placeholders::_1,  // request 객체를 전달받는 첫 번째 파라미터 위치
        std::placeholders::_2   // response 객체를 전달받는 두 번째 파라미터 위치
      )
    );
    // 서비스 준비 완료 로그 출력
    RCLCPP_INFO(this->get_logger(), "Calculator Service Server Ready!");
  }

private:
  // 클라이언트로부터 서비스 요청이 올 때 실행되는 콜백 함수
  void handle_calculate(
    const std::shared_ptr<my_cpp_pkg::srv::Calculator::Request> request, // 수신받은 요청 데이터 (a, b, op)
    std::shared_ptr<my_cpp_pkg::srv::Calculator::Response> response      // 클라이언트에 보낼 응답 데이터 (result, success)
  ) {
    // 요청받은 인자 정보 로그 출력
    RCLCPP_INFO(
      this->get_logger(), 
      "Incoming Request -> a: %ld, b: %ld, op: '%s'", 
      request->a, 
      request->b, 
      request->op.c_str()
    );

    // 1. 덧셈 연산자 처리
    if (request->op == "+") 
    {
      response->result = static_cast<double>(request->a + request->b);
      response->success = true;
    } 
    // 2. 뺄셈 연산자 처리
    else if (request->op == "-") 
    {
      response->result = static_cast<double>(request->a - request->b);
      response->success = true;
    } 
    // 3. 곱셈 연산자 처리
    else if (request->op == "*") 
    {
      response->result = static_cast<double>(request->a * request->b);
      response->success = true;
    } 
    // 4. 나눗셈 연산자 처리
    else if (request->op == "/") 
    {
      if (request->b == 0) 
      {  // 0으로 나누기 예외 처리
        RCLCPP_ERROR(this->get_logger(), "Cannot divide by zero!");
        response->result = 0.0;
        response->success = false;
      } 
      else 
      {
        response->result = static_cast<double>(request->a) / request->b;
        response->success = true;
      }
    } 
    // 5. 알 수 없는 연산자 처리
    else 
    {
      RCLCPP_ERROR(this->get_logger(), "Unknown Operator: '%s'", request->op.c_str());
      response->result = 0.0;
      response->success = false;
    }

    // 처리 결과 로그 출력
    RCLCPP_INFO(
      this->get_logger(), 
      "Sending Response -> result: %.2f, success: %s", 
      response->result, 
      response->success ? "true" : "false"
    );
  }

  // 서비스 서버 객체 스마트 포인터 선언
  rclcpp::Service<my_cpp_pkg::srv::Calculator>::SharedPtr service_;
};

// 메인 함수
int main(int argc, char * argv[]) 
{
  rclcpp::init(argc, argv);                                // ROS 2 C++ 노드 시스템 초기화
  rclcpp::spin(std::make_shared<CalculatorServer>());       // 서비스 요청 대기 상태(이벤트 루프) 진입
  rclcpp::shutdown();                                      // 종료 시 시스템 자원 해제
  return 0;
}
