#include "configfile.h"

ConfigFile::ConfigFile(QObject *parent)
    : QObject{parent}
{}
QVariant ConfigFile::readConfigValue(QString jsonPath,QString key)
{
    //获取根元素对象
    QJsonObject object = getRootJsonObject(jsonPath);
    return findValue(object,key);
}
void ConfigFile::writeConfig(QString jsonPath,QString key,QVariant value,JsonType writeType,bool& isOk)
{
    isOk = false;
    QJsonObject jsonObject = updateConfigValue(getRootJsonObject(APP_CONFIG_JSON_FILE),key,value,writeType,isOk);
    QFile writeFile(APP_CONFIG_JSON_FILE);
    if(!writeFile.open(QIODevice::WriteOnly)){
        qWarning() << "打开配置文件失败，路径：" << jsonPath;
        return;
    }
    writeFile.write(QJsonDocument(jsonObject).toJson());
    writeFile.flush();
    writeFile.close();
}
QJsonObject ConfigFile::updateConfigValue(QJsonObject jsonObject,QString updateKey,QVariant value,JsonType jsonType,bool& isOk)
{
    //一次递归视作一个对象，在这个对象下遍历所有键，如果与我的键相符，则修改对应的值，然后往前更新，返回QJsonObject
    //如果该键是一个object对象，则进入到这个对象遍历，如果没有则退出来继续往下遍历
    //当遍历所有键后未找到修改的键时，则返回原对象
    QJsonObject newObject;
    foreach(QString key ,jsonObject.keys()){
        //遍历过程中发现是个对象则继续递归遍历
        if(jsonObject.value(key).isObject()){
            newObject = updateConfigValue(jsonObject.value(key).toObject(),updateKey,value,jsonType,isOk);
            //当函数返回时，当找到该键并更新时，就将该对象的值进行一个更新，更新完后返回
            if(isOk){
                jsonObject[key] = newObject;
                break;
            }
        }
        else{
            //当找到需要更新的键时，就按照传入类型转换成对应的类型更新对应的值
            //然后退出循环将当前对象进行返回
            if(key == updateKey){
                switch(jsonType){
                case Int:
                    jsonObject[key] = value.toInt();
                    break;
                case String:
                    jsonObject[key] = value.toString();
                    break;
                }
                isOk = true;
                break;
            }
        }
    }
    //qDebug()<< "修改后的对象值：" << jsonObject;
    return jsonObject;
}
QJsonObject ConfigFile::getRootJsonObject(QString jsonPath)
{
    //打开配置文件
    QFile configFile(jsonPath);
    if(!configFile.open(QIODevice::ReadOnly)){
        qWarning() << "打开配置文件失败，路径：" << jsonPath;
        return QJsonObject();
    }

    //读取并解析数据
    QJsonParseError parseError;                                                //存储json解析的错误信息
    QByteArray jsonData = configFile.readAll();                                //读取json数据
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonData,&parseError);//利用fromJson解析数据

    configFile.close();

    //验证解析数据是否成功
    if(parseError.error != QJsonParseError::NoError){
        qWarning() << "Json数据解析错误！路径：" << jsonPath;
        return QJsonObject();
    }

    //验证数据格式是否正确(json格式的根元素通常是对象或者是数组类型)
    if(!jsonDocument.isObject()){
        qWarning() << "Json数据格式不符合要求！路径：" << jsonPath;
        return QJsonObject();
    }

    //获取根元素对象
    return jsonDocument.object();
}
QVariant ConfigFile::findValue(QJsonObject jsonObject, QString findKey)
{
    //递归寻找键，将对应的值返回来
    QVariant variant;                                                       //存储不同类型值的对象
    foreach(QString key,jsonObject.keys()){
        if(jsonObject.value(key).isObject()){
            variant = findValue(jsonObject.value(key).toObject(),findKey);
            if(!variant.isNull()){
                break;
            }
        }
        else{
            if(key == findKey){
                variant.setValue(jsonObject[key].toVariant());              //将数据封装至QVariant对象返回
                break;
            }
        }
    }
    return variant;
}
