#include "visionlibrary.h"
#include <QDebug>
#include <QPixmap>

QImage toPremultiImage_helper1(const cv::Mat &srcImage, const QImage::Format format)
{
    return QImage(srcImage.data,
                  srcImage.cols, srcImage.rows,
                  static_cast<int>(srcImage.step),
                  format);
}

template <typename T>
QImage toPremultiImage_helper2(const cv::Mat &srcImage)
{
    QImage image(srcImage.cols, srcImage.rows, QImage::Format_ARGB32_Premultiplied);
    for (int i = 0; i < srcImage.rows; ++i) {
        const T *const srcRow = srcImage.ptr<T>(i);
        QRgb *const destRow = reinterpret_cast<QRgb *>(image.scanLine(i));
        for (int j = 0; j < srcImage.cols; ++j) {
            const int color = qRound(srcRow[j] * 255.0);
            destRow[j] = qRgba(color, color, color, 255);
        }
    }
    return image;
}

QImage VisionLibrary::toPremultiImage(const cv::Mat &srcImage, const bool swapRG)
{
    switch ( srcImage.type() ) {
    // 8-bit, 4 channel
    case CV_8UC4: {
        QImage image = toPremultiImage_helper1(srcImage, QImage::Format_ARGB32);
        image.convertTo(QImage::Format_ARGB32_Premultiplied);
        return image;
    }

    // 8-bit, 3 channel
    case CV_8UC3: {
        QImage image = toPremultiImage_helper1(srcImage, QImage::Format_RGB888);
        if (swapRG) {
            image = image.rgbSwapped();
        }
        image.convertTo(QImage::Format_ARGB32_Premultiplied);
        return image;
    }

    // 8-bit, 1 channel
    case CV_8UC1: {
        QImage image = toPremultiImage_helper1(srcImage, QImage::Format_Grayscale8);
        image.convertTo(QImage::Format_ARGB32_Premultiplied);
        return image;
    }

    /* 下面元素是浮点数的图像未经测试, 等有机会再改 */
    // float, 1 channel. 调用者请注意: 每个元素必须都在[0, 1]
    case CV_32FC1: {
        return toPremultiImage_helper2<float>(srcImage);
    }

    // double, 1 channel. 调用者请注意: 每个元素必须都在[0, 1]
    case CV_64FC1: {
        return toPremultiImage_helper2<double>(srcImage);
    }
    default:
        qWarning() << QStringLiteral("%1失败! 不支持的cv::Mat类型:%2").arg(__FUNCTION__).arg(srcImage.type());
        break;
    }

    return QImage();
}

QPixmap VisionLibrary::toQPixmap(const cv::Mat &srcImage)
{
    return QPixmap::fromImage(toPremultiImage(srcImage));
}

cv::Mat VisionLibrary::threshold(const cv::Mat &srcImage, const int kSize, const int minDiff)
{
    cv::Mat background;
    // 用滤波估计背景
    cv::blur(srcImage, background, cv::Size(kSize, kSize));
    return srcImage - background >= minDiff;
}

cv::Mat VisionLibrary::otsuThreshold(const cv::Mat &srcImage)
{
    cv::Mat dstImage;
    cv::threshold(srcImage, dstImage, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    return dstImage;
}

std::vector<std::vector<cv::Point>> VisionLibrary::findSimpleExternalContours(const cv::Mat &srcImage, const cv::Point &offset)
{
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(srcImage, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, offset);
    return contours;
}

void VisionLibrary::drawText(cv::Mat &srcImage, const cv::Point &center, const std::string &text, const int fontFace, const double fontScale, const int thickness,
                             const cv::Scalar &color)
{
    // 设置绘制文本的相关参数
    int baseline;
    // 获取文本框的长宽
    cv::Size text_size = cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);
    // 将文本框居中绘制
    const cv::Point origin = center + cv::Point(-text_size.width / 2, text_size.height / 2);

    cv::putText(srcImage, text, origin, fontFace, fontScale, color, thickness);
}

double VisionLibrary::vectorAngle(const cv::Point2f &lhs, const cv::Point2f &rhs)
{
    const cv::Point2f vec = lhs - rhs;
    // 角度转换成弧度, 由于图像坐标系与传统直角坐标系的旋转方向相反, 所以angle要取相反数
    return -std::atan(vec.y / vec.x) * 180.0f / CV_PI;
}

cv::Rect VisionLibrary::toCvRect(const QRect &rect)
{
    return cv::Rect(rect.x(), rect.y(), rect.width(), rect.height());
}

cv::Size VisionLibrary::roundToCvSize(const cv::Point2d &point)
{
    return cv::Size(qRound(point.x), qRound(point.y));
}

cv::Point2d VisionLibrary::toCvPoint2d(const cv::Size &size)
{
    return cv::Point2d(size.width, size.height);
}
