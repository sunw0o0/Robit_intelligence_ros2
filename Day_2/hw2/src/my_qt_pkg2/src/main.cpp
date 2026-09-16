#include <QApplication>
#include "../include/my_qt_pkg2/main_window.hpp"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    MainWindow w(argc, argv);
    w.show();
    return app.exec();
}