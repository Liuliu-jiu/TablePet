#include "setwindow.h"
#include "ui_setwindow.h"

SetWindow::SetWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SetWindow)
{
    ui->setupUi(this);
}

SetWindow::~SetWindow()
{
    qDebug() << "SetWindow设置窗口析构";
    delete ui;
}
void SetWindow::closeEvent(QCloseEvent* event)
{
    emit requestRestoreWindowMove();    //请求恢复窗口移动
    QWidget::closeEvent(event);
}
