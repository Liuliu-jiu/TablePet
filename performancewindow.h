#ifndef PERFORMANCEWINDOW_H
#define PERFORMANCEWINDOW_H

#define GB (1024 * 1024 * 1024)

#include <QWidget>
#include<windows.h>
#include<QTimer>
#include<QCloseEvent>
#include<minwindef.h>
#include<winnt.h>
namespace Ui {
class PerformanceWindow;
}

class PerformanceWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PerformanceWindow(QWidget *parent = nullptr);
    ~PerformanceWindow();

    int getWindowWidth();                                      //获取性能窗口宽度
signals:
    void requestPerformanceWindowShow(QPoint windowPos);       //请求性能窗口显示
    void requestPerformanceWindowHide();                       //请求性能窗口隐藏
    void requestPerformanceWindowClose();                      //请求性能窗口关闭
    void requestPerformanceWindowMove(QPoint windowPos);       //请求窗口移动
private:
    QTimer memoryTimer;                                        //内存定时器
    QTimer cpuTimer;                                           //CPU定时器
    bool windowIsClose;                                        //窗口是否关闭

    ULONGLONG fristIdleTime ;                                  //第一组CPU状态(空闲，内核，用户时间)
    ULONGLONG fristKernelTime;
    ULONGLONG fristUserTime;

    ULONGLONG sencondIdleTime;                                 //第二组CPU状态
    ULONGLONG sencondKernelTime;
    ULONGLONG sencondUserTime;

    void initConnect();
    void initWindowAttribute();                                //初始化窗口属性
    void initMemoryProgressBar();                              //初始化内存进度条
    void initVariable();                                       //初始化变量

    void loadMemoryProgress();                                 //加载内存进度条(同时包含了获取内存和设置进度条值的操作)

    void loadCPUProgressBar();                                 //加载CPU进度条
    ULONGLONG concatenatedTimeDigits(DWORD low,DWORD hight);   //拼接时间的位数，将获取到的时间拼接成完整的64位
    void getCPUStatusTime(ULONGLONG* longIdleTime,ULONGLONG* longKernelTime,ULONGLONG* longUserTime);//获取CPU各个状态的时间, 定义能存储可运算的空闲，内核，用户时间结构体


    void responseWindowShow(QPoint windowPos);                 //响应窗口显示
    void responseWindowHide();                                 //响应窗口隐藏
    void responseWindowClose();                                //响应窗口关闭

    void closeEvent(QCloseEvent* event) override;              //重写关闭事件
    Ui::PerformanceWindow *ui;
};

#endif // PERFORMANCEWINDOW_H
