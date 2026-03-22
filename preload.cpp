#include "preload.h"
std::shared_ptr<Preload> Preload::preloadPtr = nullptr;
Preload::Preload(QObject *parent)
    : QObject{parent}
{
    initConnect();
}
std::shared_ptr<Preload> Preload::getPreloadObject()
{
    if(preloadPtr == nullptr){
        preloadPtr = std::shared_ptr<Preload>(new Preload());
    }
    return preloadPtr;
}
void Preload::loadPixmap(QString status,QStringList pixmapPathList)
{
    QHash<QString,QHash<int,QPixmap>>::Iterator it = this->pixmapHash.find(status); //首先查看该状态是否已经加载至容器中
    if(it == this->pixmapHash.end()){                                               //未加载则将状态，图片路径和图片对象关联后加载至容器中
        QHash<int,QPixmap> hash;                                                    //存储原图片的容器
        QHash<int,QPixmap> mirrorHash;                                              //存储翻转图片的容器
        int i = 0;                                                                  //整数作为图片对象的键名，方便查找
        foreach(QString pixmapPath,pixmapPathList){
            QPixmap pixmap(pixmapPath);
            hash.insert(i,pixmap);                                                  //将图片路径和图片对象关联后放入至容器中存储
            mirrorHash.insert(i,QPixmap::fromImage(pixmap.toImage().mirrored(true,false)));//关联图片路径和水平翻转后的图片对象
            i++;
            qDebug() << "预加载的图片资源：" << pixmapPath;
            if(pixmap.isNull()){
                emit requestPromatQPixmapIsNull();
                qDebug() << "图片为空！路径" << pixmapPath;
            }
        }
        this->pixmapHash.insert(status,hash);                                       //关联状态和原容器
        this->mirrorPixmapHash.insert(status,mirrorHash);                           //关联状态和翻转容器
    }
}
QHash<int,QPixmap> Preload::getImageAndPathHash(QString status,bool isGetMirrored)
{
    return isGetMirrored == true ? this->mirrorPixmapHash.value(status) : this->pixmapHash.value(status);//根据布尔值选择要返回的哈希容器
}
void Preload::initConnect()
{
    connect(this,&Preload::requestClearHash,this,[=](){
        for(QHash<int,QPixmap>& hash : this->pixmapHash){
            hash.clear();                               //将里面的哈希容器内容清空
        }
        this->pixmapHash.clear();                       //清空外面哈希容器内容
        for(QHash<int,QPixmap>& mirrorHash : this->mirrorPixmapHash){
            mirrorHash.clear();
        }
        this->pixmapHash.clear();
    });
}
