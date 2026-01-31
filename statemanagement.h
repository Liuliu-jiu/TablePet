#ifndef STATEMANAGEMENT_H
#define STATEMANAGEMENT_H

#include <QObject>
#include<QTimer>
#include<QDebug>
#include<QPoint>
#include<QGuiApplication>
#include<QScreen>
#include<QMouseEvent>
#include"logsystem.h"
#include"showfontwindow.h"
//站立状态宏定义
#define CAT_IDLE_PIXMAP_COUNT 12
#define CAT_IDLE_PATH_PREFIX ":/new/Idle/image/cat_Idle/cat_Idle_%1.png"

//行走状态宏定义
#define CAT_WALK_PIXMAP_COUNT 10
#define CAT_WALK_PATH_PREFIX ":/new/walk/image/cat_walk/cat_walk_%1.png"
#define MOVE_PIXEL 5

//受伤状态宏定义
#define CAT_HURT_PIXMAP_COUNT 6
#define CAT_HURT_PIXMAP_PREFIX ":/new/hurt/image/cat_hurt/cat_hurt_%1.png"
class StateManagement : public QObject
{
    Q_OBJECT
public:
    explicit StateManagement(QObject *parent = nullptr);

    void walk(int time);                     //行走动画方法
    void idle(int time);                     //站立动画方法
    void hurt(int time);                     //受伤动画方法

    void initVariable();                     //初始化变量(将接口暴露出来，确保先让mainwidget连接再初始化发射信号)
    int getCatIdlePixmapCount();             //获取小猫站立图片的总数
    int getCatWalkPixmapCount();             //获取小猫行走图片的总数
    int getCatHurtPixmapCount();             //获取小猫受伤图片的总数
    QString getCatIdlePixmapPathPrefix();    //获取小猫站立图片路径的前缀
    QString getCatWalkPixmapPathPrefix();    //获取小猫行走图片路径的前缀
    QString getCatHurtPixmapPathPrefix();    //获取小猫受伤图片路径的前缀
signals:
    void requestSwitchImage(QString ImagePath);     //请求切换图片信号
    QPoint requestGetWindowPos();                   //请求获取窗口相对于屏幕的坐标
    void requestSetWindowPos(QPoint windowPos);     //请求设置窗口坐标
    int requestGetWindowWidth();                    //请求获取窗口宽度
    void requestUpdateDirection(bool isRight);      //请求更新方向
    bool requestGetIsTriggerInjuryStatus();         //请求获取是否触发受伤状态(虽然信号通常是通知某件事件的发生，不应该具备返回值，但由于头文件包含问题，因此只能通过信号获取返回值)
private:
    QTimer walkTimer;                       //行走定时器
    QTimer IdleTimer;                       //站立定时器
    QTimer hurtTimer;                       //受伤定时器
    bool isRight;                           //窗口移动方向标志位
    QSize mainScreenSize;                   //主屏幕大小
    std::shared_ptr<LogSystem> logSystemPtr;//日志系统对象
    bool isShouldHurt;                      //是否应该受伤
    ShowFontWindow showFontWindow;          //字体显示窗口对象

    void initConnect();     //初始化连接
    void responseWalk();    //响应行走定时器timeout的方法
    void responseIdle();    //响应站立定时器timeout的方法
    void responsehurt();    //响应受伤定时器timeout的方法
    void decideAndSendPixmapPath(QString pathPrefix,int& currentIndex,int pixmalTotal);    //决定图片切换的路径
    void move(int moveX);   //移动窗口方法
    bool isArriveBorder(QPoint newWindowPos);  //判断是否到达边界方法

};

#endif // STATEMANAGEMENT_H
