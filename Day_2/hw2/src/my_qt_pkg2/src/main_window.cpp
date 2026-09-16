#include "../include/my_qt_pkg2/main_window.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(int argc, char** argv, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFocusPolicy(Qt::StrongFocus);

    qnode = new QNode(argc, argv);
    qnode->init();

    connect(qnode, &QNode::cmdVelUpdated, this, &MainWindow::onCmdVelUpdated);
    connect(qnode, &QNode::poseUpdated, this, &MainWindow::onPoseUpdated);

    connect(ui->W_btn, &QPushButton::clicked, this, &MainWindow::on_W_btn_clicked);
    connect(ui->S_btn, &QPushButton::clicked, this, &MainWindow::on_S_btn_clicked);
    connect(ui->A_btn, &QPushButton::clicked, this, &MainWindow::on_A_btn_clicked);
    connect(ui->D_btn, &QPushButton::clicked, this, &MainWindow::on_D_btn_clicked);

    connect(ui->triangle_btn, &QPushButton::clicked, this, &MainWindow::on_triangle_btn_clicked);
    connect(ui->square_btn, &QPushButton::clicked, this, &MainWindow::on_square_btn_clicked);
    connect(ui->circle_btn, &QPushButton::clicked, this, &MainWindow::on_circle_btn_clicked);

    connect(ui->fat_btn, &QPushButton::clicked, this, &MainWindow::on_fat_btn_clicked);
    connect(ui->slice_btn, &QPushButton::clicked, this, &MainWindow::on_slice_btn_clicked);
    connect(ui->red_btn, &QPushButton::clicked, this, &MainWindow::on_red_btn_clicked);
    connect(ui->blue_btn, &QPushButton::clicked, this, &MainWindow::on_blue_btn_clicked);

    connect(ui->girock_str_btn, &QPushButton::clicked, this, &MainWindow::on_girock_str_btn_clicked);
    connect(ui->girock_end_btn, &QPushButton::clicked, this, &MainWindow::on_girock_end_btn_clicked);
    connect(ui->play_btn, &QPushButton::clicked, this, &MainWindow::on_play_btn_clicked);

    step_timer_ = new QTimer(this);
    connect(step_timer_, &QTimer::timeout, this, &MainWindow::runNextStep);

    record_sample_timer_ = new QTimer(this);
    connect(record_sample_timer_, &QTimer::timeout, this, &MainWindow::recordSample);

    play_timer_ = new QTimer(this);
    connect(play_timer_, &QTimer::timeout, this, &MainWindow::playNextPose);
}

MainWindow::~MainWindow()
{
    delete qnode;
    delete ui;
}

void MainWindow::onCmdVelUpdated(double linear, double angular)
{
    if (!recording_ && !playing_) {
        ui->statusbar->showMessage(
            QString("cmd_vel   linear.x = %1   angular.z = %2").arg(linear).arg(angular));
    }
}

void MainWindow::onPoseUpdated(double x, double y, double theta)
{
    cur_x_ = x;
    cur_y_ = y;
    cur_theta_ = theta;
    has_pose_ = true;
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

void MainWindow::on_square_btn_clicked()
{
    const double turn90 = 1.5708;
    QVector<Step> steps;
    for (int i = 0; i < 4; ++i) {
        steps.push_back({2.0, 0.0, 1000});
        steps.push_back({0.0, turn90, 1000});
    }
    startSequence(steps);
}

void MainWindow::on_triangle_btn_clicked()
{
    const double turn120 = 2.0944;
    QVector<Step> steps;
    for (int i = 0; i < 3; ++i) {
        steps.push_back({2.0, 0.0, 1000});
        steps.push_back({0.0, turn120, 1000});
    }
    startSequence(steps);
}

void MainWindow::on_circle_btn_clicked()
{
    QVector<Step> steps;
    steps.push_back({1.5, 1.0, 6283});
    startSequence(steps);
}

void MainWindow::startSequence(const QVector<Step> &steps)
{
    sequence_ = steps;
    seq_index_ = 0;
    step_elapsed_ms_ = 0;
    step_timer_->start(50);
}

void MainWindow::runNextStep()
{
    if (seq_index_ >= sequence_.size()) {
        step_timer_->stop();
        qnode->sendCmdVel(0.0, 0.0);
        return;
    }
    Step s = sequence_[seq_index_];
    qnode->sendCmdVel(s.linear, s.angular);

    step_elapsed_ms_ += 50;
    if (step_elapsed_ms_ >= s.duration_ms) {
        step_elapsed_ms_ = 0;
        seq_index_++;
    }
}

void MainWindow::on_fat_btn_clicked()
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

void MainWindow::on_girock_str_btn_clicked()
{
    if (playing_) {
        ui->statusbar->showMessage("재생 중에는 기록을 시작할 수 없습니다.");
        return;
    }
    recorded_path_.clear();
    total_distance_ = 0.0;
    recording_ = true;
    record_elapsed_.start();
    record_sample_timer_->start(100);
    ui->statusbar->showMessage("기록 시작...");
}

void MainWindow::recordSample()
{
    if (!has_pose_) return;

    RecordedPose p{cur_x_, cur_y_, cur_theta_, record_elapsed_.elapsed()};

    if (!recorded_path_.isEmpty()) {
        const auto &last = recorded_path_.last();
        double dx = p.x - last.x;
        double dy = p.y - last.y;
        total_distance_ += std::sqrt(dx * dx + dy * dy);
    }
    recorded_path_.push_back(p);

    ui->statusbar->showMessage(
        QString("기록 중...   이동거리 = %1   경과시간 = %2초   포인트 = %3개")
            .arg(total_distance_, 0, 'f', 2)
            .arg(p.t_ms / 1000.0, 0, 'f', 1)
            .arg(recorded_path_.size()));
}

void MainWindow::on_girock_end_btn_clicked()
{
    if (!recording_) return;
    recording_ = false;
    record_sample_timer_->stop();

    double total_time = recorded_path_.isEmpty() ? 0.0 : recorded_path_.last().t_ms / 1000.0;
    ui->statusbar->showMessage(
        QString("기록 종료.   총 이동거리 = %1   총 소요시간 = %2초   포인트 = %3개")
            .arg(total_distance_, 0, 'f', 2)
            .arg(total_time, 0, 'f', 1)
            .arg(recorded_path_.size()));
}

void MainWindow::on_play_btn_clicked()
{
    if (recording_) {
        ui->statusbar->showMessage("기록 중에는 재생할 수 없습니다.");
        return;
    }
    if (recorded_path_.isEmpty()) {
        ui->statusbar->showMessage("기록된 경로가 없습니다. 먼저 기록해주세요.");
        return;
    }
    playing_ = true;
    play_index_ = 0;
    ui->statusbar->showMessage("재생 중...");
    play_timer_->start(100);
}

void MainWindow::playNextPose()
{
    if (play_index_ >= recorded_path_.size()) {
        play_timer_->stop();
        playing_ = false;
        ui->statusbar->showMessage(
            QString("재생 완료.   총 %1개 포인트 재현").arg(recorded_path_.size()));
        return;
    }
    const auto &p = recorded_path_[play_index_];
    qnode->teleportAbsolute(p.x, p.y, p.theta);
    play_index_++;
}
