#pragma once

#include <opencv2/imgproc/imgproc.hpp>
#include "utils.h"
using namespace cv;

struct Circle
{
    Point center;
    double r;
    /**
     * 无参构造函数
     */
    Circle() {};

    /**
     * 构造函数
     * @param center 圆中心点
     * @param r 半径
     */
    Circle(Point center, double r);

    /**
     * 判断点是否在圆内
     * @param pt 需判断的点
     * @return true在圆内包括边缘 false 圆外
     */
    bool contains(Point pt);
};