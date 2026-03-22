#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include <QObject>
#include<QFile>
#include<QVariant>
#include<QJsonDocument>
#include<QJsonParseError>
#include<QJsonObject>
#include<QJsonValue>
#include<QFileInfo>
#include<QDir>
#include<QCoreApplication>

#ifdef QT_NO_DEBUG
    //项目配置文件路径
    #define APP_CONFIG_JSON_FILE QCoreApplication::applicationDirPath() + "/appConfig.json"
#else
    //项目配置文件路径
    #define APP_CONFIG_JSON_FILE QFileInfo(__FILE__).absoluteDir().absolutePath() + "/appConfig.json"
#endif
class ConfigFile : public QObject
{
    Q_OBJECT
public:
    explicit ConfigFile(QObject *parent = nullptr);
    enum JsonType{
        Int,
        String,
    };
    QVariant readConfigValue(QString jsonPath,QString key);                                     //读取配置文件不同类型的值，用QVariant进行封装后返回来
    void writeConfig(QString jsonPath,QString key,QVariant value,JsonType writeType,bool& isOk);//修改某个值
signals:

private:
    QJsonObject getRootJsonObject(QString jsonPath);             //获取根对象
    QVariant findValue(QJsonObject jsonObject, QString findKey); //查找某个值
    QJsonObject updateConfigValue(QJsonObject jsonObject,QString updateKey,QVariant value,JsonType jsonType,bool& isOk);//更新某个值并将新的json对象返回
};

#endif // CONFIGFILE_H
