#include "imageview2.h"
#include <QPainter>
#include <QPaintEvent>
#include <QMenu>
#include <QDebug>
#include "VisionLibrary/visionlibrary.h"

constexpr int CODE(int x, int y)
{
    constexpr int OFFSET = 4;
    --x;
    --y;
    x = (x < 0) ? 0 : (1 << x);
    y = (y < 0) ? 0 : (1 << y);
    return x | (y << OFFSET);
}
// 描述点位于选框的哪个区域
enum RegionCode {
    RegionLeft = CODE(1, 0),
    RegionHCenter = CODE(2, 0),
    RegionRight = CODE(3, 0),

    RegionTop = CODE(0, 1),
    RegionVCenter = CODE(0, 2),
    RegionBottom = CODE(0, 3),
};

ImageView2::ImageView2(QWidget *parent) : ImageView1(parent)
{
    setupContextMenu();
}

void ImageView2::setMat(const cv::Mat &mat)
{
    ImageView1::setMat(mat);
    makeMarqueeInvalid();
}

const cv::Mat ImageView2::roi() const
{
    // 返回用户确认了选中区域(图像坐标系中)
    if (!imageContainsMarquee()) {
        // 如果矩形框没有完全包含在图像里
        qInfo() << "!imageContainsMarquee()";
        return cv::Mat();
    }
    const QRect rect = window2Image(_marquee);
    if (!rect.isValid()) {
        qInfo() << "!rect.isValid()";
        return cv::Mat();
    }
    return _mat(VisionLibrary::toCvRect(rect));
}

void ImageView2::paintEvent(QPaintEvent *event)
{
    ImageView1::paintEvent(event);

    QPainter painter(this);
    if (_marquee.isValid()) {
        drawMarquee(painter);
    }
}

void ImageView2::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        _start = event->pos();
        // 首先判断鼠标左键按下时在哪个区域
        if ((RegionHCenter | RegionVCenter) == _currentRegion) {
            // 如果鼠标在矩形内部
            _mouseMovingMeaing = MouseMovingMeaning::MovingMarquee;
        } else if (_marquee.contains(event->pos())) {
            // 如果鼠标在矩形边缘
            _mouseMovingMeaing = MouseMovingMeaning::AdjustingMarquee;
        } else {
            // 如果鼠标在矩形外部
            if (event->modifiers() & Qt::ControlModifier) {
                // 用户按住了Ctrl, 就开始创建Rect
                _mouseMovingMeaing = MouseMovingMeaning::CreatingMarquee;
            } else {
                // 移动图像
                _mouseMovingMeaing = MouseMovingMeaning::MovingImage;
            }
        }
    } else if (event->buttons() & Qt::RightButton) {
        // 点击右键
        if (_marquee.contains(event->pos()) &&
                imageContainsMarquee()) { // 矩形框要完全包含在图片中
            // 上下文菜单
            _menu->exec(QCursor::pos());
        } else {
            // 缩放复原
            initBasicTransform();
            update();
        }
    }
}

void ImageView2::mouseMoveEvent(QMouseEvent *event)
{
    if (MouseMovingMeaning::Nothing != _mouseMovingMeaing) {
        const QPoint offset = event->pos() - _start;
        if (MouseMovingMeaning::MovingImage == _mouseMovingMeaing) {
            // 如果正在移动图片
            _offset += offset;
            // 更新起始点
            _start = event->pos();
        } else {
            // 矩形相关
            if (MouseMovingMeaning::CreatingMarquee == _mouseMovingMeaing) {
                // 如果正在创建矩形
                _marquee = QRect(event->pos(), _start);
            } else {
                if (MouseMovingMeaning::AdjustingMarquee == _mouseMovingMeaing) {
                    // 如果正在改变矩形大小
                    if (_currentRegion & RegionLeft) {
                        _marquee.setLeft(_marquee.left() + offset.x());
                    } else if (_currentRegion & RegionRight) {
                        _marquee.setRight(_marquee.right() + offset.x());
                    }

                    if (_currentRegion & RegionTop) {
                        _marquee.setTop(_marquee.top() + offset.y());
                    } else if (_currentRegion & RegionBottom) {
                        _marquee.setBottom(_marquee.bottom() + offset.y());
                    }
                } else if (MouseMovingMeaning::MovingMarquee == _mouseMovingMeaing) {
                    // 如果正在移动矩形
                    _marquee.translate(offset);
                }
                // 更新起始点
                _start = event->pos();
            }
            _marquee = _marquee.normalized();
        }
        // 重绘
        update();
    }
    // 根据鼠标的位置设置当前的鼠标形状
    updateCurrentRegion(event->pos());
}

void ImageView2::mouseReleaseEvent(QMouseEvent *)
{
    _mouseMovingMeaing = MouseMovingMeaning::Nothing;
}

void ImageView2::setupContextMenu()
{
    _menu = new QMenu(this);
    QAction *action_del = new QAction(QStringLiteral("删除"), this);
    connect(action_del, &QAction::triggered, [this]() {
        makeMarqueeInvalid();
        update();
    });
    _menu->addAction(action_del);

    QAction *action_confirm = new QAction(QStringLiteral("确定"), this);
    connect(action_confirm, &QAction::triggered, [this]() {
        emit signal_confirmed(roi());
        makeMarqueeInvalid();
        update();
    });
    _menu->addAction(action_confirm);
}

void ImageView2::makeMarqueeInvalid()
{
    _marquee.setLeft(-1);
    _marquee.setRight(-1);
}

// 线宽
constexpr int EDGE_WIDTH = 3;
void ImageView2::drawMarquee(QPainter &painter)
{
    painter.save();
    QPen pen;
    pen.setWidth(EDGE_WIDTH);

    pen.setColor(Qt::black);
    painter.setPen(pen);

    const QColor brushColor = imageContainsMarquee() ? QColor(0, 0, 200, 100) :
                              QColor(200, 0, 0, 120);
    painter.setBrush(QBrush(brushColor));
    painter.drawRect(_marquee);

    pen.setColor(Qt::green);
    painter.setPen(pen);
    painter.drawPoint(_marquee.topLeft());

    pen.setColor(Qt::yellow);
    painter.setPen(pen);
    painter.drawPoint(_marquee.bottomRight());

    painter.restore();
}

int code(const int value, const QVector<int> &limits)
{
    int index = 0;
    while (index < limits.size()) {
        if (value < limits[index]) {
            return index;
        }
        ++index;
    }
    return index;
}

// 区域宽度
constexpr int REGION_WIDTH = 5;
int ImageView2::judgeRegion(const QPoint &pos) const
{
    const QVector<int> x{
        _marquee.left(),
        _marquee.left() + REGION_WIDTH,
        _marquee.right() - REGION_WIDTH,
        _marquee.right(),
    };
    const QVector<int> y{
        _marquee.top(),
        _marquee.top() + REGION_WIDTH,
        _marquee.bottom() - REGION_WIDTH,
        _marquee.bottom(),
    };
    const int regionCode = CODE(code(pos.x(), x), code(pos.y(), y));
//    qInfo() << regionCode << (RegionRight | RegionTop);
    return regionCode;
}

void ImageView2::updateCurrentRegion(const QPoint &pos)
{
    _currentRegion = judgeRegion(pos);
    switch (_currentRegion) {
    case RegionLeft | RegionTop: // 左上
    case RegionRight | RegionBottom: // 右下
        setCursor(Qt::SizeFDiagCursor);
        break;
    case RegionLeft | RegionVCenter: // 左中
    case RegionRight | RegionVCenter: // 右中
        setCursor(Qt::SizeHorCursor);
        break;
    case RegionLeft | RegionBottom: // 左下
    case RegionRight | RegionTop: // 右上
        setCursor(Qt::SizeBDiagCursor);
        break;
    case RegionHCenter | RegionTop: // 中上
    case RegionHCenter | RegionBottom: // 中下
        setCursor(Qt::SizeVerCursor);
        break;
    case RegionHCenter | RegionVCenter: // 中中
        setCursor(Qt::SizeAllCursor);
        break;
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}

QPoint ImageView2::window2Image(const QPoint &pos) const
{
    // 一般来说, 仿射变换都是可逆的, 所以(_matrix[0][0] * _matrix[1][1] - _matrix[1][0] * _matrix[0][1])不为0
    const QPointF point = (pos - _offset) / (_matrix[0][0] * _matrix[1][1] - _matrix[1][0] * _matrix[0][1]);
    return QPoint(qRound(_matrix[1][1] * point.x() - _matrix[0][1] * point.y()),
                  qRound(-_matrix[1][0] * point.x() + _matrix[0][0] * point.y()));
}

QRect ImageView2::window2Image(const QRect &rect) const
{
    return QRect(window2Image(rect.topLeft()),
                 window2Image(rect.bottomRight()));
}

QPoint ImageView2::image2Window(const QPoint &pos) const
{
    return QPoint(qRound(_matrix[0][0] * pos.x() + _matrix[0][1] * pos.y() + _offset.x()),
                  qRound(_matrix[1][0] * pos.x() + _matrix[1][1] * pos.y() + _offset.y()));
}

QRect ImageView2::image2Window(const QRect &rect) const
{
    return QRect(image2Window(rect.topLeft()),
                 image2Window(rect.bottomRight()));
}

bool ImageView2::imageContainsMarquee() const
{
    return image2Window(_image.rect()).contains(_marquee);
}

