#pragma once

#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include "utils.h"
using namespace cv;
using namespace std;

/**
 * 老师论文中找头
 * 准确率低 已弃用
 */
struct relativePoint
{
    Point point;
    double distance;

    /**
     * 构造函数
     * @param point 当前点位置
     * @param distance 到中心点距离
     */
    relativePoint(Point point, double distance);

    /**
     * 从大到小排序
     */
    friend bool operator<(relativePoint p1,relativePoint p2) 
	{
		return p1.distance > p2.distance;
	}
};

///找頭
vector<relativePoint> find_head_sort_point(Mat *image)
{
    vector<Point> v;
    int rows = image->rows, cols = image->cols;
    int count = 0, tmp;
    double sumx = 0.0, sumy = 0.0;
    for (int i = 0; i < rows; i++)
    {
        const uchar* inData = image->ptr<uchar>(i);
        for (int j = 0; j < cols; j++)
        {
            tmp = (int)(*inData++);
            if (tmp == 255)
            {
                sumx += i;
                sumy += j;
                v.push_back(Point(j, i));
                count++;
            }
        }
    }
    sumx /= count;
    sumy /= count;
    vector<relativePoint> res;
    double distance;
    Point2f center = Point2f(sumx, sumy);
    for (int i = 0; i < count; i++)
    {
        distance = cal_distance(v[i], center);
        res.push_back(relativePoint(v[i], distance));
    }
    sort(res.begin(), res.end());
    return res;
}

Point find_head(Mat *image,Mat *image2 ,float x, float y)
{
    vector<relativePoint> res = find_head_sort_point(image);
    for (int i = 0; i < res.size(); ++i)
    {
        circle(*image2, res[i].point, 2, Scalar(0, 0, 255), -1);
    }
    return Point(res[0].point.x + x, res[0].point.y + y);
}

Point find_head(Mat *image, float x, float y)
{
    vector<relativePoint> res = find_head_sort_point(image);

    return Point(res[0].point.x + x ,res[0].point.y + y);
}

Point find_head(Mat *image)
{
    vector<relativePoint> res = find_head_sort_point(image);

    return res[0].point;
}

//计算头部
//Point head = findHead(&target, &image,rectRange.x, rectRange.y);

//circle(image, head, 1, Scalar(0, 0, 255), -1);