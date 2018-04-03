#pragma once

#include <opencv2/video/tracking.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;

class Vector
{
public:
	Point2f start, end;
	Vector() {}
	Vector(Point2f start, Point2f end):start(start), end(end) {}
	double getAngel(Vector r)
	{
		double m1 = sqrt((end.y - start.y) * (end.y - start.y) + (end.x - start.x) * (end.x - start.x));
		double m2 = sqrt((r.end.y - r.start.y) * (r.end.y - r.start.y) + (r.end.x - r.start.x) * (r.end.x - r.start.x));
		if (isnan(m1) || isnan(m2)) return -1;
		if (m1 * m2 == 0) return -1;
		return 180.0 / CV_PI * acos(((end.x - start.x) * (r.end.x - r.start.x) + (end.y - start.y) * (r.end.y - r.start.y)) / (m1 * m2));
	}
	void axisymmetric(Point2f o)
	{
		start.x = o.x - (start.x - o.x);
		start.y = o.y - (start.y - o.y);
		end.x = o.x - (end.x - o.x);
		end.y = o.y - (end.y - o.y);
	}
};
