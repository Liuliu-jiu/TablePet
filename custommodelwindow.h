#ifndef CUSTOMMODELWINDOW_H
#define CUSTOMMODELWINDOW_H

#include <QWidget>
#include<QScrollArea>
#include<QMessageBox>
#include<QJsonDocument>
#include<QFile>
#include<QFileInfo>
#include<QDir>
#include<QFileDialog>
#include<QScreen>
#include"configfile.h"

//通过宏定义决定json文件的目录路径
#ifdef QT_NO_DEBUG
    //模型JSON文件的目录路径
    #define MODEL_JSON_FILE_DIRETORY_PATH QCoreApplication::applicationDirPath()
#else
//模型JSON文件的目录路径
    #define MODEL_JSON_FILE_DIRETORY_PATH QFileInfo(__FILE__).absoluteDir().absolutePath()
#endif
namespace Ui {
class CustomModelWindow;
}

class CustomModelWindow : public QWidget
{
    Q_OBJECT

public:
    explicit CustomModelWindow(QWidget *parent = nullptr);
    ~CustomModelWindow();

private slots:
    void on_generatePushButton_clicked();
    void on_previewImagePushButton_clicked();
    void on_walkImagePushButton_clicked();
    void on_idleImagePushButton_clicked();
    void on_specialImagePushButton_clicked();
    void on_editModelFilePushButton_clicked();

    void on_createModelFilePushButton_clicked();

private:
    Ui::CustomModelWindow *ui;

    QString previewImagePath;               //预览图片路径
    QString walkImagePath;                  //行走状态图片路径，不仅是json数据，也是显示在label控件上的字符串
    QString idleImagePath;                  //站立状态图片路径
    QString specialImagePath;               //特殊状态图片路径
    ConfigFile configFile;                  //配置文件对象
    QString readFilePath;                   //读取的文件路径，用来查看模型名是否修改从而决定是否删除旧模型文件

    void initWindowAttribute();             //初始化窗口属性
    void initSpinBox();
    void promptError(QString errorText);    //提示错误
    QString concatenateFilePaths(QStringList filePathList);     //拼接文件路径
    void setFilePathsToLabel(QString filePaths,QLabel* label);  //设置多条文件路径到label
    bool valueIsEmpty();
    void updateModelFile();                                     //更新配置文件

    void closeEvent(QCloseEvent* event) override;
};

#endif // CUSTOMMODELWINDOW_H
