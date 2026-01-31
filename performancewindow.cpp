#include "performancewindow.h"
#include "ui_performancewindow.h"

PerformanceWindow::PerformanceWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PerformanceWindow)
{
    ui->setupUi(this);
    initWindowAttribute();
    initMemoryProgressBar();
    initConnect();
}
PerformanceWindow::~PerformanceWindow()
{
    qDebug() << "PerformanceWindow性能窗口析构";
    delete ui;
}
int PerformanceWindow::getWindowWidth()
{
    return this->width();
}
void PerformanceWindow::initConnect()
{
    //响应请求性能窗口显示信号
    connect(this,&PerformanceWindow::requestPerformanceWindowShow,this,&PerformanceWindow::responseWindowShow);

    //响应请求性能窗口隐藏信号
    connect(this,&PerformanceWindow::requestPerformanceWindowHide,this,&PerformanceWindow::responseWindowHide);

    //响应请求性能窗口关闭信号
    connect(this,&PerformanceWindow::requestPerformanceWindowClose,this,&PerformanceWindow::responseWindowClose);

    //响应请求性能窗口移动信号
    connect(this,&PerformanceWindow::requestPerformanceWindowMove,this,[=](QPoint windowPos){
        this->move(windowPos);
    });

    //定时更新内存情况
    connect(&this->memoryTimer,&QTimer::timeout,this,&PerformanceWindow::loadMemoryProgress);

    //定时更新CPU情况
    connect(&this->cpuTimer,&QTimer::timeout,this,&PerformanceWindow::loadCPUProgressBar);
}
void PerformanceWindow::initWindowAttribute()
{
    this->setAttribute(Qt::WA_TranslucentBackground);                           //设置透明背景
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);   //设置隐藏标题栏和边框并且使得窗口一直置于顶层
}
void PerformanceWindow::initMemoryProgressBar()
{
    ui->memoryProgressBar->setRange(0,100);                     //最小最大值跟内存的最小最大百分比一致
}
void PerformanceWindow::initVariable()
{
    this->windowIsClose = true;                                 //窗口一开始是关闭状态
}
void PerformanceWindow::loadMemoryProgress()
{
    MEMORYSTATUSEX memoryStatusEx;                              //声明结构体，用来存储物理和虚拟内存情况的结构体
    memoryStatusEx.dwLength = sizeof(memoryStatusEx);           //将dwLength初始化为结构体大小，用来确认结构体版本，防止写入错误
    if(GlobalMemoryStatusEx(&memoryStatusEx)){
        double total = memoryStatusEx.ullTotalPhys / (1.0*GB);  //总共物理空间，转换为GB单位存储在变量中
        double avail = memoryStatusEx.ullAvailPhys / (1.0*GB);  //已用物理空间
        double used = total-avail;                              //可用物理空间
        int usedPercentage = used/total * 100;                  //可用物理空间百分比

        qDebug() << "总共物理内存：" << total;
        qDebug() << "已用物理内存：" << avail;
        qDebug() << "可用物理内存：" << used;
        qDebug() << "------------------";

        ui->memoryProgressBar->setValue(usedPercentage);
        ui->showMemoryByteLabel->setText(QString(" 已用：%1G").arg(used,0,'f',2));//arg参数中，0代表总字段宽度，'f'代表定点表示法，4代表只保留4位小数
    }
    else{
        qDebug() << "Get memory is fail！";
    }
}
void PerformanceWindow::loadCPUProgressBar()
{
    //计算使用率时，程序设置的时间间隔和实际系统所使用的时间间隔不同，就会导致展示的使用率与系统的使用率略有差异
    getCPUStatusTime(&this->sencondIdleTime,&this->sencondKernelTime,&this->sencondUserTime);   //第二次采集数据
    ULONGLONG idle = this->sencondIdleTime - this->fristIdleTime;                               //通过第二次数据减去第一次数据得到1秒内空闲时间
    ULONGLONG kernel = this->sencondKernelTime - this->fristKernelTime;                         //1秒内内核时间
    ULONGLONG user = this->sencondUserTime - this->fristUserTime;                               //1秒内用户时间
    ULONGLONG total = kernel + user;                                                            //1秒内总时间
    ULONGLONG work = total - idle;                                                              //1秒内非空闲时间

    ui->cpuProgressBar->setValue((double)work/total*100);                                       //非空闲时间在1秒内的占比
    qDebug()<< "workPrecent：" << (double)work/total*100.0;

    this->fristIdleTime = this->sencondIdleTime;                                                //将这一次第二次采样的数据放到第一次数据的变量中，作为下次对比的第一次采样数据
    this->fristKernelTime = this->sencondKernelTime;
    this->fristUserTime = this->sencondUserTime;
}
ULONGLONG PerformanceWindow::concatenatedTimeDigits(DWORD low,DWORD hight)
{
    //将时间的两个32位拼接成一个完整的64位可运算时间
    ULARGE_INTEGER ull;
    ull.LowPart = low;      //低32位
    ull.HighPart = hight;   //高32位
    return ull.QuadPart;    //将拼接之后的结果返回来
}
void PerformanceWindow::getCPUStatusTime(ULONGLONG* longIdleTime,ULONGLONG* longKernelTime,ULONGLONG* longUserTime)
{
    _FILETIME idleTime,kernelTime,userTime;                                                            //定义第一次能存储空闲时间，内核时间，用户时间的结构体
    if(GetSystemTimes(&idleTime,&kernelTime,&userTime)){
        *longIdleTime = concatenatedTimeDigits(idleTime.dwLowDateTime,idleTime.dwHighDateTime);        //拼接成可运算的时间，然后通过参数带回值
        *longKernelTime = concatenatedTimeDigits(kernelTime.dwLowDateTime,kernelTime.dwHighDateTime);
        *longUserTime = concatenatedTimeDigits(userTime.dwLowDateTime,userTime.dwHighDateTime);
    }
    else{
        qDebug() << "Get cpu status is fail！";
    }
}
void PerformanceWindow::responseWindowShow(QPoint windowPos)
{
    getCPUStatusTime(&this->fristIdleTime,&this->fristKernelTime,&this->fristUserTime);//CPU定时器开启前采样一次，作位第一次采样的数据
    this->cpuTimer.start(1000);     //每1秒更新一次CPU使用情况

    this->windowIsClose = false;
    this->memoryTimer.start(1000);  //每一秒更新一次内存情况
    this->move(windowPos);          //将窗口移动至指定的位置
    this->show();                   //再进行显示
}
void PerformanceWindow::responseWindowHide()
{
    this->memoryTimer.stop();       //隐藏时定时器关闭，防止性能浪费
    this->cpuTimer.stop();
    this->hide();
}
void PerformanceWindow::responseWindowClose()
{
    this->windowIsClose = true;        //将标志位改为true，说明要将性能窗口关闭而非隐藏
    this->close();
}
void PerformanceWindow::closeEvent(QCloseEvent* event)
{
    if(this->windowIsClose){
        QWidget::closeEvent(event);     //如果标志位为true，则响应关闭事件
    }
    else{
        event->ignore();                //忽略关闭事件，防止性能窗口意外关闭
    }
}
