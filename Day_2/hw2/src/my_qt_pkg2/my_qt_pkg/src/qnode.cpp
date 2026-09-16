#include "../include/my_qt_pkg2/qnode.hpp"

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

// ROS2 노드, 퍼블리셔, 구독자, 서비스 클라이언트 생성
bool QNode::init()
{
    rclcpp::init(init_argc, init_argv);
    node = rclcpp::Node::make_shared("my_qt_pkg2_node");

    cmd_vel_pub = node->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel", 10);

    // 거북이의 현재 pose를 실시간으로 구독하여 GUI에 emit
    pose_sub = node->create_subscription<turtlesim::msg::Pose>(
        "/turtle1/pose", 10,
        [this](const turtlesim::msg::Pose::SharedPtr msg) {
            emit poseUpdated(msg->x, msg->y, msg->theta);
        });

    teleport_client = node->create_client<turtlesim::srv::TeleportAbsolute>("/turtle1/teleport_absolute");

    this->start();
    return true;
}

// QThread 스핀 함수
void QNode::run()
{
    rclcpp::spin(node);
}

// 속도 토픽 발행 함수 (WASD 수동 조작용)
void QNode::sendCmdVel(double linear, double angular)
{
    if (!cmd_vel_pub) return;
    auto msg = geometry_msgs::msg::Twist();
    msg.linear.x = linear;
    msg.angular.z = angular;
    cmd_vel_pub->publish(msg);
    emit cmdVelUpdated(linear, angular);
}

// 저장된 좌표로 순간이동 (경로 재생용)
void QNode::teleportAbsolute(double x, double y, double theta)
{
    if (!teleport_client) return;
    if (!teleport_client->wait_for_service(std::chrono::milliseconds(100)))
    {
        return;
    }
    auto request = std::make_shared<turtlesim::srv::TeleportAbsolute::Request>();
    request->x = x;
    request->y = y;
    request->theta = theta;
    teleport_client->async_send_request(request);
}