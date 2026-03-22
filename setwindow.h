#ifndef SETWINDOW_H
#define SETWINDOW_H

#include<QFileInfo>
#include<QDir>
#include <QWidget>
#include<QAbstractButton>
#include<QThread>
#include<QMessageBox>
#include<QPalette>
#include<QSurfaceFormat>
#include<QDirIterator>
#include<QRadioButton>
#include<QLabel>
#include<QGridLayout>
#include<QJsonObject>
#include<QScreen>
#include<QSettings>
#include<QStandardPaths>
#include"configfile.h"
#include"preload.h"
#include"logsystem.h"

//通过宏定义决定json文件的目录路径
#ifdef QT_NO_DEBUG
    //图标路径
    #define ICON_PATH QCoreApplication::applicationDirPath() + "/cat_Idle.ico"

    //模型JSON文件的目录路径
    #define MODEL_JSON_FILE_DIRETORY_PATH QCoreApplication::applicationDirPath()

    //项目配置文件路径
    #define APP_CONFIG_JSON_FILE QCoreApplication::applicationDirPath() + "/appConfig.json"
#else
    //图标路径
    #define ICON_PATH QFileInfo(__FILE__).absoluteDir().absolutePath() + "/cat_Idle.ico"

    //模型JSON文件的目录路径
    #define MODEL_JSON_FILE_DIRETORY_PATH QFileInfo(__FILE__).absoluteDir().absolutePath()

    //项目配置文件路径
    #define APP_CONFIG_JSON_FILE QFileInfo(__FILE__).absoluteDir().absolutePath() + "/appConfig.json"
#endif

namespace Ui {
class SetWindow;
}

class SetWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SetWindow(QWidget *parent = nullptr);
    ~SetWindow();

    void loadAppConfig();                                         //加载项目配置文件
    void loadOtherSet();                                          //加载其它设置
signals:
    void requestRestoreWindowMove();                              //请求恢复窗口移动信号
    void reqeustAdjustWindowSize(QSize windowSize);               //请求调整窗口大小信号
    void requestPlayWalkAnimation(int time);                      //请求播放行走动画
    void requestPlayIdleAnimation(int time);                      //请求播放站立动画
    void requestUpdateMemoryProgressDisPlayStatus(bool isChecked);//请求更新内存进度条的显示状态
    void requestUpdateCpuProgressDisPlayStatus(bool isChecked);   //请求更新CPU进度条的显示状态
    void requestUpdateSpecialActionDisplayStatus(bool isDisplay); //请求更新特殊状态按钮的显示情况
    void requestUpdateSpecialActionText(QString text);            //请求更新特殊状态按钮的文本
    void requestUpdateWindowPos(QPoint pos);                      //请求更新窗口位置
    void requestUpdateWindowDirection(bool isRight);              //请求更新窗口方向
    void requestCloseWindow();                                    //请求关闭设置窗口

    QSize requestGetWindowSize();                                 //请求获取窗口大小
private slots:
    void on_memroyCheckBox_checkStateChanged(const Qt::CheckState &arg1);
    void on_cpuCheckBox_checkStateChanged(const Qt::CheckState &arg1);

    void on_walkFrequencyComboBox_currentTextChanged(const QString &arg1);
    void on_idleFrequencyComboBox_currentTextChanged(const QString &arg1);

    void on_xSpinBox_valueChanged(int arg1);
    void on_ySpinBox_valueChanged(int arg1);

    void on_originalStatusComboBox_currentTextChanged(const QString &arg1);
    void on_originalDirectionComboBox_currentTextChanged(const QString &arg1);

    void on_autoStartCheckBox_stateChanged(int arg1);
    void on_specialStatusComboBox_currentTextChanged(const QString &arg1);

    void on_shortcutCheckBox_stateChanged(int arg1);

private:
    ConfigFile configFile;                                  //配置文件对象
    std::shared_ptr<Preload> preloadptr;                    //预加载类指针
    QHash<QString,QString> modelAssociatedJsonHash;         //模型关联Json文件的哈希容器
    std::shared_ptr<LogSystem> logSystemPtr;                //日志系统对象
    int walkTime;                                           //行走时间
    int idleTime;                                           //站立呼吸频率

    void initConnect();                                     //初始化连接
    void initButton();                                      //初始化各类按钮
    void initVariable();                                    //初始化变量
    void initWindowAttribute();                             //初始化窗口属性
    void initModelSetWindow();                              //初始化模型设置窗口

    void loadModelConfig(QRadioButton* button);             //加载模型配置

    void setWindowAutoStart(bool isChecked);                //设置windows开机自启动方法
    void setLinuxAutoStart(bool isChecked);                 //设置linux开机自启动方法
    void setLinuxShortcut(bool isChecked);                  //设置linux平台快捷方式

    void closeEvent(QCloseEvent* event) override;           //重写关闭事件
    Ui::SetWindow *ui;
};

#endif // SETWINDOW_H
