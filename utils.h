#pragma once

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
using namespace cv;
using namespace std;

const int _UTILS_WINDOWS_WIDTH = 400;
const int _UTILS_WINDOWS_HEIGTH = 400;
const int _UTILS_WINDOWS_MAX_SHOW = 50;	//同时显示50个
/**
 * 角度转弧度
 *
 * @param angle 角度
 * @return 弧度
 */
float to_rad(float angle);

/**
 * 计算2个点之间的距离
 *
 * @param p1 点1
 * @param p2 点2
 * @return 2点之间距离
 */
double cal_distance(Point2f p1, Point2f p2);

/**
 * 软件提示
 */
void software_help();

/**
 * 计算2个有共同起点的向量(c,pt1) (c,pt2)的夹角
 * @param pt1 向量1终点
 * @param pt2 向量2终点
 * @param c 共同起点
 * @return
 */
float get_angel_of_two_vector(Point2f &pt1, Point2f &pt2, Point2f &c);

/**
 * 将速度图以折线图形式打印在指定窗口
 * @param window_name 指定窗口名称
 * @param speeds 速度存储
 */
void paint_speed_graph(string window_name, vector<double> *speeds);

/**
 * 将小白鼠中whishaw参数以折线图形式打印在指定窗口
 * @param window_name 指定窗口名称
 * @param whishaws whishaws 角度存储
 */
void paint_whishaw_graph(string window_name, vector<double> *whishaws);

/**
 * 判断点是否在椭圆中
 * @param p 判断点
 * @param center 矩形中心
 * @param hAxis 长轴
 * @param vAxis 短轴
 * @return true在椭圆内部包括边缘 false不在
 */
bool judge_in_ellipse(Point p, Point2f center, double hAxis, double vAxis);