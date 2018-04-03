#pragma once

#include <opencv2/imgproc/imgproc.hpp>
#include "utils.h"
#include "Circle.h"
using namespace cv;

/**
 * 选取平台
 * @param window_name 选取窗口的窗口名
 * @param platform_color 平台提示颜色
 * @param image 视频原图 任意帧
 * @return 选取平台的位置
 */
Circle select_platform(string window_name, Scalar platform_color,Mat *image);

/**
 * 选取平台时监听鼠标事件
 * @param event
 * @param x
 * @param y
 */
void on_select_platform(int event, int x, int y, int, void*);

/**
 * 选取水池
 * @param window_name 选取窗口的窗口名
 * @param pool_color 选取水池时提示颜色
 * @param image 视频原图 任意帧
 * @return 选取平台的位置 矩形
 */
Rect select_pool(string window_name, Scalar pool_color,Mat *image);

/**
 * 选取水池时监听事件
 */
void on_pool_mouse(int event, int x, int y, int, void *);