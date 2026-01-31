#include "preload.h"

Preload::Preload(QObject *parent)
    : QObject{parent}
{}
void Preload::loadPixmap(QString pixmapPath,bool isMirrored)
{
    QHash<QString,QPixmap>::iterator it = this->pixmapHash.find(pixmapPath);    //首先查看该路径是否已经加载至容器中
    if(it == this->pixmapHash.end()){                                           //未加载则将路径加载至容器中
        this->pixmapHash.insert(pixmapPath,QPixmap(pixmapPath));
        if(isMirrored){
            this->mirrorPixmapHash.insert(pixmapPath,QPixmap::fromImage(QPixmap(pixmapPath).toImage().mirrored(true,false)));   //加载翻转图片
        }
    }
}
QPixmap Preload::getPixmap(QString pixmapPath,bool isGetMirrored)
{
    return isGetMirrored == true ? this->mirrorPixmapHash[pixmapPath] : this->pixmapHash[pixmapPath];           //根据参数获取不同方向的图片
}
