#include "mainwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
#ifdef Q_OS_UNIX
    qputenv("QT_QPA_PLATFORM","xcb");//linux系统使用x11协议确保窗口正确移动
#endif
    QApplication a(argc, argv);
    MainWidget w;
    w.show();
    return a.exec();
}
