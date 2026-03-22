#ifndef PERFORMANCEWINDOW_H
#define PERFORMANCEWINDOW_H

#include <QWidget>
#include<QFile>
#include<QTimer>
#include<QCloseEvent>

//win和linux获取的单位不一样，win是B，而linux是KB，因此根据系统决定GB到底乘多少1024
#ifdef Q_OS_WIN
#define GB (1024 * 1024 * 1024)
#elif defined(Q_OS_UNIX)
#define GB (1024 * 1024)
#endif

#ifdef Q_OS_WIN
#include<minwindef.h>
#include<winnt.h>
#include<windows.h>
#endif
namespace Ui {
class PerformanceWindow;
}

class PerformanceWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PerformanceWindow(QWidget *parent = nullptr);
    ~PerformanceWindow();

    int getWindowWidth();                                         //获取性能窗口宽度
signals:
    void requestPerformanceWindowShow(QPoint windowPos);          //请求性能窗口显示
    void requestPerformanceWindowHide();                          //请求性能窗口隐藏
    void requestPerformanceWindowClose();                         //请求性能窗口关闭
    void requestPerformanceWindowMove(QPoint windowPos);          //请求窗口移动
    void requestUpdateMemoryProgressDisPlayStatus(bool isChecked);//请求更新内存进度条的显示状态
    void requestUpdateCpuProgressDisPlayStatus(bool isChecked);   //请求更新CPU进度条的显示状态
private:
    QTimer memoryTimer;                                        //内存定时器
    QTimer cpuTimer;                                           //CPU定时器
    bool windowIsClose;                                        //窗口是否关闭

#ifdef Q_OS_WIN
    ULONGLONG fristIdleTime ;                                  //第一组CPU状态(空闲，内核，用户时间)
    ULONGLONG fristKernelTime;
    ULONGLONG fristUserTime;

    ULONGLONG sencondIdleTime;                                 //第二组CPU状态
    ULONGLONG sencondKernelTime;
    ULONGLONG sencondUserTime;

    ULONGLONG concatenatedTimeDigits(DWORD low,DWORD hight);   //拼接时间的位数，将获取到的时间拼接成完整的64位
    void getWindowMemory(int& usedPresent,double& used);       //获取windows的内存情况
    void getCPUStatusTime(ULONGLONG* longIdleTime,ULONGLONG* longKernelTime,ULONGLONG* longUserTime);//获取CPU各个状态的时间, 定义能存储可运算的空闲，内核，用户时间结构体
    qreal calculateWindowsCpuUsage();                          //计算windows平台CPU使用率
#elif defined(Q_OS_UNIX)
    qulonglong fristTotal;                                     //第一次总时间
    qulonglong fristTotalIdle;                                 //第一次总空闲时间
    void getLinuxMemory(int& usedPresent,double& used);        //获取linux的内存情况
    void getLinuxCPUTimes(qulonglong& total,qulonglong& totalIdle); //获取linux的CPU时间值                                       //获取linux平台CPU的使用率
    qreal calculateLinuxCpuUsage();                                 //计算linux平台CPU使用率
#endif
    void initConnect();
    void initWindowAttribute();                                //初始化窗口属性
    void initMemoryProgressBar();                              //初始化内存进度条
    void initVariable();                                       //初始化变量

    void responseWindowShow(QPoint windowPos);                      //响应窗口显示
    void responseWindowHide();                                      //响应窗口隐藏
    void responseWindowClose();                                     //响应窗口关闭
    void responseUpdateMemroyProgressDisplayStatus(bool isChecked); //响应更新内存进度条显示状态
    void responseUpdateCpuProgressDisplayStatus(bool isChecked);    //响应更新CPU进度条显示状态

    void closeEvent(QCloseEvent* event) override;                   //重写关闭事件
    Ui::PerformanceWindow *ui;
};

#endif // PERFORMANCEWINDOW_H
