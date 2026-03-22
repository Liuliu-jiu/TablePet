#ifndef STATEMANAGEMENT_H
#define STATEMANAGEMENT_H

#include <QObject>
#include<QTimer>
#include<QDebug>
#include<QPoint>
#include<QGuiApplication>
#include<QScreen>
#include<QMouseEvent>
#include<QtConcurrent/QtConcurrentRun>
#include"logsystem.h"
#include"showfontwindow.h"
#include"preload.h"
#include"configfile.h"

//移动像素宏定义
#define MOVE_PIXEL 5

class StateManagement : public QObject
{
    Q_OBJECT
public:
    explicit StateManagement(QObject *parent = nullptr);

    void walk(int time);                       //行走动画方法
    void idle(int time);                       //站立动画方法
    void specialStatus(int time);              //特殊状态动画方法

    void initVariable();                       //初始化变量(将接口暴露出来，确保先让mainwidget连接再初始化发射信号)
signals:
    void requestSwitchImage(QPixmap pixmap);   //请求切换图片信号
    QPoint requestGetWindowPos();              //请求获取窗口相对于屏幕的坐标
    void requestSetWindowPos(QPoint windowPos);//请求设置窗口坐标
    int requestGetWindowWidth();               //请求获取窗口宽度
    void requestUpdateDirection(bool isRight); //请求更新方向
    bool requestGetIsTriggerInjuryStatus();    //请求获取是否触发受伤状态(虽然信号通常是通知某件事件的发生，不应该具备返回值，但由于头文件包含问题，因此只能通过信号获取返回值)
private:
    QTimer walkTimer;                        //行走定时器
    QTimer IdleTimer;                        //站立定时器
    QTimer specialStatusTimer;               //特殊状态定时器
    bool isRight;                            //窗口移动方向标志位
    QSize mainScreenSize;                    //主屏幕大小
    std::shared_ptr<LogSystem> logSystemPtr; //日志系统对象
    bool isShouldHurt;                       //是否应该受伤
    ShowFontWindow showFontWindow;           //字体显示窗口对象
    std::shared_ptr<Preload> preloadPtr;     //预加载类对象指针
    ConfigFile configFile;

    void initConnect();                      //初始化连接
    void responseWalk();                     //响应行走定时器timeout的方法
    void responseIdle();                     //响应站立定时器timeout的方法
    void responseSpecialStatus();            //响应特殊状态定时器timeout的方法
    bool decideAndSendPixmapPath(QHash<int,QPixmap> hash,int& currentIndex);    //决定图片切换的路径
    void move(int moveX);                    //移动窗口方法
    bool isArriveBorder(QPoint newWindowPos);//判断是否到达边界方法

};

#endif // STATEMANAGEMENT_H
