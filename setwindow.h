#ifndef SETWINDOW_H
#define SETWINDOW_H

#include <QWidget>

namespace Ui {
class SetWindow;
}

class SetWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SetWindow(QWidget *parent = nullptr);
    ~SetWindow();

signals:
    void requestRestoreWindowMove();                //请求恢复窗口移动信号
private:
    void closeEvent(QCloseEvent* event) override;   //重写关闭事件
    Ui::SetWindow *ui;
};

#endif // SETWINDOW_H
