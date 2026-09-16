#ifndef my_qt_pkg_MAIN_WINDOW_HPP
#define my_qt_pkg_MAIN_WINDOW_HPP

#include <QMainWindow>
#include <QKeyEvent>
#include <QTimer>
#include <QVector>
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

    // 자동 도형 이동 (거북이가 실제로 그리도록)
    void on_triangle_btn_clicked();
    void on_square_btn_clicked();
    void on_circle_btn_clicked();

    // 펜 굵기
    void on_hard_btn_clicked();
    void on_slice_btn_clicked();

    // 펜 색상
    void on_red_btn_clicked();
    void on_blue_btn_clicked();

    void onCmdVelUpdated(double linear, double angular);

    // 타이머 콜백: 현재 스텝을 반복 재전송하며 진행
    void runNextStep();

private:
    Ui::MainWindow *ui;
    QNode *qnode;

    // 현재 펜 상태
    int pen_width_ = 3;
    int pen_r_ = 255, pen_g_ = 255, pen_b_ = 255;

    // 이동 시퀀스
    struct Step { double linear; double angular; int duration_ms; };
    QVector<Step> sequence_;
    int seq_index_ = 0;
    int step_elapsed_ms_ = 0;
    QTimer *step_timer_ = nullptr;

    void startSequence(const QVector<Step> &steps);
};

#endif