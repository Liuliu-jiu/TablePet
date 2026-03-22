#include "mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    initVariable();                     //初始化变量
    initConnect();                      //初始化连接
    initWindowAttribute();              //初始化窗口相关属性
    s.initVariable();                   //先让mainwidget连接再初始化发射信号，防止信号发送时信号还没连接
    this->setWindowPtr->loadAppConfig();//先让主窗口连接再初始化，防止发送信号时未连接
    //this->w.show();
    //qDebug() << " \"width\":1 ";        //用\将"从表示字符串类型转换为输出内容
}
MainWidget::~MainWidget()
{
    qDebug() << "MainWidget主窗口析构";
    delete ui;
}
void MainWidget::initConnect()
{
    connect(&s,&StateManagement::requestSwitchImage,this,[=](QPixmap pixmap){
        //当加载的模型路径无效时，此时无法正常显示窗口，如果不修改路径则影响用户体验
        //因此当路径无效时，可显示配置窗口使得用户修改路径
        ui->label->setPixmap(pixmap.scaled(this->width(),this->height()));    //通过信号传递图片对象设置图片
    });

    connect(&s,&StateManagement::requestGetWindowPos,this,[=](){
        return this->pos();         //获取窗口位置坐标
    });

    connect(&s,&StateManagement::requestSetWindowPos,this,[=](QPoint windowPos){
        //我希望在单击桌面宠物(开始拖拽时)，右键菜单或打开设置窗口时停止移动
        if(!this->isPress && !this->isRightMenu && !this->isOpenSetWindow){
            this->move(windowPos);  //移动窗口
            if(this->performanceWindowAction->isChecked()){                   //如果性能窗口选项处于选中状态，性能窗口会不断处于桌面宠物下方
                emit this->performanceWindow.requestPerformanceWindowMove(calculatePerformanceWindowPos(windowPos));
            }
        }
    });

    connect(&s,&StateManagement::requestGetWindowWidth,this,[=](){
        return this->width();       //返回窗口宽度
    });

    connect(this->rightClickMenu,&QMenu::aboutToHide,this,[=](){
        this->isRightMenu = false;  //当菜单要隐藏之前，窗口恢复移动
    });

    connect(&s,&StateManagement::requestGetIsTriggerInjuryStatus,this,[=](){
        return this->specialStatusAction->isChecked();  //返回受伤选项选中状态
    });

    connect(this->setAction,&QAction::triggered,this,[=](){
        this->isOpenSetWindow = true;
        this->setWindowPtr->show();
    });

    connect(this->closeAction,&QAction::triggered,this,&MainWidget::close);  //当点击关闭选项时，就将桌面宠物关闭

    //响应显示自定义窗口信号
    connect(this->customModelAction,&QAction::triggered,&this->w,&CustomModelWindow::show);

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

    connect(this->setWindowPtr.get(),&SetWindow::reqeustAdjustWindowSize,this,[=](QSize windowSize){
        qDebug() << "重置大小：" << windowSize;
        this->resize(windowSize);                       //重置大小
        ui->label->resize(windowSize);
    });

    connect(this->setWindowPtr.get(),&SetWindow::requestPlayWalkAnimation,this,[=](int time){
        this->isWalk = true;
        s.walk(time);                                   //播放行走动画
    });

    connect(this->setWindowPtr.get(),&SetWindow::requestPlayIdleAnimation,this,[=](int time){
        this->isWalk = false;
        s.idle(time);                                   //播放站立动画
    });

    //请求更新内存进度条的显示状态
    connect(this->setWindowPtr.get(),&SetWindow::requestUpdateMemoryProgressDisPlayStatus,&this->performanceWindow,&PerformanceWindow::requestUpdateMemoryProgressDisPlayStatus);

    //请求更新CPU进度条的显示状态
    connect(this->setWindowPtr.get(),&SetWindow::requestUpdateCpuProgressDisPlayStatus,&this->performanceWindow,&PerformanceWindow::requestUpdateCpuProgressDisPlayStatus);

    //请求更新特殊状态按钮的显示情况
    connect(this->setWindowPtr.get(),&SetWindow::requestUpdateSpecialActionDisplayStatus,this,[=](bool isDisplay){
        this->specialStatusAction->setVisible(isDisplay);
        if(!isDisplay){                 //如果是为隐藏状态，则将选择状态设置为false，防止没有特殊动画的模型播放特殊动画
            this->specialStatusAction->setChecked(false);
        }
    });

    //请求更新特殊状态按钮的文本
    connect(this->setWindowPtr.get(),&SetWindow::requestUpdateSpecialActionText,this,[=](QString text){
        this->specialStatusAction->setText(text);
    });

    //请求返回窗口大小
    connect(this->setWindowPtr.get(),&SetWindow::requestGetWindowSize,this,[=](){
        return this->size();
    });

    //请求更新窗口位置
    connect(this->setWindowPtr.get(),&SetWindow::requestUpdateWindowPos,this,[=](QPoint pos){
        this->move(pos);
    });

    //请求更新窗口方向
    connect(this->setWindowPtr.get(),&SetWindow::requestUpdateWindowDirection,&this->s,&StateManagement::requestUpdateDirection);

    connect(this->preloadPtr.get(),&Preload::requestPromatQPixmapIsNull,this,[=](){
        //当图片模型为空时，将设置和配置窗口显示出来，并提醒用户重新设置
        if(this->setWindowPtr->isHidden() || this->w.isHidden()){       //当窗口显示时，就不在重新显示和提示
            this->setWindowPtr->show();
            this->w.show();
            QMessageBox::critical(this,"错误","该模型图片路径无效，请重新设置！");
        }
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
    this->preloadPtr = Preload::getPreloadObject();                                      //获取预加载对象

    this->rightClickMenu = new QMenu(tr("右键菜单"),this);                                //开辟右键菜单的空间
    this->specialStatusAction = this->rightClickMenu->addAction("");                     //添加特殊状态按钮选项
    this->performanceWindowAction = this->rightClickMenu->addAction("性能窗口");          //添加性能窗口选项
    this->customModelAction = this->rightClickMenu->addAction("自定义模型");              //添加自定义模型选项
    this->setAction = this->rightClickMenu->addAction("设置");                           //添加设置选项
    this->closeAction = this->rightClickMenu->addAction("关闭");                         //添加关闭选项

    this->specialStatusAction->setCheckable(true);                                      //将特殊选项改为拥有可选择状态特性
    this->performanceWindowAction->setCheckable(true);                                  //将性能窗口选项改为拥有可选择状态特性

    this->setWindowPtr = std::make_shared<SetWindow>();                                 //开辟设置窗口空间
    this->isOpenSetWindow = false;
    this->isPress = false;
    this->isRightMenu = false;
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
        int walkFrequency = configFile.readConfigValue(APP_CONFIG_JSON_FILE,"walkFrequency").toInt();
        s.walk(walkFrequency);
    }
    else{
        int idleFrequency = configFile.readConfigValue(APP_CONFIG_JSON_FILE,"idleFrequency").toInt();
        s.idle(idleFrequency);
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
        else if(windowPos.x() + this->width() > this->avaliableSize.width()){     //检测是否超出右边界
            windowPos.setX(this->avaliableSize.width()- this->width());
        }
        if(windowPos.y() < 0){                                                    //检测是否超出上边界
            windowPos.setY(0);
        }
        else if(windowPos.y() + this->height() > this->avaliableSize.height()){   //检测是否超出下边界
            windowPos.setY(this->avaliableSize.height() - this->height());
        }
        this->logSystemPtr->writeLog(LogSystem::Info,QString("用户拖拽的坐标：(%1,%2)").arg(windowPos.x()).arg(windowPos.y()));
        this->move(windowPos);

        if(this->performanceWindowAction->isChecked()){                           //如果性能窗口选项处于选中状态，性能窗口会不断处于桌面宠物下方
            emit this->performanceWindow.requestPerformanceWindowMove(calculatePerformanceWindowPos(windowPos));
        }
    }
}
void MainWidget::closeEvent(QCloseEvent* event)
{
    emit this->performanceWindow.requestPerformanceWindowClose();                 //关闭主窗口前先关闭性能窗口
    emit this->setWindowPtr->requestCloseWindow();
    QWidget::closeEvent(event);
}
