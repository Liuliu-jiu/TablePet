#include "custommodelwindow.h"
#include "ui_custommodelwindow.h"

CustomModelWindow::CustomModelWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CustomModelWindow)
{
    ui->setupUi(this);
    initWindowAttribute();
    initSpinBox();
}
CustomModelWindow::~CustomModelWindow()
{
    delete ui;
}
void CustomModelWindow::on_generatePushButton_clicked()
{
    if(valueIsEmpty()){     //检查所需要值是否为空
        return;
    }
    updateModelFile();      //更新配置文件
}
void CustomModelWindow::on_previewImagePushButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,"获取预览图片路径","","*.png");
    if(!filePath.isEmpty()){
        previewImagePath = "\"" + filePath + "\"";
        ui->previewImageLabel->setText(previewImagePath);
        qDebug() << "预览图片更新成功！路径：" << previewImagePath;
    }
}
void CustomModelWindow::on_walkImagePushButton_clicked()
{
    //更新行走状态图片路径
    QStringList walkImagePathList = QFileDialog::getOpenFileNames(this,"获取行走状态图片","","*.png");
    if(!walkImagePathList.isEmpty()){
        //将行走图片路径拼接成一个字符串，并设置在label上显示
        walkImagePath = concatenateFilePaths(walkImagePathList);
        setFilePathsToLabel(walkImagePath,ui->walkImageLabel);
        qDebug() << "站立图片路径更新成功！路径：" << idleImagePath;
    }
}
void CustomModelWindow::on_idleImagePushButton_clicked()
{
    //更新站立状态图片路径
    QStringList idleImagePathList = QFileDialog::getOpenFileNames(this,"获取站立图片","","*.png");
    if(!idleImagePathList.isEmpty()){
        idleImagePath = concatenateFilePaths(idleImagePathList);
        setFilePathsToLabel(idleImagePath,ui->idleImageLabel);
        qDebug() << "站立图片路径更新成功！路径：" << idleImagePath;
    }
}
void CustomModelWindow::on_specialImagePushButton_clicked()
{
    //更新特殊状态图片路径
    QStringList specialImagePathList = QFileDialog::getOpenFileNames(this,"获取特殊状态图片","","*.png");
    if(!specialImagePathList.isEmpty()){
        specialImagePath = concatenateFilePaths(specialImagePathList);
        setFilePathsToLabel(specialImagePath,ui->specialImageLabel);
        qDebug() << "特殊图片路径更新成功！路径：" << specialImagePath;
    }
}
void CustomModelWindow::initWindowAttribute()
{
    this->setWindowTitle("自定义配置窗口");
}
void CustomModelWindow::initSpinBox()
{
    //初始化spinbox的最小，最大值
    QSize availableSize = QApplication::primaryScreen()->availableSize();
    // ui->modelWidthSpinBox->setRange(0,availableSize.width());
    // ui->modelHeightSpinBox->setRange(0,availableSize.height());
    // ui->maxWidthLabel->setText(QString("最大：%1").arg(availableSize.width()));    //显示能设置的最大宽高，最大不能超过屏幕的4分之一
    // ui->maxHeightLabel->setText(QString("最大：%1").arg(availableSize.height()));
    ui->modelWidthSpinBox->setRange(0,availableSize.width()*2);
    ui->modelHeightSpinBox->setRange(0,availableSize.height()*2);
    ui->maxWidthLabel->setText(QString("最大：%1").arg(availableSize.width()*2));    //显示能设置的最大宽高，最大不能超过屏幕的4分之一
    ui->maxHeightLabel->setText(QString("最大：%1").arg(availableSize.height()*2));
}
void CustomModelWindow::promptError(QString errorText)
{
    //提示错误信息
    QMessageBox error(QMessageBox::Critical,"错误",errorText);
    error.exec();
}
QString CustomModelWindow::concatenateFilePaths(QStringList filePathList)
{
    QString result;
    //将多条路径拼接成一个字符串，路径与路径之间用逗号隔开
    for(int i = 0;i < filePathList.size();i++){
        result += ("\"" + filePathList[i]+ "\"");       //图片路径的首尾加双引号代表在json文件中这是一个字符串
        if(i < (filePathList.size() - 1)){              //当来到最后一个字符串时，末尾不需要加逗号
            result += ",\n";                            //加\n是为了在label上显示时更好看，写入json文件加不加都可以
        }
    }
    result.insert(0,"[");                               //在字符串首尾加左右中括号表示这是一个数组类型
    result.insert(result.size(),"]");
    return result;
}
void CustomModelWindow::setFilePathsToLabel(QString filePaths,QLabel* label)
{
    filePaths = filePaths.removeFirst();                //去除首尾中括号，方便展示到界面上
    filePaths = filePaths.removeLast();
    label->setText(filePaths);                          //将去除首尾后字符串设置在label上
}
bool CustomModelWindow::valueIsEmpty()
{
    //检查需要模型所需要的变量或控件的值是否为空
    if(ui->modelNameLineEdit->text().isEmpty()){
        promptError("模型名不能为空！");
        return true;
    }
    else if(ui->modelWidthSpinBox->value() <= 0){
        promptError("模型宽度不能为0！");
        return true;
    }
    else if(ui->modelHeightSpinBox->value() <= 0){
        promptError("模型高度不能为0！");
        return true;
    }
    else if(previewImagePath.isEmpty()){
        promptError("预览图片未导入！");
        return true;
    }
    else if(walkImagePath.isEmpty()){
        promptError("行走状态图片未导入！");
        return true;
    }
    else if(idleImagePath.isEmpty()){
        promptError("站立状态图片未导入！");
        return true;
    }
    //由于模型不一定有特殊状态，因此不需要对特殊状态控件值进行检测
    // else if(ui->specialStatusNameLineEdit->text().isEmpty()){
    //     promptError("特殊状态名字不能为空！");
    //     return true;
    // }
    // else if(specialImagePath.isEmpty()){
    //     promptError("特殊状态图片未导入！");
    //     return true;
    // }
    return false;
}
void CustomModelWindow::updateModelFile()
{
    //获取值
    int width = ui->modelWidthSpinBox->value();                       //获取宽
    int height = ui->modelHeightSpinBox->value();                     //获取高
    QString modelName = "\"" + ui->modelNameLineEdit->text() + "\"";                 //获取模型名

    //按照模型文件的格式填入对应的值
    //只要按照正确的方式嵌套值和闭合，不管有没有换行和空格，都能能通过toJson将字符串转换成带有格式的json数据写入至文件中
    QString jsonData = QString(
                           "{"
                           "\"width\": %1,"
                           "\"height\" : %2,"
                           "\"modelName\" : %3,"
                           "\"previewImagePath\" : %4,"
                           "\"walkImagePath\" : %5,"
                           "\"idleImagePath\" : %6"
                           "}").arg(width).arg(height).arg(modelName).arg(previewImagePath).arg(walkImagePath).arg(idleImagePath);

    //如果该模型有特殊状态，则将特殊状态属性加入至写入的json字符串中
    if(!ui->specialStatusNameLineEdit->text().isEmpty() && !specialImagePath.isEmpty()){
        QString specialStatusName = "\"" + ui->specialStatusNameLineEdit->text() + "\"";//获取特殊状态名
        QString specialJsonData = QString(","
                                          "\"specialStatus\" : {"
                                          "\"specialStatusName\" : %7,"
                                          "\"specialStatusImagePath\": %8"
                                          "}").arg(specialStatusName).arg(specialImagePath);
        //由于插入前，最后一位为右括号，我需要在右括号内且在最后一个属性后插入特殊状态属性，所以，我将特殊状态值插入至倒数第二位中
        //这样特殊状态值也就插入了右括号内，此时特殊状态也就成为了最后一个属性
        jsonData.insert(jsonData.size() - 1,specialJsonData);
        qDebug()<< "插入后：" << jsonData;
    }

    //写入文件
    QJsonParseError jsonParseError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonData.toUtf8(),&jsonParseError);
    if(jsonParseError.error != QJsonParseError::NoError){
        qDebug() << "生成json文件失败！错误信息：" << jsonParseError.errorString();
        promptError( "生成json文件失败！错误信息：" + jsonParseError.errorString());
        qDebug() << "jsonData：" << jsonData;
        return;
    }
    //提取模型名时由于要写入至json文件中，因此在首尾加了双引号，这就导致打开文件时会将双引号视为文件路径，从而导致失败
    //因此打开文件时，将双引号去除再打开
    modelName.remove(0,1);                      //将头尾的双引号删除，防止打开文件失败
    modelName.remove(modelName.size() - 1,1);

    QFile file(MODEL_JSON_FILE_DIRETORY_PATH + "/" + modelName + ".json");

    //判断是否在新建场景下，如果是则判断是否文件存在，防止其它模型文件被覆盖
    if(readFilePath.isEmpty() && QFile(file.fileName()).exists()){
        promptError(modelName + " 该模型文件已存在！请修改模型名");
        return;
    }

    //如果读取的路径和我生成时拼接的路径不一致，就代表模型名不一致，就将旧模型文件删除，防止新旧模型混合
    if(!readFilePath.isEmpty() && readFilePath != file.fileName()){
        QFile(readFilePath).remove();
        qDebug() << "模型名不一致！删除  " << readFilePath << "  旧模型文件";
        readFilePath = file.fileName();         //记录此次模型名，生成时便于判断模型名是否改变
    }

    if(!file.open(QIODevice::WriteOnly)){
        qDebug() << file.fileName() + " 文件打开失败！错误信息：" << file.errorString();
        promptError(file.fileName() + " 文件打开失败！错误信息：" + file.errorString());
        return;
    }
    file.write(jsonDocument.toJson());
    file.flush();
    file.close();
    qDebug() << "生成json文件成功！";
    QMessageBox info(QMessageBox::Information,"信息","生成配置文件成功！请重新启动加载模型");
    info.exec();
}
void CustomModelWindow::closeEvent(QCloseEvent* event)
{
    on_createModelFilePushButton_clicked();
}
void CustomModelWindow::on_editModelFilePushButton_clicked()
{
    //选取json文件
    QString jsonPath = QFileDialog::getOpenFileName(this,"获取模型文件",MODEL_JSON_FILE_DIRETORY_PATH,"*.json");
    if(jsonPath == APP_CONFIG_JSON_FILE){           //当用户选择的是项目文件时，我就让用户选择正确的模型文件
        promptError("文件选择错误！请选择正确的模型文件");
        on_editModelFilePushButton_clicked();
        return;
    }
    if(!jsonPath.isEmpty()){
        readFilePath = jsonPath;                    //记录读取json路径
        //将值全部读取来并放到对应的控件及变量中
        //读取模型名并设置在控件上
        QString modelName = configFile.readConfigValue(jsonPath,"modelName").toString();
        ui->modelNameLineEdit->setText(modelName);

        //读取宽
        int width = configFile.readConfigValue(jsonPath,"width").toInt();
        ui->modelWidthSpinBox->setValue(width);

        //读取高
        int height = configFile.readConfigValue(jsonPath,"height").toInt();
        ui->modelHeightSpinBox->setValue(height);

        //读取预览图片
        previewImagePath = ("\"" + configFile.readConfigValue(jsonPath,"previewImagePath").toString() + "\"");
        ui->previewImageLabel->setText(previewImagePath);

        //读取行走状态图片
        QStringList walkImageList = configFile.readConfigValue(jsonPath,"walkImagePath").toStringList();
        walkImagePath = concatenateFilePaths(walkImageList);
        setFilePathsToLabel(walkImagePath,ui->walkImageLabel);

        //读取行走状态图片
        QStringList idleImageList = configFile.readConfigValue(jsonPath,"idleImagePath").toStringList();
        idleImagePath = concatenateFilePaths(idleImageList);
        setFilePathsToLabel(idleImagePath,ui->idleImageLabel);

        //读取特殊状态名
        QString specialStatusName = configFile.readConfigValue(jsonPath,"specialStatusName").toString();
        ui->specialStatusNameLineEdit->setText(specialStatusName);

        //读取特殊状态图片
        QStringList specialStatusImagePath = configFile.readConfigValue(jsonPath,"specialStatusImagePath").toStringList();
        if(!specialStatusImagePath.isEmpty()){
            specialImagePath = concatenateFilePaths(specialStatusImagePath);
            setFilePathsToLabel(specialImagePath,ui->specialImageLabel);
        }
        else{
            specialImagePath = "";
            ui->specialImageLabel->setText("无");
        }
    }
}
void CustomModelWindow::on_createModelFilePushButton_clicked()
{
    //将控件和变量值全清空，为用户提供一个新建模板
    ui->modelNameLineEdit->clear();
    ui->modelWidthSpinBox->setValue(0);
    ui->modelHeightSpinBox->setValue(0);

    previewImagePath = "";
    ui->previewImageLabel->setText("无");

    walkImagePath = "";
    ui->walkImageLabel->setText("无");

    idleImagePath = "";
    ui->idleImageLabel->setText("无");

    ui->specialStatusNameLineEdit->clear();

    specialImagePath = "";
    ui->specialImageLabel->setText("无");

    readFilePath = "";
}
