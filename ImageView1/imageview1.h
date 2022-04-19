#pragma once

#include <QWidget>
#include <QImage>
#include <opencv2/opencv.hpp>

class ImageView1 : public QWidget
{
    Q_OBJECT
public:
    explicit ImageView1(QWidget *parent = nullptr);

    const cv::Mat &mat() const;

public slots:
    virtual void setMat(const cv::Mat &mat);

    void loadMatFromPath(const QString &path);
protected:
    void paintEvent(QPaintEvent *event) override;
    // 鼠标事件
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    // 控件大小改变事件
    void resizeEvent(QResizeEvent *event) override;
signals:
    void signal_matChanged(const cv::Mat &mat);
    void signal_matLoaded(const cv::Mat &mat);
protected:
    // 每当窗口大小或图像大小改变, 都要重新计算一次基本变换
    void initBasicTransform();
    void scale(const double scaleFactor);

    // 绘制背景
    void drawBackground(QPainter &painter);
    // 绘制图片
    void drawImage(QPainter &painter);

    // 原图
    QImage _image;
    cv::Mat _mat;

    // 基本变换 = _matrix + _offset
    double _matrix[2][2] {
        {1.0, 0.0},
        {0.0, 1.0},
    };
    QPointF _offset{0.0, 0.0};

    QPoint _start; // 描述鼠标每次点击, 或移动的开始坐标. 位于窗口坐标系
    bool _isMovingImage = false; // 是否正在移动图像
};

