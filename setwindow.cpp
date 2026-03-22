#include "setwindow.h"
#include "ui_setwindow.h"

SetWindow::SetWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SetWindow)
{
    //每个模型有两个相同的状态(行走和站立)，一个不同的状态(小猫受伤，骷髅攻击)
    //如果一个模型的状态太多，就不好写，比如骷髅士兵的出现和死亡动画与其它状态的切换逻辑过于复杂，就不写了
    ui->setupUi(this);

    //获取程序所在目录路径设置为当前工作目录，防止工作目录不同导致自启动时找不到资源导致启动失败
    QDir::setCurrent(QCoreApplication::applicationDirPath());

    initVariable();
    initWindowAttribute();
    initConnect();
    initButton();                  //初始化单选框状态
    initModelSetWindow();
}
SetWindow::~SetWindow()
{
    qDebug() << "SetWindow设置窗口析构";
    delete ui;
}
void SetWindow::initConnect()
{
    connect(this,&SetWindow::requestCloseWindow,this,&SetWindow::close);
}
void SetWindow::initButton()
{
    ui->cpuCheckBox->setCheckState(Qt::Checked);
    ui->memroyCheckBox->setCheckState(Qt::Checked);

    //由于快捷方式控件是为linux所设计，因此不是linux平台时，我将禁用该控件
#ifndef Q_OS_UNIX
    ui->shortcutCheckBox->setEnabled(false);
#else
    ui->shortcutCheckBox->setEnabled(true);
#endif
}
void SetWindow::initVariable()
{
    this->preloadptr = Preload::getPreloadObject();
    this->logSystemPtr = LogSystem::getLogSystemObject();
}
void SetWindow::initWindowAttribute()
{
    this->setWindowTitle("设置");
}
void SetWindow::initModelSetWindow()
{
    QGridLayout* layout = new QGridLayout(ui->scrollAreaWidgetContents);
    //用过滤器过滤json文件，遍历json文件，找到模型名字和预览图片，初始化模型设置窗口(QDirIterator::NoIteratorFlags表示只遍历指定目录，不包含子目录)
    //初始化单选框和label控件，初始化时根据json文件的个数开辟对应的单选框和label控件
    QDirIterator it(MODEL_JSON_FILE_DIRETORY_PATH,QStringList("*.json"),QDir::Files,QDirIterator::NoIteratorFlags);
    int col = 0;                             //模型所需要的控件放置的列数
    while(it.hasNext()){
        //通过哈希表关联模型名字与文件之间的关系，这样用户选择模型时，就能通过模型名字找到对应的文件路径了，用户选择时，就可以通过单选框的模型名找到json文件了
        QString jsonPath = it.next();
        if(jsonPath == APP_CONFIG_JSON_FILE){//如果json文件路径是项目配置文件，那么就直接进入下次循环，不创建新的单选框和label
            continue;
        }
        QString modelName = configFile.readConfigValue(jsonPath,"modelName").toString();
        this->modelAssociatedJsonHash.insert(modelName,jsonPath);

        //初始化单选框，让所有创建的单选框连接同一个方法，谁点击就将谁的地址传过来
        QString previewImagePath = configFile.readConfigValue(jsonPath,"previewImagePath").toString();
        QRadioButton* button = new QRadioButton(ui->scrollAreaWidgetContents);
        button->setText(modelName);
        connect(button,&QRadioButton::toggled,this,[=](bool isChecked){
            if(isChecked){
                bool isOk;
                configFile.writeConfig(APP_CONFIG_JSON_FILE,"userSelectedModelName",button->text(),ConfigFile::String,isOk);
                if(!isOk){
                    qWarning() << "userSelectedModelName更新失败！模型名：" << button->text();
                }
                loadModelConfig(button);
                loadOtherSet();             //根据模型更新最初可生成的位置
            }
        });
        //初始化预览图片
        int w = configFile.readConfigValue(jsonPath,"width").toInt()/2;
        int h = configFile.readConfigValue(jsonPath,"height").toInt()/2;
        QLabel* label = new QLabel(ui->scrollAreaWidgetContents);
        label->setPixmap(QPixmap(previewImagePath).scaled(w,h));

        layout->addWidget(label,0,col);     //label永远在第一排
        layout->addWidget(button,1,col);    //button永远在第二排
        col++;                              //每过一个模型列数加1，防止控件重叠
    }
    ui->scrollAreaWidgetContents->setLayout(layout);
}
void SetWindow::loadModelConfig(QRadioButton* button)
{
    emit this->preloadptr->requestClearHash();
    QString text = button->text();
    QString jsonPath = this->modelAssociatedJsonHash.value(text);

    //加载行走图片
    QStringList walkImagePathList = this->configFile.readConfigValue(jsonPath,"walkImagePath").toStringList();
    this->preloadptr->loadPixmap("walkImagePath",walkImagePathList);

    //加载站立图片
    QStringList idleImagePathList = this->configFile.readConfigValue(jsonPath,"idleImagePath").toStringList();
    this->preloadptr->loadPixmap("idleImagePath",idleImagePathList);

    //加载特殊状态，根据是否有特殊状态决定是否显示播放特殊状态的按钮
    QVariant specialStatusNameVariant = this->configFile.readConfigValue(jsonPath,"specialStatusName");
    if(specialStatusNameVariant.isNull()){
        emit this->requestUpdateSpecialActionDisplayStatus(false);
    }
    else{
        emit this->requestUpdateSpecialActionDisplayStatus(true);
        QStringList specialStatusImagePathList = this->configFile.readConfigValue(jsonPath,"specialStatusImagePath").toStringList();
        this->preloadptr->loadPixmap("specialStatusImage",specialStatusImagePathList);
        emit this->requestUpdateSpecialActionText(specialStatusNameVariant.toString());
        qDebug() << "特殊状态名：" << specialStatusNameVariant.toString();
        qDebug() << "特殊状态图片路径容器：" << specialStatusImagePathList;
    }

    //加载模型宽高
    int w = configFile.readConfigValue(jsonPath,"width").toInt()/2;
    int h = configFile.readConfigValue(jsonPath,"height").toInt()/2;
    emit this->reqeustAdjustWindowSize(QSize(w,h));
}
void SetWindow::setWindowAutoStart(bool isChecked)
{
    //1.进入注册表的开机自启动路径
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",QSettings::NativeFormat);

    //2.找到程序exe路径
    QString appFilePath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    QString appName = QCoreApplication::applicationName();//将路径分隔符转换为当前系统格式，windows分隔符

    //3.根据标志位看是否将该程序路径加入至开机自启动路径
    if(isChecked){
        settings.setValue(appName,appFilePath);
        qDebug() << "已将appName：" << appName << "  appFilePath：" << appFilePath << " 加入至开机自启动";
    }
    else{
        settings.remove(appName);
        qDebug() << "已将appName：" << appName << "  appFilePath：" << appFilePath << " 从开机自启动中移除";
    }
}
void SetWindow::loadOtherSet()
{
    //加载行走频率(初始化行走下拉框控件)
    QString walkTime = configFile.readConfigValue(APP_CONFIG_JSON_FILE,"walkFrequency").toString();
    ui->walkFrequencyComboBox->setCurrentText(walkTime);

    //加载站立呼吸频率(初始化站立呼吸下拉框控件)
    QString idleTime = configFile.readConfigValue(APP_CONFIG_JSON_FILE,"idleFrequency").toString();
    ui->idleFrequencyComboBox->setCurrentText(idleTime);

    //加载特殊状态频率值
    QString specialStatusTime = configFile.readConfigValue(APP_CONFIG_JSON_FILE,"specialStatusFrequency").toString();
    ui->specialStatusComboBox->setCurrentText(specialStatusTime);

    //加载最初生成位置(初始化最初生成位置控件)我希望x或y最大生成的位置能够使得窗口右边界或下边界正好贴在屏幕的右边界或下边界
    //因此获取的宽度和高度需要减去窗口的宽度和高度以致于窗口移动时能够达到效果
    QSize availableSize = QApplication::primaryScreen()->availableSize();
    QSize mainWindowSize = emit this->requestGetWindowSize();
    int birthMaxX = (availableSize.width() - mainWindowSize.width()) * 2;   //乘2是因为API获取的是逻辑分辨率，要转为物理分辨率供用户查看
    int birthMaxY = (availableSize.height() - mainWindowSize.height()) * 2;
    ui->xSpinBox->setRange(0,birthMaxX);
    ui->ySpinBox->setRange(0,birthMaxY);
    ui->maxSizeLabel->setText(QString(" 最大X：%1，最大Y：%2 ").arg(birthMaxX).arg(birthMaxY));
    ui->xSpinBox->setValue(configFile.readConfigValue(APP_CONFIG_JSON_FILE,"x").toInt());
    ui->ySpinBox->setValue(configFile.readConfigValue(APP_CONFIG_JSON_FILE,"y").toInt());
    emit this->requestUpdateWindowPos(QPoint(ui->xSpinBox->value()/2,ui->ySpinBox->value()/2)); //除2是为了将物理分辨率转为逻辑分辨率

    //加载最初状态
    QString status = configFile.readConfigValue(APP_CONFIG_JSON_FILE,"originalStatus").toString();
    ui->originalStatusComboBox->setCurrentText(status);
    if(status == "行走"){
        emit this->requestPlayWalkAnimation(ui->walkFrequencyComboBox->currentText().toInt());
    }
    else{
        emit this->requestPlayIdleAnimation(ui->idleFrequencyComboBox->currentText().toInt());
    }

    //加载最初方向
    QString direction = configFile.readConfigValue(APP_CONFIG_JSON_FILE,"originalDirection").toString();
    ui->originalDirectionComboBox->setCurrentText(direction);
    if(direction == "左"){
        emit this->requestUpdateWindowDirection(false);
    }
    else{
        emit this->requestUpdateWindowDirection(true);
    }

    //加载开机自启动
    bool isAutoStart = configFile.readConfigValue(APP_CONFIG_JSON_FILE,"isAutoStart").toInt();
    ui->autoStartCheckBox->setChecked(isAutoStart);

#ifdef Q_OS_UNIX
    //加载快捷方式状态
    bool isCreateShortcut = configFile.readConfigValue(APP_CONFIG_JSON_FILE,"isCreateShortcut").toInt();
    ui->shortcutCheckBox->setChecked(isCreateShortcut);
#endif
}
void SetWindow::setLinuxAutoStart(bool isChecked)
{
    //1.获取linux开机自启动路径
    //QStandardPaths::ConfigLocation是配置文件的枚举值，QStandardPaths::writableLocation是根据枚举值获取对应系统的标准路径
    //QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)表示根据配置文件的枚举值找对应系统配置文件的标准路径
    //QString autoStartPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/autoStart";
    //qDebug() << QStandardPaths::writableLocation(QStandardPaths::D);

    QString autoStartPath = "/etc/xdg/autostart";
    //如果当前目录不存在，则递归创建
    QDir dir(autoStartPath);
    if(!dir.exists()){
        dir.mkdir(".");                     //mkpath(".")代表创建指向的当前目录(autoStart)，关键特性是会递归创建，如果某个父目录不存在，那么会将该目录创建出来
    }

    QString appName = QCoreApplication::applicationName();
    QString appPath = QDir::fromNativeSeparators(QCoreApplication::applicationFilePath());
    //2.编写程序开机自启动内容
    QString string = QString(
        "[Desktop Entry]\n"                 //桌面文件入口标准开头
        "Type=Application\n"                //程序类型
        "Name=%1\n"                         //程序名称
        "Exec=%2\n"                         //程序路径
        "Hidden=false\n"                    //是否在菜单中隐藏
        "NoDisplay=false\n"                 //是否在应用列表中不显示
        "X-GNOME-Autostart-enabled=true\n").arg(appName).arg(appPath);//GNOME桌面(ubuntu)特定的自启动启用标志

    //3.将该内容放到开机自启动路径下的.desktop文件中
    QFile file(autoStartPath + "/.desktop");
    if(isChecked){
        if(file.open(QIODevice::WriteOnly)){
            file.write(string.toUtf8());
            QString successText = QString("已启用开机自启动，路径：%1" ).arg(file.fileName());
            qDebug() << successText;
            this->logSystemPtr->writeLog(LogSystem::Info,successText);
        }
        else{
            QString errorText = "启用开机自启动失败！路径：" + file.fileName();
            qDebug() << errorText;
            this->logSystemPtr->writeLog(LogSystem::Error,errorText);
        }
    }
    else{
        if(file.remove()){
            QString successText = "已禁用开机自启动，路径：" + file.fileName();
            qDebug() << successText;
            this->logSystemPtr->writeLog(LogSystem::Info,successText);
        }
        else{
            QString errorText = "禁用开机自启动失败！路径：" + file.fileName();
            qDebug() << errorText;
            this->logSystemPtr->writeLog(LogSystem::Error,errorText);
        }
    }
}
void SetWindow::setLinuxShortcut(bool isChecked)
{
    //进入桌面文件路径
    QString appLocationPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);

    QDir dir(appLocationPath);
    if(!dir.exists()){
        dir.mkdir(".");
    }

    //编写快捷方式文件内容
    QString appName = QCoreApplication::applicationName();
    QString appPath = QDir::fromNativeSeparators(QCoreApplication::applicationFilePath());
    QString string = QString(
                    "[Desktop Entry]\n"                     //桌面文件标准开头
                    "Name=%1\n"                             //程序名称
                    "Exec=%2\n"                             //程序路径
                    "Type=Application\n"                    //该软件类型，有Application和Link两种类型
                    "Terminal=false\n"                      //设置是否在终端运行，窗口应用程序填写false，命令行程序填true
                    "Icon=%3\n").arg(appName).arg(appPath).arg(ICON_PATH); //图标路径

    QFile file(appLocationPath + "/" + appName + ".desktop");
    //根据标志位决定是否创建.desktop文件
    if(isChecked){
        if(file.open(QIODevice::WriteOnly)){
            file.write(string.toUtf8());
            QString successText = ".desktop快捷方式创建成功！路径：" + appLocationPath;
            qDebug() << successText;
            this->logSystemPtr->writeLog(LogSystem::Info,successText);
        }
        else{
            QString errorText = ".desktop快捷方式创建失败！路径：" + appLocationPath;
            qDebug() << errorText;
            this->logSystemPtr->writeLog(LogSystem::Error,errorText);
        }
    }else{
        if(file.remove()){
            QString successText = "删除快捷方式成功，路径：" + file.fileName();
            qDebug() << successText;
            this->logSystemPtr->writeLog(LogSystem::Info,successText);
        }
        else{
            QString errorText = "删除快捷方式失败！路径：" + file.fileName();
            qDebug() << errorText;
            this->logSystemPtr->writeLog(LogSystem::Error,errorText);
        }
    }
}
void SetWindow::loadAppConfig()
{
    //读取配置文件，获取用户上一次选择的模型，根据模型名，遍历单选框，比较文本，触发对应的按钮
    QString userSelectedModelName = this->configFile.readConfigValue(APP_CONFIG_JSON_FILE,"userSelectedModelName").toString();
    QList<QRadioButton*> radioButtonList = this->findChildren<QRadioButton*>();
    foreach(QRadioButton* radioButton,radioButtonList){
        if(radioButton->text() == userSelectedModelName){
            radioButton->setChecked(true);
            emit radioButton->clicked(true);
            break;
        }
    }
}
void SetWindow::closeEvent(QCloseEvent* event)
{
    emit requestRestoreWindowMove();             //请求恢复窗口移动
    QWidget::closeEvent(event);
}
void SetWindow::on_memroyCheckBox_checkStateChanged(const Qt::CheckState &arg1)
{
    //更新性能窗口相关设置
    if(arg1 == Qt::Checked){
        qDebug() << "显示内存相关控件";
        emit requestUpdateMemoryProgressDisPlayStatus(true);
    }
    else{
        qDebug() << "隐藏内存相关控件";
        emit requestUpdateMemoryProgressDisPlayStatus(false);
    }
}
void SetWindow::on_cpuCheckBox_checkStateChanged(const Qt::CheckState &arg1)
{
    if(arg1 == Qt::Checked){
        qDebug() << "显示CPU相关控件";
        emit requestUpdateCpuProgressDisPlayStatus(true);
    }
    else{
        qDebug() << "隐藏CPU相关控件";
        emit requestUpdateCpuProgressDisPlayStatus(false);
    }
}
void SetWindow::on_walkFrequencyComboBox_currentTextChanged(const QString &arg1)
{
    //更新行走频率，更新定时器的时间并将值写入文件中
    int walkTime = arg1.toInt();
    emit this->requestPlayWalkAnimation(walkTime);
    bool isOk;
    configFile.writeConfig(APP_CONFIG_JSON_FILE,"walkFrequency",walkTime,ConfigFile::Int,isOk);
    if(isOk){
        qDebug() << "行走频率更新成功，值：" << walkTime << "ms";
    }
    else{
        QString errorText = QString("行走呼吸频率更新失败，值：%1 ms").arg(walkTime);
        this->logSystemPtr->writeLog(LogSystem::Error,errorText);
        qDebug() << errorText;
    }
}
void SetWindow::on_idleFrequencyComboBox_currentTextChanged(const QString &arg1)
{
    //更新站立呼吸频率，更新定时器的时间并将值写入文件中
    int idleTime = arg1.toInt();
    emit this->requestPlayIdleAnimation(idleTime);
    bool isOk;
    configFile.writeConfig(APP_CONFIG_JSON_FILE,"idleFrequency",idleTime,ConfigFile::Int,isOk);
    if(isOk){
        qDebug() << "站立呼吸频率更新成功，值：" << idleTime << "ms";
    }
    else{
        QString errorText = QString("站立呼吸频率更新失败，值：%1 ms").arg(idleTime);
        this->logSystemPtr->writeLog(LogSystem::Error,errorText);
        qDebug() << errorText;
    }
}
void SetWindow::on_xSpinBox_valueChanged(int arg1)
{
    //更新最初位置的x坐标
    bool isOk;
    configFile.writeConfig(APP_CONFIG_JSON_FILE,"x",arg1,ConfigFile::Int,isOk);
    if(isOk){
        qDebug() << "最初生成x坐标更新成功，值：" << arg1;
    }
    else{
        QString errorText = QString("最初生成x坐标更新失败，值：%1").arg(arg1);
        this->logSystemPtr->writeLog(LogSystem::Error,errorText);
        qDebug() << errorText;
    }
}
void SetWindow::on_ySpinBox_valueChanged(int arg1)
{
    //更新最初位置的y坐标
    bool isOk;
    configFile.writeConfig(APP_CONFIG_JSON_FILE,"y",arg1,ConfigFile::Int,isOk);
    if(isOk){
        qDebug() << "最初生成y坐标更新成功，值：" << arg1;
    }
    else{
        QString errorText = QString("最初生成y坐标更新失败，值：%1").arg(arg1);
        this->logSystemPtr->writeLog(LogSystem::Error,errorText);
        qDebug() << errorText;
    }
}
void SetWindow::on_originalStatusComboBox_currentTextChanged(const QString &arg1)
{
    //更新最初位置的y坐标
    bool isOk;
    configFile.writeConfig(APP_CONFIG_JSON_FILE,"originalStatus",arg1,ConfigFile::String,isOk);
    if(isOk){
        qDebug() << "最初状态更新成功，值：" << arg1;
    }
    else{
        QString errorText = QString("最初状态更新失败，值：%1 ").arg(arg1);
        this->logSystemPtr->writeLog(LogSystem::Error,errorText);
        qDebug() << errorText;
    }
}
void SetWindow::on_originalDirectionComboBox_currentTextChanged(const QString &arg1)
{
    bool isOk;
    configFile.writeConfig(APP_CONFIG_JSON_FILE,"originalDirection",arg1,ConfigFile::String,isOk);
    if(isOk){
        qDebug() << "最初方向更新成功，值：" << arg1;
    }
    else{
        QString errorText = QString("最初方向更新失败，值：%1 ").arg(arg1);
        this->logSystemPtr->writeLog(LogSystem::Error,errorText);
        qDebug() << errorText;
    }
}
void SetWindow::on_autoStartCheckBox_stateChanged(int arg1)
{
    //根据系统调用对应的自启动方法
#ifdef Q_OS_WIN
    setWindowAutoStart(arg1);
#elif defined(Q_OS_UNIX)
    setLinuxAutoStart(arg1);
#endif
    bool isOk;
    configFile.writeConfig(APP_CONFIG_JSON_FILE,"isAutoStart",arg1,ConfigFile::Int,isOk);
    if(isOk){
        qDebug() << "是否开机自启动值更新成功，值：" << (arg1 == 0 ? false : true);
    }
    else{
        QString errorText = QString("是否开机自启动值更新失败，值：%1 ").arg(arg1);
        this->logSystemPtr->writeLog(LogSystem::Error,errorText);
        qDebug() << errorText;
    }
}
void SetWindow::on_specialStatusComboBox_currentTextChanged(const QString &arg1)
{
    bool isOk;
    configFile.writeConfig(APP_CONFIG_JSON_FILE,"specialStatusFrequency",arg1.toInt(),ConfigFile::Int,isOk);
    if(isOk){
        qDebug() << "特殊状态频率更新成功，值：" << arg1;
    }
    else{
        QString errorText = QString("特殊状态频率更新失败，值：%1 ").arg(arg1);
        this->logSystemPtr->writeLog(LogSystem::Error,errorText);
        qDebug() << errorText;
    }
}
void SetWindow::on_shortcutCheckBox_stateChanged(int arg1)
{
    setLinuxShortcut(arg1);
    bool isOk;
    configFile.writeConfig(APP_CONFIG_JSON_FILE,"isCreateShortcut",arg1,ConfigFile::Int,isOk);
    if(isOk){
        qDebug() << "是否创建快捷方式更新成功，值：" << (arg1 == 0 ? false : true);
    }
    else{
        QString errorText = QString("是否创建快捷方式更新失败，值：%1 ").arg(arg1);
        this->logSystemPtr->writeLog(LogSystem::Error,errorText);
        qDebug() << errorText;
    }
}

