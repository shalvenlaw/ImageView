#pragma once

#include <QImage>
#include <opencv2/opencv.hpp>

namespace VisionLibrary {

//const auto WHITE = cv::Scalar::all(255);
//const auto BLACK = cv::Scalar::all(0);
const cv::Scalar SCALAR_WHITE(255, 255, 255);
const cv::Scalar SCALAR_BLACK(0, 0, 0);
const cv::Scalar SCALAR_BLUE(255, 0, 0);
const cv::Scalar SCALAR_GREEN(0, 255, 0);
const cv::Scalar SCALAR_RED(0, 0, 255);
const cv::Scalar SCALAR_YELLOW(255, 255, 0);

const cv::Point POINT_ZEROS(0, 0);
const cv::Point POINT_MINUS_ONES(-1, -1);
const cv::Point2f POINT2F_ZEROS(0.0f, 0.0f);
const cv::Point2f POINT2F_MINUS_ONES(-1.0f, -1.0f);

/*!
 * \brief toPremultiImage cv::Mat转换成预乘的QImage, 之后的QImage专门用于显示或blend
 * \param srcImage
 * \param swapRG 如果是CV_8UC3, 是否要交换R和G通道? 如果是通过cv::imread读进来的图片, 就需要交换.
 * \return
 * \note
 * ## 不同类型存储图像的惯例:
 * - 0 to 255 for CV_8U images
 * - 0 to 65535 for CV_16U images
 * - 0 to 1 for CV_32F images
 *
 * 由于QPainter对QImage::Format_RGB32和QImage::Format_ARGB32_Premultiplied的渲染效率最高,
 * 所以本函数返回的都是QImage::Format_ARGB32_Premultiplied
 */
QImage toPremultiImage(const cv::Mat &srcImage, const bool swapRG = true);

QPixmap toQPixmap(const cv::Mat &srcImage);

/*!
 * \brief threshold 动态阈值二值化. 由于不均匀的光照或噪声太大, (直方图中不存在双峰), 无法找出一个对整幅图像都适用的固定阈值. 将图像与其局部背景进行比较的操作被称为动态阈值分割处理.
 * 1. 看图像与局部背景亮多少: S = {(r, c)^T \in R | f_{r, c} - g_{r, c} \ge g_{diff}}
 * \param srcImage
 * \return
 * \note 此方法比较适合字符的分割(因为字符笔划比较细)
 */
cv::Mat threshold(const cv::Mat &srcImage, const int kSize = 3, const int minDiff = 5);

/*!
 * \brief otsuThreshold 用大津法(OSTU)进行阈值分割
 * \param srcImage
 * \return
 * \note 简单包装cv::threshold(srcImage, dstImage, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU)
 */
cv::Mat otsuThreshold(const cv::Mat &srcImage);

/*!
 * \brief findSimpleExternalContours
 * \param srcImage
 * \param offset
 * \return
 * \note 简单封装cv::findContours(image, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, offset)
 */
std::vector<std::vector<cv::Point>> findSimpleExternalContours(const cv::Mat &srcImage, const cv::Point &offset = POINT_ZEROS);

void drawText(cv::Mat &srcImage,
              const cv::Point &center,
              const std::string &text,
              const int fontFace = cv::FONT_HERSHEY_SIMPLEX,
              const double fontScale = 10,
              const int thickness = 10,
              const cv::Scalar &color = SCALAR_WHITE);

/*!
 * \brief vectorAngle 求向量的角度
 * \param lhs
 * \param rhs
 * \return
 */
double vectorAngle(const cv::Point2f &lhs, const cv::Point2f &rhs);

cv::Rect toCvRect(const QRect &rect);

cv::Size roundToCvSize(const cv::Point2d &point);

cv::Point2d toCvPoint2d(const cv::Size &size);


}

