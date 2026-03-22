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
#ifdef Q_OS_WIN
ULONGLONG PerformanceWindow::concatenatedTimeDigits(DWORD low,DWORD hight)
{
    //将时间的两个32位拼接成一个完整的64位可运算时间
    ULARGE_INTEGER ull;
    ull.LowPart = low;      //低32位
    ull.HighPart = hight;   //高32位
    return ull.QuadPart;    //将拼接之后的结果返回来
}
void PerformanceWindow::getWindowMemory(int& usedPresent,double& used)
{
    MEMORYSTATUSEX memoryStatusEx;                              //声明结构体，用来存储物理和虚拟内存情况的结构体
    memoryStatusEx.dwLength = sizeof(memoryStatusEx);           //将dwLength初始化为结构体大小，用来确认结构体版本，防止写入错误
    if(GlobalMemoryStatusEx(&memoryStatusEx)){
        double total = memoryStatusEx.ullTotalPhys / (1.0*GB);  //总共物理空间，转换为GB单位存储在变量中
        double avail = memoryStatusEx.ullAvailPhys / (1.0*GB);  //可用物理空间
        used = total-avail;                                     //已用物理空间
        usedPresent = used/total * 100;                         //已用物理空间百分比

        qDebug() << "------------------";
        qDebug() << "总共物理内存：" << total;
        qDebug() << "已用物理内存：" << avail;
        qDebug() << "可用物理内存：" << used;
        qDebug() << "------------------";
    }
    else{
        qDebug() << "Get memory is fail！";
    }
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
qreal PerformanceWindow::calculateWindowsCpuUsage()
{
    //计算使用率时，程序设置的时间间隔和实际系统所使用的时间间隔不同，就会导致展示的使用率与系统的使用率略有差异
    getCPUStatusTime(&this->sencondIdleTime,&this->sencondKernelTime,&this->sencondUserTime);   //第二次采集数据
    ULONGLONG idle = this->sencondIdleTime - this->fristIdleTime;                               //通过第二次数据减去第一次数据得到1秒内空闲时间
    ULONGLONG kernel = this->sencondKernelTime - this->fristKernelTime;                         //1秒内内核时间
    ULONGLONG user = this->sencondUserTime - this->fristUserTime;                               //1秒内用户时间
    ULONGLONG total = kernel + user;                                                            //1秒内总时间
    ULONGLONG work = total - idle;                                                              //1秒内非空闲时间

    qreal usedPresend = (double)work/total*100;                                                 //非空闲时间在1秒内的占比

    this->fristIdleTime = this->sencondIdleTime;                                                //将这一次第二次采样的数据放到第一次数据的变量中，作为下次对比的第一次采样数据
    this->fristKernelTime = this->sencondKernelTime;
    this->fristUserTime = this->sencondUserTime;
    return usedPresend;
}
#elif defined(Q_OS_UNIX)
void PerformanceWindow::getLinuxMemory(int& usedPresent,double& used)
{
    //1.打开linux存储内存情况的虚拟文件
    QFile file("/proc/meminfo");
    if(!file.open(QIODevice::ReadOnly)){
        qWarning() << "/proc/meminfo文件打开失败！";
        return;
    }

    //2.读取内容
    QTextStream stream(&file);
    //memTotal内存总和，memFree空闲空间，memAvailable可用空间(包含缓存和缓冲区)，buffers缓冲区，cached缓存
    long memTotal = 0,memAvailable = 0;
    //读取虚拟文件meminfo时即使有内容atEnd方法也会返回true
    //原因：QFile读取的是真实文件，读虚拟文件时可能会导致指针位置不对，从而判断错误，即使调整指针位置也没用
    //解决方法：先将所有内容读取出来，然后按"\n"分割字符串，最后遍历
    //为什么按"\n"分割呢，内存的情况是按一行行排列的，我按"\n"分割，就相当于把内存情况的具体部分都分割了出来
    QStringList lins = stream.readAll().split("\n");
    foreach(QString l,lins){
        if(l.size() < 2){                                     //某些内存情况分割出来长度是小于2，而我需要的是大于等于2的字段(如内存总和)，因此如果小于2,直接跳到下一次循环
            continue;
        }
        QStringList list = l.split(":");                      //分割字符串，以便获取名字和字节数
        QString key = list.at(0).trimmed();                   //拿到具体信息名字
        QString value = list.at(1).trimmed().split(" ").at(0);//拿到字节数，先将前面的空格去除，再把单位去除
        //根据名字存储到对应的变量中
        if(key == "MemTotal"){
            memTotal = value.toLong();
        }
        else if(key == "MemAvailable"){
            memAvailable = value.toLong();
        }
    }

    //3.转换成百分比，通过带回型参数将百分比和使用的字节数带回来
    used = (static_cast<double>(memTotal - memAvailable))/GB;
    usedPresent = used/(memTotal/GB) * 100;
    qDebug() << "------------------";
    qDebug() << "总共物理内存：" << static_cast<double>(memTotal)/GB << " GB";
    qDebug() << "已用物理内存：" << used << " GB";
    qDebug() << "可用物理内存：" << static_cast<double>(memAvailable)/GB << " GB";
    qDebug() << "------------------";
}
void PerformanceWindow::getLinuxCPUTimes(qulonglong& total,qulonglong& totalIdle)
{
    //1.打开/proc/stat文件
    QFile file("/proc/stat");
    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "/proc/stat打开失败！";
        return;
    }

    //2.读取文件内容
    QString cpuTime = file.readLine();
    if(!cpuTime.startsWith("cpu ")){            //验证开头是否以"cpu "开头，确保读取的文件是对的，如果不是，证明文件不对或不存在
        qDebug() << "/proc/stat文件数据解析错误！";
        return;
    }
    file.close();

    //3.解析内容，获取各个时间值
    QStringList cpuTimeList = cpuTime.split(" ");
    cpuTimeList.pop_front();

    qulonglong user = cpuTimeList.at(0).toLongLong();       //用户态时间
    qulonglong nice = cpuTimeList.at(1).toLongLong();       //低优先级用户态时间
    qulonglong system = cpuTimeList.at(2).toLongLong();     //内核态时间
    qulonglong idle = cpuTimeList.at(3).toLongLong();       //空闲时间
    qulonglong iowait = cpuTimeList.at(4).toLongLong();     //I/O等待时间
    qulonglong irp = cpuTimeList.at(5).toLongLong();        //硬中断时间
    qulonglong softirp = cpuTimeList.at(6).toLongLong();    //软中断时间
    qulonglong steal = cpuTimeList.at(7).toULongLong();     //被偷走的时间

    //总时间就是8个值的总和
    total = user + nice + system + idle + iowait + irp + softirp + steal;
    totalIdle = idle + iowait;                              //总空闲时间则是CPU空闲时间+I/O等待时间
}
qreal PerformanceWindow::calculateLinuxCpuUsage()
{
    //计算两次采样中CPU的工作占比
    qulonglong sencondTotal = 0,sencondTotalIdle = 0;
    getLinuxCPUTimes(sencondTotal,sencondTotalIdle);

    qulonglong totalDiff = sencondTotal - fristTotal;
    qulonglong idleDiff = sencondTotalIdle - fristTotalIdle;
    qreal usedPresent = (static_cast<double>(totalDiff - idleDiff) / totalDiff) * 100;

    //将这一次的采样当作下一次的第一次采样，便于下次计算
    fristTotal = sencondTotal;
    fristTotalIdle = sencondTotalIdle;
    return usedPresent;
}
#endif
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
    connect(&this->memoryTimer,&QTimer::timeout,this,[=](){
        int present = 0;                //已使用的百分比
        double used = 0;                //已使用的字节数(GB)
#ifdef Q_OS_WIN
        getWindowMemory(present,used);
#elif defined(Q_OS_UNIX)
        getLinuxMemory(present,used);
#endif
        ui->memoryProgressBar->setValue(present);
        ui->showMemoryByteLabel->setText(QString(" 已用：%1G").arg(used,0,'f',2));//arg参数中，0代表总字段宽度，'f'代表定点表示法，4代表只保留4位小数
    });

    //定时更新CPU情况
    connect(&this->cpuTimer,&QTimer::timeout,this,[=](){
        qreal usedPresent = 0;
#ifdef Q_OS_WIN
        usedPresent = calculateWindowsCpuUsage();
#elif defined(Q_OS_UNIX)
        usedPresent = calculateLinuxCpuUsage();
#endif
        qDebug() << "CPU使用率：" << usedPresent;
        ui->cpuProgressBar->setValue(usedPresent);
    });

    //更新内存进度条显示状态
    connect(this,&PerformanceWindow::requestUpdateMemoryProgressDisPlayStatus,this,&PerformanceWindow::responseUpdateMemroyProgressDisplayStatus);

    //更新内存进度条显示状态
    connect(this,&PerformanceWindow::requestUpdateCpuProgressDisPlayStatus,this,&PerformanceWindow::responseUpdateCpuProgressDisplayStatus);
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
void PerformanceWindow::responseWindowShow(QPoint windowPos)
{
    //CPU定时器开启前采样一次，作位第一次采样的数据
#ifdef Q_OS_WIN
    getCPUStatusTime(&this->fristIdleTime,&this->fristKernelTime,&this->fristUserTime);
#elif defined(Q_OS_UNIX)
    getLinuxCPUTimes(fristTotal,fristTotalIdle);
#endif
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
    this->windowIsClose = true;     //将标志位改为true，说明要将性能窗口关闭而非隐藏
    this->close();
}
void PerformanceWindow::responseUpdateMemroyProgressDisplayStatus(bool isChecked)
{
    //根据内存多选框选中状态决定是否显示或隐藏
    if(isChecked){
        ui->memoryWidget->show();
    }
    else{
        ui->memoryWidget->hide();
    }
}
void PerformanceWindow::responseUpdateCpuProgressDisplayStatus(bool isChecked)
{
    //根据CPU多选框选中状态决定是否显示或隐藏
    if(isChecked){
        ui->cpuWidget->show();
    }
    else{
        ui->cpuWidget->hide();
    }
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
