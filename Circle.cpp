#include "Circle.h"

Circle::Circle(Point center, double r)
{
    this->center = center;
    this->r = r;
}

bool Circle:: contains(Point pt)
{
    double dis = cal_distance(pt, center);
    return dis >= r ? false : true;
}
