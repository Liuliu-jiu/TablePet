#ifndef SHOWFONTWINDOW_H
#define SHOWFONTWINDOW_H

//平滑淡出时间
#define SMOOTH_FADE_DURATION 1000
#define SMOOTH_MOVE_DURATION 1000

#include <QWidget>
#include<QLabel>
#include<QPropertyAnimation>
#include<QGraphicsOpacityEffect>
#include<QCloseEvent>
namespace Ui {
class ShowFontWindow;
}

class ShowFontWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ShowFontWindow(QWidget *parent = nullptr);
    ~ShowFontWindow();

    void showText(QString text,QPoint pos,QColor textColor);
private:
    QLabel* label;
    QGraphicsOpacityEffect* smoothFadeEffect;//透明度对象
    QPropertyAnimation* smoothFadeAnimation; //平滑淡出属性动画对象
    QPropertyAnimation* smoothMoveAnimation; //平滑移动属性动画对象

    void initWindowAttribute();                     //初始化窗口相关属性
    void initVariable();                            //初始化变量
    void initSmoothFade();                          //初始化平滑淡出
    void initSmoothMove();                          //初始化平滑移动
    void initConnect();                             //初始化连接
    void closeEvent(QCloseEvent* event) override;   //重写关闭事件
    Ui::ShowFontWindow *ui;
};

#endif // SHOWFONTWINDOW_H
