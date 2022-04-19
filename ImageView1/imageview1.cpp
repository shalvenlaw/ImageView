#include "imageview1.h"
#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <QMessageBox>
#include "VisionLibrary/visionlibrary.h"
#include "CommonLibrary/GlobalTools/globaltools.h"

ImageView1::ImageView1(QWidget *parent) : QWidget(parent)
{
    // QWidget默认是不追踪鼠标的, 要一直点着鼠标的一个键移动才能触发mouseMoveEvent.
    // setMouseTracking(true)之后就可以追踪鼠标了
    setMouseTracking(true);
}

void ImageView1::setMat(const cv::Mat &mat)
{
    const QSize oldSize = _image.size();
    _image = VisionLibrary::toPremultiImage(mat);
    if (_image.size() != oldSize) {
        // 如果图像大小发生变化, 那么要重新计算基本变换
        initBasicTransform();
    }
    _mat = mat; // 浅拷贝
    emit signal_matChanged(_mat);
    update();
}

void ImageView1::loadMatFromPath(const QString &imagePath)
{
    if (imagePath.isEmpty()) {
        return;
    }
    // 导入灰度图
    const cv::Mat tmp = cv::imread(utf8_to_gbk(imagePath), cv::IMREAD_GRAYSCALE);
    if (tmp.empty()) {
        QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("导入图片{%1}失败!").arg(imagePath));
        return;
    }
    setMat(tmp);
    emit signal_matLoaded(tmp);
}

const cv::Mat &ImageView1::mat() const
{
    return _mat;
}

void ImageView1::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    drawBackground(painter);
    drawImage(painter);
}

void ImageView1::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        _isMovingImage = true;
        _start = event->pos();
    } else if (event->buttons() & Qt::RightButton) {
        initBasicTransform();
        update();
    }
}

void ImageView1::mouseMoveEvent(QMouseEvent *event)
{
    if (_isMovingImage) {
        // 如果正在移动图片
        _offset += event->pos() - _start;
        // 更新起始点
        _start = event->pos();
        update();
    }
}

void ImageView1::mouseReleaseEvent(QMouseEvent *)
{
    _isMovingImage = false;
}

void ImageView1::wheelEvent(QWheelEvent *event)
{
    _offset -= event->pos();

    // 缩放速度, 范围:(0.0, 1.0)
    constexpr float ZOOM_SPEED = 0.2f;
    // 缩放因子
    constexpr float ZOOM_IN_FACTOR = 1.0f + ZOOM_SPEED;
    constexpr float ZOOM_OUT_FACTOR = 1.0f - ZOOM_SPEED;
    scale(event->delta() > 0 ? ZOOM_IN_FACTOR : ZOOM_OUT_FACTOR);

    _offset += event->pos();
    update();
}

void ImageView1::resizeEvent(QResizeEvent *)
{
    // show()的时候也会调用resizeEvent(), 奇怪的是为什么会调用两次resize?
    // 貌似是父窗口show的时候调用一次resize, 子窗口show的时候也调用一次resize!!
    // 猜想: 由于layout(布局)的作用, 最多有两次resize吧?
    initBasicTransform();
    // 之后会自动调用paintEvent, 所以这里不用调用update()
}

void ImageView1::initBasicTransform()
{
    if (_image.isNull()) {
        // 图像为空就不进行计算了, 不然后面的计算中可能出现0除错误
        return;
    }
    _matrix[0][1] = _matrix[1][0] = 0.0;
    const double heightRatio = height() / double(_image.height());
    const double widthRatio = width() / double(_image.width());
    if (heightRatio < widthRatio) {
        // window的width比较长, 则左右有黑边
        _matrix[0][0] = _matrix[1][1] = heightRatio;
        _offset.setX(_image.width() * (widthRatio - heightRatio) * 0.5);
        _offset.setY(0.0);
    } else {
        // window的height比较长, 则上下有黑边
        _matrix[0][0] = _matrix[1][1] = widthRatio;
        _offset.setX(0.0);
        _offset.setY(_image.height() * (heightRatio - widthRatio) * 0.5);
    }
}

void ImageView1::scale(const double scaleFactor)
{
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            _matrix[i][j] *= scaleFactor;
        }
    }
    _offset *= scaleFactor;
}

void ImageView1::drawBackground(QPainter &painter)
{
    painter.save();
    // 灰色背景
    painter.setPen(Qt::gray);
    painter.setBrush(Qt::gray);
    painter.drawRect(rect());

    painter.restore();
}

void ImageView1::drawImage(QPainter &painter)
{
    painter.save();
    // 绘制图片
    painter.setWorldTransform(QTransform(_matrix[0][0], _matrix[0][1],
                                         _matrix[1][0], _matrix[1][1],
                                         _offset.x(), _offset.y()));
    painter.drawImage(0, 0, _image);

    painter.restore();
}
