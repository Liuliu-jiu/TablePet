#include "mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    initVariable();         //初始化变量
    initConnect();          //初始化连接
    initWindowAttribute();  //初始化窗口相关属性
    initPixmap();           //初始化图片

    s.initVariable();       //先让mainwidget连接再初始化发射信号，防止信号发送时信号还没连接
    s.walk(80);
    //s.Idle(100);
}
MainWidget::~MainWidget()
{
    qDebug() << "MainWidget主窗口析构";
    delete ui;
}

void MainWidget::initConnect()
{
    connect(&s,&StateManagement::requestSwitchImage,this,[=](QString pixmapPath){
        //qDebug() << "切换的图片路径：" << pixmapPath;
        //qDebug() << "this->isRight：" <<this->isRight;
        ui->label->setPixmap(p.getPixmap(pixmapPath,this->isRight).scaled(WIDTH,HEIGHT));    //通过信号传递路径设置图片
    });

    connect(&s,&StateManagement::requestGetWindowPos,this,[=](){
        return this->pos();         //获取窗口位置坐标
    });

    connect(&s,&StateManagement::requestSetWindowPos,this,[=](QPoint windowPos){
        //我希望在单击桌面宠物，右键菜单或打开设置窗口时停止移动
        if(!this->isPress && !this->isRightMenu && !this->isOpenSetWindow){
            this->move(windowPos);  //移动窗口
            if(this->performanceWindowAction->isChecked()){   //如果性能窗口选项处于选中状态，性能窗口会不断处于桌面宠物下方
                emit this->performanceWindow.requestPerformanceWindowMove(calculatePerformanceWindowPos(windowPos));
            }
        }
    });

    connect(&s,&StateManagement::requestGetWindowWidth,this,[=](){
        return this->width();       //返回窗口宽度
    });

    connect(&s,&StateManagement::requestUpdateDirection,this,[=](bool isRight){
        this->isRight = isRight;    //方向状态更新
    });

    connect(this->rightClickMenu,&QMenu::aboutToHide,this,[=](){
        this->isRightMenu = false;  //当菜单要隐藏之前，窗口恢复移动
    });

    connect(&s,&StateManagement::requestGetIsTriggerInjuryStatus,this,[=](){
        return this->triggerInjuryAction->isChecked();  //返回受伤选项选中状态
    });

    connect(this->setAction,&QAction::triggered,this,[=](){
        this->isOpenSetWindow = true;
        this->setWindowPtr->show();
    });

    connect(this->closeAction,&QAction::triggered,this,&MainWidget::close);      //当点击关闭选项时，就将桌面宠物关闭

    //响应点击性能窗口事件
    connect(this->performanceWindowAction,&QAction::triggered,this,[=](bool checked){
        if(checked){
            emit this->performanceWindow.requestPerformanceWindowShow(calculatePerformanceWindowPos(this->pos()));
        }else{
            emit this->performanceWindow.requestPerformanceWindowHide();
        }
    });

    connect(this->setWindowPtr.get(),&SetWindow::requestRestoreWindowMove,this,[=](){
        this->isOpenSetWindow = false;                  //恢复窗口移动
    });
}
void MainWidget::initWindowAttribute()
{
    this->setAttribute(Qt::WA_TranslucentBackground);                           //设置透明背景
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);   //设置隐藏标题栏和边框并且使得窗口一直置于顶层
}
void MainWidget::initVariable()
{
    this->isWalk = true;                                                                 //一开始是行走状态
    this->avaliableSize = QGuiApplication::primaryScreen()->availableSize();             //获取可用空间大小(不包括任务栏)
    this->logSystemPtr = LogSystem::getLogSystemObject();                                //获取日志系统对象

    this->rightClickMenu = new QMenu(tr("右键菜单"),this);                                //开辟右键菜单的空间
    this->triggerInjuryAction = this->rightClickMenu->addAction("行走碰壁触发受伤状态");    //添加触发受伤选项
    this->performanceWindowAction = this->rightClickMenu->addAction("性能窗口");          //添加性能窗口选项
    this->setAction = this->rightClickMenu->addAction("设置");                           //添加设置选项
    this->closeAction = this->rightClickMenu->addAction("关闭");                         //添加关闭选项

    this->triggerInjuryAction->setCheckable(true);                                      //将受伤选项改为可选中状态
    this->performanceWindowAction->setCheckable(true);                                  //将性能窗口选项改为可选中状态

    this->setWindowPtr = std::make_shared<SetWindow>();                                 //开辟设置窗口空间
    this->isOpenSetWindow = false;
    this->isPress = false;
    this->isRightMenu = false;
}
void MainWidget::initPixmap()
{
    //加载小猫站立图片
    for(int i = 1;i<=s.getCatIdlePixmapCount();i++){
        p.loadPixmap(QString(s.getCatIdlePixmapPathPrefix()).arg(i),true);
        qDebug() << "预加载的图片资源路径：" << QString(s.getCatIdlePixmapPathPrefix()).arg(i);
        this->logSystemPtr->writeLog(LogSystem::Info,"预加载的图片资源路径：" + QString(s.getCatIdlePixmapPathPrefix()).arg(i));
    }
    //加载小猫行走图片
    for(int i = 1;i<=s.getCatWalkPixmapCount();i++){
        p.loadPixmap(QString(s.getCatWalkPixmapPathPrefix()).arg(i),true);
        qDebug() << "预加载的图片资源路径：" << QString(s.getCatWalkPixmapPathPrefix()).arg(i);
        this->logSystemPtr->writeLog(LogSystem::Info, "预加载的图片资源路径：" + QString(s.getCatWalkPixmapPathPrefix()).arg(i));
    }
    //加载小猫受伤图片
    for(int i = 1;i<=s.getCatHurtPixmapCount();i++){
        p.loadPixmap(QString(s.getCatHurtPixmapPathPrefix()).arg(i),true);
        qDebug() << "预加载的图片资源路径：" << QString(s.getCatHurtPixmapPathPrefix()).arg(i);
        this->logSystemPtr->writeLog(LogSystem::Info, "预加载的图片资源路径：" + QString(s.getCatHurtPixmapPathPrefix()).arg(i));
    }
}
QPoint MainWidget::calculatePerformanceWindowPos(QPoint mainWidgetPos)
{
    //计算性能窗口的坐标，使其性能窗口中心部分处于桌面宠物下方
    QPoint performanceWindowPos = mainWidgetPos;
    int value = (this->performanceWindow.getWindowWidth()-this->width())/2;
    performanceWindowPos.setX(mainWidgetPos.x() - value);
    performanceWindowPos.setY(mainWidgetPos.y() + this->height());
    return performanceWindowPos;
}
void MainWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    //双击切换状态
    this->isWalk = !(this->isWalk);
    qDebug() << "双击切换后的状态：" << (this->isWalk ? "行走" : "站立");

    //重载运算符+没有准备两个char字符串相加的情况，因此无法拼接，重载运算符+准备的场景有两个QString拼接或一个QString一个char字符串拼接
    //所以为了达到拼接的要求，将其中char字符串一个转为QString后，使用重载运算符+就能将char字符串转为QString字符串后拼接
    this->logSystemPtr->writeLog(LogSystem::Info,QString("双击切换后的状态：") + (this->isWalk ? "行走" : "站立"));
    if(this->isWalk){
        s.walk(80);
    }
    else{
        s.idle(150);
    }
}
void MainWidget::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton){
        this->isPress = true;                       //禁止窗口移动
        this->eventPos = event->pos();              //获取鼠标在窗口中的坐标
    }
    else if(event->button() == Qt::RightButton){
        this->isRightMenu = true;                   //当右键菜单时，窗口禁止移动
        this->rightClickMenu->move(QCursor::pos()); //移动至鼠标基于屏幕的位置
        this->rightClickMenu->show();
    }
    else{
        QWidget::mousePressEvent(event);
    }
}
void MainWidget::mouseReleaseEvent(QMouseEvent* event)
{
    this->isPress = false;              //允许窗口移动
}
void MainWidget::mouseMoveEvent(QMouseEvent* event)
{
    //拖拽功能
    if(this->isPress && event->buttons() == Qt::LeftButton){        //移动时是否按的是左键
        QPoint windowPos = QCursor::pos() - this->eventPos;         //获取鼠标相对于屏幕的坐标，减去鼠标相对于窗口的坐标，得出窗口左上角的坐标
        if(windowPos.x() < 0){                                      //检测是否超出左边界
            windowPos.setX(0);                                      //将窗口边框设置在边界线上
        }
        else if(windowPos.x() + WIDTH > this->avaliableSize.width()){     //检测是否超出右边界
            windowPos.setX(this->avaliableSize.width()- WIDTH);
        }
        if(windowPos.y() < 0){                                            //检测是否超出上边界
            windowPos.setY(0);
        }
        else if(windowPos.y() + HEIGHT > this->avaliableSize.height()){   //检测是否超出下边界
            windowPos.setY(this->avaliableSize.height() - HEIGHT);
        }
        this->logSystemPtr->writeLog(LogSystem::Info,QString("用户拖拽的坐标：(%1,%2)").arg(windowPos.x()).arg(windowPos.y()));
        this->move(windowPos);

        if(this->performanceWindowAction->isChecked()){                   //如果性能窗口选项处于选中状态，性能窗口会不断处于桌面宠物下方
            emit this->performanceWindow.requestPerformanceWindowMove(calculatePerformanceWindowPos(windowPos));
        }
    }
}
void MainWidget::closeEvent(QCloseEvent* event)
{
    emit this->performanceWindow.requestPerformanceWindowClose();         //关闭主窗口前先关闭性能窗口
    QWidget::closeEvent(event);
}
