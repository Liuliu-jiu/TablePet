#ifndef LOGSYSTEM_H
#define LOGSYSTEM_H

#define MB 1024 * 1024
#define CONFIG_FILE_PATH QCoreApplication::applicationDirPath() + "/" +QCoreApplication::applicationName() +"_LogFile_Config" + ".txt"

#include <QObject>
#include<QDateTime>
#include<QFile>
#include<QCoreApplication>
#include<QMessageBox>
#include<QApplication>
#include<QFileInfo>
#include<QTimer>
class LogSystem : public QObject
{
    Q_OBJECT
public:
    enum Grade{                 //调试信息等级，暴露出来的枚举可以像访问静态变量那样访问
        Info,                   //一般信息
        Warning,                //警告信息
        Error                   //错误信息
    };

    ~LogSystem();
    void writeLog(enum Grade grade,QString text);            //写入日志
    static std::shared_ptr<LogSystem> getLogSystemObject();  //获取日志系统对象静态方法
signals:
private:
    static std::shared_ptr<LogSystem> logSystemPtr;          //静态指针，用于指向自己
    QFile file;                                              //文件对象
    QTimer timer;                                            //定时器

    explicit LogSystem(QObject *parent = nullptr);           //将构造函数变为私有，防止创建多个对象
    LogSystem(const LogSystem& logSystem) = delete;          //将拷贝构造删了，防止利用拷贝构造创建出不同的实例
    void initConnect();                                      //初始化连接
    void openLogFile();                                      //打开日志文件方法
    QString readLogFileConfig();                             //读取日志配置文件
    void saveLogFileConfig(QString fileBaseName);            //保存日志文件配置
    QString logFileExistsMechanism(int index);               //日志文件存在检测机制
    void checkfileIsExists(QString filePath);                //检查文件是否存在
};

#endif // LOGSYSTEM_H
