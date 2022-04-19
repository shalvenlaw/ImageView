#pragma once

#include "ImageView1/imageview1.h"

class QMenu;
class ImageView2 : public ImageView1
{
    Q_OBJECT
public:
    explicit ImageView2(QWidget *parent = nullptr);

    void setMat(const cv::Mat &mat) override;

    const cv::Mat roi() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    // 鼠标事件
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
signals:
    // 返回用户确认了选中区域(图像坐标系中)
//    void signal_confirmed(QRect rect_inImage);
    void signal_confirmed(const cv::Mat &roi);
private:
    void setupContextMenu();
    void makeMarqueeInvalid();

    // 绘制矩形框
    void drawMarquee(QPainter &painter);

    // 判断点位于选框的哪个区域
    int judgeRegion(const QPoint &pos) const;
    void updateCurrentRegion(const QPoint &pos);

    // 把坐标从窗口坐标系转换到图像坐标系
    QPoint window2Image(const QPoint &pos) const;
    QRect window2Image(const QRect &rect) const;
    // 把坐标从图像坐标系转换到窗口坐标系
    QPoint image2Window(const QPoint &pos) const;
    QRect image2Window(const QRect &rect) const;

    bool imageContainsMarquee() const;

    // 描述鼠标移动的意义
    enum class MouseMovingMeaning : int {
        Nothing, //
        CreatingMarquee, // 正在创建选框
        MovingMarquee, // 正在移动选框
        AdjustingMarquee, // 正在调整选框
        MovingImage, // 正在移动图像
    };
    MouseMovingMeaning _mouseMovingMeaing = MouseMovingMeaning::Nothing; // 鼠标移动的意义

    // 选框. 位于窗口坐标系
    QRect _marquee;

    int _currentRegion = 0; // 鼠标位于选框的哪个区域

    // 上下文菜单. 当鼠标位于选框中, 点击右键时弹出
    QMenu *_menu;
};

