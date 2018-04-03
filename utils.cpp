#include "utils.h"

float to_rad(float angle)
{
	return angle * CV_PI / 180.0f;
}

double cal_distance(Point2f p1, Point2f p2)
{
	return sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));
}

void software_help()
{
	cout << "\nThis is a demo that shows mean-shift based tracking\n"
		"You select a color objects such as your face and it tracks it.\n"
		"This reads from video camera (0 by default, or the camera number the user enters\n"
		"Usage: \n"
		"   ./camshiftdemo [camera number]\n";

	cout << "\n\nHot keys: \n"
		"\tESC - quit the program\n"
		"\tc - stop the tracking\n"
		"\tb - switch to/from backprojection view\n"
		"\th - show/hide object histogram\n"
		"\tp - pause video\n"
		"To initialize tracking, select the object with mouse\n";
}

float get_angel_of_two_vector(Point2f &pt1, Point2f &pt2, Point2f &c)
{
	float theta = atan2(pt1.x - c.x, pt1.y - c.y) - atan2(pt2.x - c.x, pt2.y - c.y);
	if (theta > CV_PI)
		theta -= 2 * CV_PI;
	if (theta < -CV_PI)
		theta += 2 * CV_PI;

	theta = theta * 180.0 / CV_PI;
	return theta;
}

void paint_speed_graph(string window_name, vector<double> *speeds)
{
	Mat speed_mat = Mat(_UTILS_WINDOWS_WIDTH, _UTILS_WINDOWS_HEIGTH, CV_8SC3,Scalar(255,255,255));
	double dx = _UTILS_WINDOWS_WIDTH;
	dx = 1.0*dx /50;
	int size = speeds->size();
	if (size < 2)
	{
		return;
	}
	int index = (size - _UTILS_WINDOWS_MAX_SHOW) < 0 ? 0 : (size - _UTILS_WINDOWS_MAX_SHOW);
	for (int i = index; i < size - 1; i++)
	{
		line(speed_mat, Point2f(dx*(i + 1 - index), _UTILS_WINDOWS_HEIGTH - (*speeds)[i]), Point2f(dx*(i + 2 - index), _UTILS_WINDOWS_HEIGTH - (*speeds)[i + 1]), Scalar(0, 255, 0), 2);
	}
	imshow(window_name, speed_mat);
}

void paint_whishaw_graph(string window_name, vector<double> *whishaws)
{
	Mat whishaw_mat = Mat(_UTILS_WINDOWS_WIDTH, _UTILS_WINDOWS_HEIGTH, CV_8SC3, Scalar(255, 255, 255));
	double dx = _UTILS_WINDOWS_WIDTH;
	dx = 1.0*dx / 50;
	int size = whishaws->size();
	if (size < 2)
	{
		return;
	}
	int index = (size - _UTILS_WINDOWS_MAX_SHOW) < 0 ? 0 : (size - _UTILS_WINDOWS_MAX_SHOW);
	for (int i = index; i < size - 1; i++)
	{
		line(whishaw_mat, Point2f(dx*(i + 1 - index), _UTILS_WINDOWS_HEIGTH - (*whishaws)[i]/180*400), Point2f(dx*(i + 2 - index), _UTILS_WINDOWS_HEIGTH - (*whishaws)[i + 1] / 180 * 400), Scalar(0, 255, 0), 2);
	}
	imshow(window_name, whishaw_mat);
}