#ifndef my_qt_pkg2_MAIN_WINDOW_HPP
#define my_qt_pkg2_MAIN_WINDOW_HPP

#include <QMainWindow>
#include <QKeyEvent>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector>
#include <cmath>
#include "qnode.hpp"

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(int argc, char** argv, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    // 수동 이동 (WASD)
    void on_W_btn_clicked();
    void on_S_btn_clicked();
    void on_A_btn_clicked();
    void on_D_btn_clicked();

    // 경로 기록/재생
    void on_girock_str_btn_clicked();
    void on_girock_end_btn_clicked();
    void on_play_btn_clicked();

    void onPoseUpdated(double x, double y, double theta);
    void onCmdVelUpdated(double linear, double angular);

    void recordSample();   // 기록 중 주기적으로 호출
    void playNextPose();   // 재생 중 주기적으로 호출

private:
    Ui::MainWindow *ui;
    QNode *qnode;

    // 현재 거북이 위치 (pose 구독으로 갱신)
    double cur_x_ = 0.0, cur_y_ = 0.0, cur_theta_ = 0.0;
    bool has_pose_ = false;

    // 기록 상태
    struct RecordedPose { double x, y, theta; qint64 t_ms; };
    QVector<RecordedPose> recorded_path_;
    bool recording_ = false;
    double total_distance_ = 0.0;
    QElapsedTimer record_elapsed_;
    QTimer *record_sample_timer_;

    // 재생 상태
    bool playing_ = false;
    int play_index_ = 0;
    QTimer *play_timer_;
};

#endif