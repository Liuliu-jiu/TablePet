#ifndef PRELOAD_H
#define PRELOAD_H

#include <QObject>
#include<QPixmap>
#include<QHash>
#include<QDebug>
class Preload : public QObject
{
    Q_OBJECT
public:
    static std::shared_ptr<Preload> getPreloadObject();                                   //获取预加载类实例对象指针，确保不同源文件获取同一个指针
    void loadPixmap(QString status,QStringList pixmapPathList);                           //加载图片资源
    QHash<int,QPixmap> getImageAndPathHash(QString status,bool isGetMirrored = false);    //获取关联图片路径和对象的哈希容器
signals:
    void requestClearHash();                                         //请求清除哈希容器信号
    void requestPromatQPixmapIsNull();                               //请求提示图片路径为空信号
private:
    static std::shared_ptr<Preload> preloadPtr;

    //哈希容器说明：<状态,<图片索引，图片对象>>，为了统一管理模型的状态，图片路径，图片对象之间的关系
    QHash<QString,QHash<int,QPixmap>> pixmapHash;                   //存储图片的容器
    QHash<QString,QHash<int,QPixmap>> mirrorPixmapHash;             //存储翻转后的图片容器的

    explicit Preload(QObject *parent = nullptr);                    //构造函数禁用，防止创建多个实例
    Preload(const Preload& p) = delete;                             //拷贝构造删除
    void initConnect();                                             //初始化连接
};

#endif // PRELOAD_H
