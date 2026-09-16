#include "../include/my_qt_pkg2/qnode.hpp"

QNode::QNode(int argc, char** argv) : init_argc(argc), init_argv(argv) {}

QNode::~QNode()
{
    if (rclcpp::ok())
    {
        rclcpp::shutdown();
    }
    wait();
}

bool QNode::init()
{
    rclcpp::init(init_argc, init_argv);
    node = rclcpp::Node::make_shared("my_qt_pkg2_node");

    cmd_vel_pub = node->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel", 10);

    pose_sub = node->create_subscription<turtlesim::msg::Pose>(
        "/turtle1/pose", 10,
        [this](const turtlesim::msg::Pose::SharedPtr msg) {
            emit poseUpdated(msg->x, msg->y, msg->theta);
        });

    pen_client = node->create_client<turtlesim::srv::SetPen>("/turtle1/set_pen");
    teleport_client = node->create_client<turtlesim::srv::TeleportAbsolute>("/turtle1/teleport_absolute");

    this->start();
    return true;
}

void QNode::run()
{
    rclcpp::spin(node);
}

void QNode::sendCmdVel(double linear, double angular)
{
    if (!cmd_vel_pub) return;
    auto msg = geometry_msgs::msg::Twist();
    msg.linear.x = linear;
    msg.angular.z = angular;
    cmd_vel_pub->publish(msg);
    emit cmdVelUpdated(linear, angular);
}

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
