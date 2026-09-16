#ifndef my_qt_pkg_QNODE_HPP
#define my_qt_pkg_QNODE_HPP

#include <QThread>
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <turtlesim/srv/set_pen.hpp>

class QNode : public QThread
{
    Q_OBJECT
public:
    QNode(int argc, char** argv);
    virtual ~QNode();

    bool init();
    void run() override;
    void sendCmdVel(double linear, double angular);
    void setPen(int r, int g, int b, int width);

signals:
    void cmdVelUpdated(double linear, double angular);

private:
    int init_argc;
    char** init_argv;
    rclcpp::Node::SharedPtr node;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub;
    rclcpp::Client<turtlesim::srv::SetPen>::SharedPtr pen_client;
};

#endif