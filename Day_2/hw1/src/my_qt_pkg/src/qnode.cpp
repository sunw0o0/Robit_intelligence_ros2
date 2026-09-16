#include "../include/my_qt_pkg/qnode.hpp"

// 생성자
QNode::QNode(int argc, char** argv) : init_argc(argc), init_argv(argv) {}

// 소멸자: ROS 셧다운
QNode::~QNode()
{
    if (rclcpp::ok())
    {
        rclcpp::shutdown();
    }
    wait();
}

// ROS2 노드, 퍼블리셔, 서비스 클라이언트 생성
bool QNode::init()
{
    rclcpp::init(init_argc, init_argv);
    node = rclcpp::Node::make_shared("my_qt_node");
    cmd_vel_pub = node->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel", 10);
    pen_client = node->create_client<turtlesim::srv::SetPen>("/turtle1/set_pen");
    this->start();
    return true;
}

// QThread 스핀 함수
void QNode::run()
{
    rclcpp::spin(node);
}

// 속도 토픽 발행 함수
void QNode::sendCmdVel(double linear, double angular)
{
    if (!cmd_vel_pub) return;
    auto msg = geometry_msgs::msg::Twist();
    msg.linear.x = linear;
    msg.angular.z = angular;
    cmd_vel_pub->publish(msg);
    emit cmdVelUpdated(linear, angular);
}

// 펜 색상/굵기 변경 서비스 호출
void QNode::setPen(int r, int g, int b, int width)
{
    if (!pen_client) return;
    if (!pen_client->wait_for_service(std::chrono::milliseconds(100)))
    {
        return;
    }
    auto request = std::make_shared<turtlesim::srv::SetPen::Request>();
    request->r = r;
    request->g = g;
    request->b = b;
    request->width = width;
    request->off = 0;
    pen_client->async_send_request(request);
}