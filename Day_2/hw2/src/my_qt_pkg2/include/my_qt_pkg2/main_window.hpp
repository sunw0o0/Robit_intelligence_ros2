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
    void on_W_btn_clicked();
    void on_S_btn_clicked();
    void on_A_btn_clicked();
    void on_D_btn_clicked();

    void on_triangle_btn_clicked();
    void on_square_btn_clicked();
    void on_circle_btn_clicked();

    void on_fat_btn_clicked();
    void on_slice_btn_clicked();
    void on_red_btn_clicked();
    void on_blue_btn_clicked();

    void on_girock_str_btn_clicked();
    void on_girock_end_btn_clicked();
    void on_play_btn_clicked();

    void onPoseUpdated(double x, double y, double theta);
    void onCmdVelUpdated(double linear, double angular);

    void runNextStep();
    void recordSample();
    void playNextPose();

private:
    Ui::MainWindow *ui;
    QNode *qnode;

    int pen_width_ = 3;
    int pen_r_ = 255, pen_g_ = 255, pen_b_ = 255;

    struct Step { double linear; double angular; int duration_ms; };
    QVector<Step> sequence_;
    int seq_index_ = 0;
    int step_elapsed_ms_ = 0;
    QTimer *step_timer_;

    void startSequence(const QVector<Step> &steps);

    double cur_x_ = 0.0, cur_y_ = 0.0, cur_theta_ = 0.0;
    bool has_pose_ = false;

    struct RecordedPose { double x, y, theta; qint64 t_ms; };
    QVector<RecordedPose> recorded_path_;
    bool recording_ = false;
    double total_distance_ = 0.0;
    QElapsedTimer record_elapsed_;
    QTimer *record_sample_timer_;

    bool playing_ = false;
    int play_index_ = 0;
    QTimer *play_timer_;
};

#endif
