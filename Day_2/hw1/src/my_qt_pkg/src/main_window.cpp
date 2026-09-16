#include "../include/my_qt_pkg/main_window.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(int argc, char** argv, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::StrongFocus);

    qnode = new QNode(argc, argv);
    qnode->init();

    connect(qnode, &QNode::cmdVelUpdated, this, &MainWindow::onCmdVelUpdated);

    connect(ui->W_btn, &QPushButton::clicked, this, &MainWindow::on_W_btn_clicked);
    connect(ui->S_btn, &QPushButton::clicked, this, &MainWindow::on_S_btn_clicked);
    connect(ui->A_btn, &QPushButton::clicked, this, &MainWindow::on_A_btn_clicked);
    connect(ui->D_btn, &QPushButton::clicked, this, &MainWindow::on_D_btn_clicked);

    connect(ui->triangle_btn, &QPushButton::clicked, this, &MainWindow::on_triangle_btn_clicked);
    connect(ui->square_btn, &QPushButton::clicked, this, &MainWindow::on_square_btn_clicked);
    connect(ui->circle_btn, &QPushButton::clicked, this, &MainWindow::on_circle_btn_clicked);

    connect(ui->hard_btn, &QPushButton::clicked, this, &MainWindow::on_hard_btn_clicked);
    connect(ui->slice_btn, &QPushButton::clicked, this, &MainWindow::on_slice_btn_clicked);

    connect(ui->red_btn, &QPushButton::clicked, this, &MainWindow::on_red_btn_clicked);
    connect(ui->blue_btn, &QPushButton::clicked, this, &MainWindow::on_blue_btn_clicked);

    step_timer_ = new QTimer(this);
    connect(step_timer_, &QTimer::timeout, this, &MainWindow::runNextStep);
}

MainWindow::~MainWindow()
{
    delete qnode;
    delete ui;
}

void MainWindow::onCmdVelUpdated(double linear, double angular)
{
    ui->statusbar->showMessage(
        QString("cmd_vel   linear.x = %1   angular.z = %2").arg(linear).arg(angular));
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_W: qnode->sendCmdVel(2.0, 0.0); break;
        case Qt::Key_S: qnode->sendCmdVel(-2.0, 0.0); break;
        case Qt::Key_A: qnode->sendCmdVel(0.0, 2.0); break;
        case Qt::Key_D: qnode->sendCmdVel(0.0, -2.0); break;
        default: QMainWindow::keyPressEvent(event); break;
    }
}

void MainWindow::on_W_btn_clicked() { qnode->sendCmdVel(2.0, 0.0); }
void MainWindow::on_S_btn_clicked() { qnode->sendCmdVel(-2.0, 0.0); }
void MainWindow::on_A_btn_clicked() { qnode->sendCmdVel(0.0, 2.0); }
void MainWindow::on_D_btn_clicked() { qnode->sendCmdVel(0.0, -2.0); }

// 사각형: 전진 1초 → 90도 회전, 4번 반복
void MainWindow::on_square_btn_clicked()
{
    const double turn90 = 1.5708; // 90도(rad) / 1초
    QVector<Step> steps;
    for (int i = 0; i < 4; ++i) {
        steps.push_back({2.0, 0.0, 1000});
        steps.push_back({0.0, turn90, 1000});
    }
    startSequence(steps);
}

// 삼각형: 전진 1초 → 120도 회전, 3번 반복
void MainWindow::on_triangle_btn_clicked()
{
    const double turn120 = 2.0944; // 120도(rad) / 1초
    QVector<Step> steps;
    for (int i = 0; i < 3; ++i) {
        steps.push_back({2.0, 0.0, 1000});
        steps.push_back({0.0, turn120, 1000});
    }
    startSequence(steps);
}

// 원: 전진 + 회전을 동시에, 한 바퀴 도는 시간만큼 유지
void MainWindow::on_circle_btn_clicked()
{
    QVector<Step> steps;
    // angular=1.0 rad/s 로 한 바퀴(2π rad) 도는 데 걸리는 시간 ≈ 6283ms
    steps.push_back({1.5, 1.0, 6283});
    startSequence(steps);
}

void MainWindow::on_hard_btn_clicked()
{
    pen_width_ = 6;
    qnode->setPen(pen_r_, pen_g_, pen_b_, pen_width_);
}

void MainWindow::on_slice_btn_clicked()
{
    pen_width_ = 1;
    qnode->setPen(pen_r_, pen_g_, pen_b_, pen_width_);
}

void MainWindow::on_red_btn_clicked()
{
    pen_r_ = 255; pen_g_ = 0; pen_b_ = 0;
    qnode->setPen(pen_r_, pen_g_, pen_b_, pen_width_);
}

void MainWindow::on_blue_btn_clicked()
{
    pen_r_ = 0; pen_g_ = 0; pen_b_ = 255;
    qnode->setPen(pen_r_, pen_g_, pen_b_, pen_width_);
}

// 시퀀스 시작: 50ms 간격 반복 타이머 가동
void MainWindow::startSequence(const QVector<Step> &steps)
{
    sequence_ = steps;
    seq_index_ = 0;
    step_elapsed_ms_ = 0;
    step_timer_->start(50);
}

// 매 50ms마다 호출됨: 현재 스텝의 속도값을 계속 재전송하고,
// 지정된 시간(duration_ms)이 지나면 다음 스텝으로 넘어감
void MainWindow::runNextStep()
{
    if (seq_index_ >= sequence_.size()) {
        step_timer_->stop();
        qnode->sendCmdVel(0.0, 0.0); // 시퀀스 종료 시 정지
        return;
    }

    Step s = sequence_[seq_index_];
    qnode->sendCmdVel(s.linear, s.angular); // 계속 재전송 → 메시지 유실 방지

    step_elapsed_ms_ += 50;
    if (step_elapsed_ms_ >= s.duration_ms) {
        step_elapsed_ms_ = 0;
        seq_index_++;
    }
}