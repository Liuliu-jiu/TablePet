#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#define WIDTH 70
#define HEIGHT 80

#include <QWidget>
#include<QMenu>
#include"statemanagement.h"
#include"preload.h"
#include"logsystem.h"
#include"setwindow.h"
#include"performancewindow.h"
//class Preload;  //前向声明，cpp文件包含头文件能够防止头文件重复包含
namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();
private:
    StateManagement s;                      //状态管理对象
    Preload p;                              //预加载对象
    bool isWalk;                            //是否行走
    bool isRight;                           //方向，是否向右(翻转后的方向)
    bool isPress;                           //是否单击按压
    QPoint eventPos;                        //鼠标坐标
    QSize avaliableSize;                    //可用空间大小
    std::shared_ptr<LogSystem> logSystemPtr;//日志系统对象
    QMenu* rightClickMenu;                  //右键菜单
    QAction* triggerInjuryAction;           //触发受伤选项
    QAction* setAction;                     //设置选项
    QAction* closeAction;                   //关闭选项
    QAction* performanceWindowAction;       //性能窗口选项
    std::shared_ptr<SetWindow> setWindowPtr;//设置窗口指针对象
    bool isOpenSetWindow;                   //是否打开设置窗口
    bool isRightMenu;                       //是否右键了菜单
    PerformanceWindow performanceWindow;

    void initConnect();                     //初始化连接
    void initWindowAttribute();             //初始化窗口属性
    void initVariable();                    //初始化变量
    void initPixmap();                      //初始化图片
    QPoint calculatePerformanceWindowPos(QPoint mainWidgetPos);//计算性能窗口坐标(基于主窗口坐标进行计算)
    void mouseDoubleClickEvent(QMouseEvent* event) override;   //重写双击事件
    void mousePressEvent(QMouseEvent* event) override;         //重写鼠标单击事件
    void mouseReleaseEvent(QMouseEvent* event) override;       //重写鼠标释放事件
    void mouseMoveEvent(QMouseEvent* event) override;          //重写鼠标移动事件
    void closeEvent(QCloseEvent* event) override;              //重写关闭窗口事件
    Ui::MainWidget *ui;
};

#endif // MAINWIDGET_H
