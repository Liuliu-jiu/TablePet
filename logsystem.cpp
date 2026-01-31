#include "logsystem.h"
std::shared_ptr<LogSystem> LogSystem::logSystemPtr = nullptr;
LogSystem::LogSystem(QObject *parent)
    : QObject{parent}
{
    initConnect();         //初始化连接
    openLogFile();         //打开日志文件
}
LogSystem::~LogSystem()
{
    this->writeLog(LogSystem::Info,this->file.fileName().toUtf8() + " 日志文件关闭");
    this->file.close();
    qDebug() << this->file.fileName() + " 日志文件关闭";
}
void LogSystem::initConnect()
{
    connect(&this->timer,&QTimer::timeout,this,[=](){
        this->file.flush(); //定时刷新缓冲区
    });
}
void LogSystem::writeLog(enum Grade grade,QString text)
{
    //格式：时间+等级+调试信息
    QString writeStr;

    //获取时间
    writeStr += QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss ");

    //根据等级判断这条信息的严重性，并附加至写入文件的变量中
    if(grade == LogSystem::Info){
        writeStr += "[Info] ";
    }
    else if(grade == LogSystem::Warning){
        writeStr += "[Warning] ";
    }
    else if(grade == LogSystem::Error){
        writeStr += "[Error] ";
    }

    //附加调试信息
    writeStr += text;
    // QTextStream out(&this->file);
    // out << writeStr << "\n";
    this->file.write(writeStr.toUtf8() + "\n");
}
std::shared_ptr<LogSystem> LogSystem::getLogSystemObject()
{
    if(logSystemPtr == nullptr){                                       //如果未开辟对应的空间
        logSystemPtr = std::shared_ptr<LogSystem>(new LogSystem());    //使用new开辟空间，将这块空间交给智能指针管理
    }
    return logSystemPtr;                                               //否则返回静态指针指向的地址使其不同源文件获取到同一个对象
}
void LogSystem::openLogFile()
{
    //日志文件路径：程序exe目录下 + "/" + 日志文件名
    //日志文件名：应用名+"_"+LogFile+"_"+日志文件索引.log
    QString logFileBaseName = readLogFileConfig();                                                     //获取上一次所使用的日志文件名(不带后缀)
    QString logFilePath = QCoreApplication::applicationDirPath()+"/"+logFileBaseName + ".log";         //拼接路径

    //检查大小是否超过100mb
    QFileInfo fileInfo(logFilePath);
    if(fileInfo.exists() && fileInfo.size() > 100 * MB){                                               //检查文件是否存在，如果存在则检查是否超出100mb，如果不存在，则不检查，直接创建文件
        qDebug() << logFilePath << " 日志文件超出100mb，即将更换文件";
                                                                                                       //更换日志文件(利用日志文件存在检测机制找到全新的日志文件，并将带后缀的路径返回来)
        logFilePath = logFileExistsMechanism(fileInfo.baseName().split("_").back().toInt());           //切割字符串拿到索引部分的值
        saveLogFileConfig(QFileInfo(logFilePath).baseName());                                          //更新日志配置文件

        qDebug() <<"更换后的日志文件路径：" << logFilePath;
    }

    this->file.setFileName(logFilePath);                                                               //确定文件路径
    if(!this->file.open(QIODevice::Append)){                                                           //以追加的方式打开
        QMessageBox::critical(QApplication::activeWindow(),"错误",this->file.fileName() + " 打开失败！");//将主窗口作为父窗口，自动管理生命周期
    }
    else{
        this->timer.start(5000);                                                                       //每5秒刷新缓冲区
        this->writeLog(LogSystem::Info,this->file.fileName() + " 日志文件打开成功！");
        qDebug() << this->file.fileName() << " 日志文件打开成功！";
    }
}
QString LogSystem::readLogFileConfig()
{
    //读取日志配置文件
    QFile configFile(CONFIG_FILE_PATH);
    if(!configFile.exists()){                               //当日志文件不存在时
        qDebug() << "配置文件不存在，即将重新创建";

        QString filePath = logFileExistsMechanism(0);       //从头(0)开始递增寻找未创建的日志文件路径(配置文件的消失，无法确定上次所使用的日志文件，所以从0递增，直到找到未创建的日志文件)
        saveLogFileConfig(QFileInfo(filePath).baseName());  //并且重新创建配置文件并将日志文件名写入进去

        qDebug() << "配置文件重新创建成功，本次使用日志文件路径：" << filePath;
    }
    if(!configFile.open(QIODevice::ReadOnly)){
       QMessageBox::critical(QApplication::activeWindow(),"错误",configFile.fileName() + " 打开失败！");
    }
    QString fileBaseName = configFile.readAll();
    configFile.close();
    return fileBaseName;
}
void LogSystem::saveLogFileConfig(QString fileBaseName)
{
    //更新配置文件的日志文件名(不带后缀)
    QFile configFile(CONFIG_FILE_PATH);
    configFile.open(QIODevice::WriteOnly);
    configFile.write(fileBaseName.toUtf8());
    configFile.close();
}
QString LogSystem::logFileExistsMechanism(int index)
{
    //通过索引递增寻找未创建的日志文件
    QString tempPath;
    do{
        //先转换为整数递增后加附加至字符串末尾，防止出现字符9加到字符冒号的问题出现
        tempPath = QCoreApplication::applicationDirPath() + "/" +QCoreApplication::applicationName() +"_LogFile_"+ QString::number(index++) + ".log";
    }while(QFile(tempPath).exists());
    return tempPath;
}
