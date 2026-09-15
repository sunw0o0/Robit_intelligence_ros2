#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class MySubscriber : public rclcpp::Node 
{
public:
  MySubscriber() 
  : Node("my_subscriber_node") 
  {
    
    // 서브스크라이버 생성
    subscription_ = this->create_subscription<std_msgs::msg::String>(
      "topic", 
      10, 
      std::bind
      (
        &MySubscriber::topic_callback, 
        this, 
        std::placeholders::_1
      )
    );
  }

private:
  void topic_callback(const std_msgs::msg::String & msg) const 
  {
    RCLCPP_INFO(
      this->get_logger(), 
      "I heard: '%s'", 
      msg.data.c_str()
    );
  }

  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
};

int main(int argc, char * argv[]) 
{
  rclcpp::init(argc, argv);
  
  rclcpp::spin(
    std::make_shared<MySubscriber>()
  );
  
  rclcpp::shutdown();
  return 0;
}
