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
    explicit Preload(QObject *parent = nullptr);

    void loadPixmap(QString pixmapPath,bool isMirrored = false);        //加载图片资源
    QPixmap getPixmap(QString pixmapPath,bool isGetMirrored = false);      //获取图片资源对象
signals:
private:
    QHash<QString,QPixmap> pixmapHash;          //存储图片的容器
    QHash<QString,QPixmap> mirrorPixmapHash;    //存储翻转后的图片容器的
};

#endif // PRELOAD_H
