#include "statemanagement.h"

StateManagement::StateManagement(QObject *parent)
    : QObject{parent}
{
    initConnect();          //初始化连接
    initVariable();         //初始化变量
}
void StateManagement::walk(int time)
{
    this->IdleTimer.stop();
    this->specialStatusTimer.stop();
    this->walkTimer.start(time);    //开启行走定时器
}
void StateManagement::idle(int time)
{
    this->walkTimer.stop();
    this->IdleTimer.start(time);    //开启站立定时器
}
void StateManagement::specialStatus(int time)
{
    this->walkTimer.stop();
    this->specialStatusTimer.start(time);    //开启受伤定时器
}
void StateManagement::initConnect()
{
    connect(&this->walkTimer,&QTimer::timeout,this,&StateManagement::responseWalk);                  //连接行走定时器timeout信号
    connect(&this->IdleTimer,&QTimer::timeout,this,&StateManagement::responseIdle);                  //连接站立定时器timeout信号
    connect(&this->specialStatusTimer,&QTimer::timeout,this,&StateManagement::responseSpecialStatus);//连接受伤定时器timeout信号

    //请求更新方向
    connect(this,&StateManagement::requestUpdateDirection,this,[=](bool isRight){
        this->isRight = isRight;
    });
}
void StateManagement::initVariable()
{
    this->isRight = false;                                              //一开始往左移动
    emit requestUpdateDirection(this->isRight);                         //请求方向状态更新
    this->mainScreenSize = QGuiApplication::primaryScreen()->size();    //获取主屏幕大小
    this->logSystemPtr = LogSystem::getLogSystemObject();               //获取日志系统对象
    this->isShouldHurt = false;                                         //一开始不应该受伤
    this->preloadPtr = Preload::getPreloadObject();                     //获取预加载类对象指针
}
void StateManagement::responseWalk()
{
    //切换行走图片
    static int currentWalkCount = 0;    //决定切换至哪张图片
    decideAndSendPixmapPath(this->preloadPtr->getImageAndPathHash("walkImagePath",this->isRight),currentWalkCount);
    move(MOVE_PIXEL);
}
void StateManagement::responseIdle()
{
    //切换站立呼吸图片
    static int currentIdleCount = 1;    //决定切换至哪张图片
    decideAndSendPixmapPath(this->preloadPtr->getImageAndPathHash("idleImagePath",this->isRight),currentIdleCount);
}
void StateManagement::responseSpecialStatus()
{
    static int currentSpecialStatusCount = 0;
    static int currentSpecialStatusSwitchTime = 0;               //记录响应timeout的次数
    decideAndSendPixmapPath(this->preloadPtr->getImageAndPathHash("specialStatusImage",this->isRight),currentSpecialStatusCount);
    qDebug() << "currentSpecialStatusCount" << currentSpecialStatusCount;
    ++currentSpecialStatusSwitchTime;
    qDebug() << "currentSpecialStatusSwitchTime ：" << currentSpecialStatusSwitchTime;
    if(currentSpecialStatusSwitchTime >= this->preloadPtr->getImageAndPathHash("specialStatusImage",this->isRight).count()){   //当完整播放一遍后，重新切换为行走状态
        currentSpecialStatusSwitchTime = 0;                      //重新初始化为1，下次次数将从1开始
        this->isShouldHurt = false;                              //碰壁受伤过后，就不能重复受伤
        this->isRight = !(this->isRight);                        //下次移动时进行反弹
        int walkFrequency = configFile.readConfigValue(APP_CONFIG_JSON_FILE,"walkFrequency").toInt();
        this->walk(walkFrequency);
    }
}
bool StateManagement::decideAndSendPixmapPath(QHash<int,QPixmap> hash,int& currentIndex)
{
    //qDebug() << "图片索引：" << currentIndex;
    emit this->requestSwitchImage(hash.value(currentIndex++));//然后发送找到pixmap对象并发送至从主窗口处设置
    //如果索引超出了最大图片数，那么重新初始化为0
    if(currentIndex >= hash.size()){
        currentIndex = 0;
        return true;            //表示状态已经播放完了一遍
    }
    return false;
}
void StateManagement::move(int moveX)
{
    QPoint windowPos = emit requestGetWindowPos();  //获取原位置
    //qDebug() << "原位置：" << windowPos;
    if(this->isRight){                              //判断窗口移动方向，true为向右移动，反之为左
        windowPos.setX(windowPos.x() + moveX);      //向右移动窗口
    }
    else{
        windowPos.setX(windowPos.x() - moveX);      //向左移动窗口
    }
    //qDebug() << "新位置：" << windowPos;
    if(isArriveBorder(windowPos)){                 //判断新位置是否到达了左右边界

                                                   //为什么新位置到达左右边界时需要将窗口设置在各自的边界线上呢
                                                   //就是因为之前新位置到达左右边界外时，我的做法不能移动，下次移动时直接反弹
                                                   //这样造成的视觉效果就是还没完全碰到边界线时直接反弹，导致效果不是特别的好
                                                   //我希望完全碰壁之后再进行反弹，所以写了判断语句

        if(this->isRight){                         //如果到达了边界，就看行驶方向
            windowPos.setX(this->mainScreenSize.width()- (emit requestGetWindowWidth()));   //如果isRight为true，那么就说明一直往右走新位置到达了右边界外，需要将窗口的右边贴近右边界线
        }
        else{
            windowPos.setX(0);                     //如果isRight为false，说明一直往左边走新位置超出了左边界外，需要将位置设置在左边界线上
        }

        if(emit requestGetIsTriggerInjuryStatus()){//查看受伤选项的状态从而决定是否播放受伤动画
            this->isShouldHurt = true;
        }
        else{
            this->isShouldHurt = false;
        }

        //当完全碰壁后，我希望往原方向播放受伤动画，播放完后再更新状态
        if(!this->isShouldHurt){
            this->isRight = !(this->isRight);          //下次移动时进行反弹
            emit requestUpdateDirection(this->isRight);//请求更新方向状态
        }
    }
    emit requestSetWindowPos(windowPos);               //确定坐标后发送过去进行设置
    if(this->isShouldHurt){
        this->specialStatus(configFile.readConfigValue(APP_CONFIG_JSON_FILE,"specialStatusFrequency").toInt());
        showFontWindow.showText("hp - 1",emit requestGetWindowPos(),QColor(237,28,36));
    }
}
bool StateManagement::isArriveBorder(QPoint newWindowPos)
{
    //获取主屏幕信息
    if(newWindowPos.x() < 0 || (newWindowPos.x() + (emit requestGetWindowWidth())) > this->mainScreenSize.width()){     //判断是否到达了左右边界
        qDebug() << "窗口到达边界，下次往"<< (this->isRight ? "右" : "左") <<"移动，超出窗口的坐标：" << newWindowPos;
        this->logSystemPtr->writeLog(LogSystem::Info,QString("窗口到达边界，下次往")+(this->isRight ? "右" : "左") +QString("移动，超出窗口的坐标：(%1,%2)").arg(newWindowPos.x()).arg(newWindowPos.y()));
        return true;
    }
    return false;
}
