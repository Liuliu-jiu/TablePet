#include "showfontwindow.h"
#include "ui_showfontwindow.h"
#include<QLabel>
ShowFontWindow::ShowFontWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ShowFontWindow)
{
    ui->setupUi(this);

    initVariable();
    initWindowAttribute();
    initSmoothFade();
    initSmoothMove();
    initConnect();
}
ShowFontWindow::~ShowFontWindow()
{
    qDebug() << "ShowFontWindow显示字体窗口析构";
    delete ui;
}
void ShowFontWindow::showText(QString text,QPoint pos,QColor textColor)
{
    //设置label相关属性
    this->label->setText(text);                                             //设置文本
    this->label->setFont(QFont("Microsoft YaHei UI",15));                   //设置字体和字体大小
    QPalette palette = this->label->palette();                              //获取现有调色板
    palette.setColor(QPalette::WindowText,textColor);                       //设置文本颜色
    this->label->setPalette(palette);                                       //应用文本颜色
    this->label->adjustSize();                                              //调整label控件大小，不调用这行代码会导致控件的大小值不能更新，即使控件能够自适应字体大小
    this->label->update();

    //设置窗口大小，使窗口跟控件大小一致
    this->setFixedSize(this->label->size());

    //设置移动的开始和结束位置
    int windowY = pos.y() - this->label->height();                          //将字体窗口的起始位置移动至距离桌面宠物窗口上边框的label控件高度距离，防止窗口重叠
    this->smoothMoveAnimation->setStartValue(QPoint(pos.x(),windowY));
    this->smoothMoveAnimation->setEndValue(QPoint(pos.x(),windowY - 20));

    this->show();                                                           //窗口显示

    //开启平滑淡出和移动动画化
    this->smoothFadeAnimation->start();
    this->smoothMoveAnimation->start();
}
void ShowFontWindow::initWindowAttribute()
{
    this->setAttribute(Qt::WA_TranslucentBackground);                           //设置透明背景
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);   //设置隐藏标题栏和边框并且使得窗口一直置于顶层
}
void ShowFontWindow::initVariable()
{
    this->label = new QLabel(this);
}
void ShowFontWindow::initSmoothFade()
{
    this->smoothFadeEffect = new QGraphicsOpacityEffect(this->label);                                   //创建透明度对象，并操作label控件的透明度
    this->label->setGraphicsEffect(smoothFadeEffect);                                                   //将透明度对应用至label控件中，每次渲染时都会经过该对象渲染

    this->smoothFadeAnimation = new QPropertyAnimation(this->smoothFadeEffect,"opacity",this->label);   //创建属性动画对象，并指定动画化的对象和属性
    this->smoothFadeAnimation->setDuration(SMOOTH_FADE_DURATION);                                       //设置时长
    this->smoothFadeAnimation->setStartValue(1.0);                                                      //设置开始值，1.0为完全不透明
    this->smoothFadeAnimation->setEndValue(0);                                                          //设置结束值。0为完全透明
    this->smoothFadeAnimation->setEasingCurve(QEasingCurve::InOutQuad);                                 //设置缓动曲线，QEasingCurve::InOutQuad代表开始末尾时会加速，使其更加平滑，默认匀速
}
void ShowFontWindow::initSmoothMove()
{
    this->smoothMoveAnimation = new QPropertyAnimation(this,"pos",this);                                //创建动画化对象，并操作窗口的坐标
    this->smoothMoveAnimation->setDuration(SMOOTH_MOVE_DURATION);                                       //时长为2秒
}
void ShowFontWindow::initConnect()
{
    connect(this->smoothMoveAnimation,&QPropertyAnimation::finished,this,[=](){
        this->hide();           //当最后一个动画播放完后隐藏窗口
    });
}
void ShowFontWindow::closeEvent(QCloseEvent* event)
{
    event->ignore();            //忽略关闭事件防止动画播放时窗口关闭
}
