#include "initiation.h"
#include "opencv2/calib3d.hpp"

///correction
int _point_cnt, _is_full;
int _temp_x, _temp_y;
Mat _src_image;
Point2f _tmp[4];
int _image_width;
void on_MouseHandle(int event, int x, int y, int flags, void* param)
{
	_temp_x = x;
	_temp_y = y;
	switch (event)
	{
		//左键按下消息
	case EVENT_LBUTTONDOWN:
	{
		if (_is_full == 0) {
			_tmp[_point_cnt++] = Point2f(x, y);
		}
		if (_point_cnt == 4)
			_is_full = 1;
		cout << x << " " << y << endl;
	}
	break;
	}
}

void correction::init()
{
	cameraMatrix = Mat::eye(3, 3, CV_64F);
	cameraMatrix.at<double>(0, 0) = 1.65316008577199e+03; cameraMatrix.at<double>(0, 1) = 0;						cameraMatrix.at<double>(0, 2) = 6.113689008471209e+02;
	cameraMatrix.at<double>(1, 0) = 0;					  cameraMatrix.at<double>(1, 1) = 1.772515363065066e+03;    cameraMatrix.at<double>(1, 2) = 5.229834906076032e+02;
	cameraMatrix.at<double>(2, 0) = 0;					  cameraMatrix.at<double>(2, 1) = 0;					    cameraMatrix.at<double>(2, 2) = 1;
	distCoeffs = Mat::zeros(5, 1, CV_64F);
	distCoeffs.at<double>(0, 0) = 0.1566519094750514;
	distCoeffs.at<double>(1, 0) = -3.181653453113646;
	distCoeffs.at<double>(2, 0) = -0.001494054814669329;
	distCoeffs.at<double>(3, 0) = -0.005437536265284954;
	distCoeffs.at<double>(4, 0) = 27.12027288173052;
}

void correction::init(Mat *image)
{
	namedWindow("frame", WINDOW_AUTOSIZE); 
	_src_image = image->clone();
	imshow("frame", _src_image);
	_point_cnt = 0;
	_is_full = 0;
	_temp_x = 0;
	_temp_y = 0;
	setMouseCallback("frame", on_MouseHandle);
	while (!_is_full)
	{
		_src_image = image->clone();
		line(_src_image, Point(0, _temp_y), Point(_src_image.cols, _temp_y), Scalar(255, 255, 255));
		line(_src_image, Point(_temp_x, 0), Point(_temp_x, _src_image.rows), Scalar(255, 255, 255));
		for (int i = 0; i < _point_cnt; i++)
		{
			if (i == _point_cnt - 1)
			{
				break;
			}
			line(_src_image, _tmp[i], _tmp[i+1], Scalar(255, 255, 255),2);

		}
		waitKey(10);
		imshow("frame", _src_image);
	}
	for (int i = 0; i < _point_cnt; i++)
	{
		tmp1[i] = Point2f(_tmp[i].x,_tmp[i].y);
	}
	destroyWindow("frame");
}

void correction::work(Mat *image, Mat *result)
{
	Mat view, rview, map1, map2, frameCalibration;
	Mat frame = *image;
	Size imageSize;
	imageSize = frame.size();
	initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
		getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0),
		imageSize, CV_16SC2, map1, map2);

	remap(frame, frameCalibration, map1, map2, INTER_LINEAR);

	frameCalibration.copyTo(frame);
	int image_row = frame.rows;
	int image_col = frame.cols;
	tmp2[0] = Point2f(0, 0);
	tmp2[1] = Point2f(image_col, 0);
	tmp2[2] = Point2f(image_col, image_col);
	tmp2[3] = Point2f(0, image_col);

	Mat transform = getPerspectiveTransform(tmp1, tmp2);
	Size size = frame.size();
	Mat img_trans = Mat::zeros(image_col, image_col, CV_8UC3);
	warpPerspective(frame, img_trans, transform, Size(image_col, image_col));
	img_trans.copyTo(*result);

	center.x = (tmp2[2].x - tmp2[0].x) / 2.0;
	center.y = (tmp2[2].y - tmp2[0].y) / 2.0;
	r = (tmp2[2].x - tmp2[0].x) / 2.0;
	for (int i = 0; i < result->rows; i++)
	{
		uchar* in_data = result->ptr<uchar>(i);
		for (int j = 0; j < result->cols; j++)
		{
			if (((j - center.x)*(j - center.x) + (i - center.y)*(i - center.y)) > r*r)
			{
				(*in_data) = 255;
				in_data++;
				(*in_data) = 255;
				in_data++;
				(*in_data) = 255;
				in_data++;
				continue;
			}
			in_data += 3;
		}
	}
}

///选择平台

Circle _platform;    // 平台的位置
Point _platform_origin;   // 确认平台时 鼠标上一帧位置
int _platform_image_cols,_platform_image_rows;  // 确认平台时 视频大小
bool _move_platform = false, _scale_platform = false;   //判断鼠标操作 扩大圆形 还是 移动圆

Circle select_platform(string window_name, Scalar platform_color,Mat *image)
{
	Mat temp = image->clone();
    _platform_image_cols = image->cols;
    _platform_image_rows = image->rows;
	_platform = Circle(Point(10, 10), 10);
	namedWindow(window_name, WINDOW_AUTOSIZE);
	setMouseCallback(window_name, on_select_platform, 0);
	imshow(window_name, temp);
	int c = waitKey(30);
	//回车
	while (c != 13 || _move_platform || _scale_platform)
	{
		c = waitKey(10);
		temp = image->clone();
		circle(temp, _platform.center, _platform.r, platform_color);
		imshow(window_name, temp);
	}

	destroyWindow(window_name);
	return _platform;
}

void on_select_platform(int event, int x, int y, int, void*)
{
    Point now = Point(x, y);
    if (_move_platform)
    {
        _platform.center.x += x - _platform_origin.x;
        _platform.center.y += y - _platform_origin.y;
        if (_platform.center.x - _platform.r < 0)
        {
            _platform.center.x = _platform.r;
        }
        if (_platform.center.y - _platform.r < 0)
        {
            _platform.center.y = _platform.r;
        }
        if (_platform.center.x + _platform.r >= _platform_image_cols)
        {
            _platform.center.x = _platform_image_cols - _platform.r;
        }
        if (_platform.center.y + _platform.r >= _platform_image_rows)
        {
            _platform.center.y = _platform_image_rows - _platform.r;
        }
        _platform_origin = now;
    }
    if (_scale_platform)
    {
        _platform.r += x - _platform_origin.x;
        if (_platform.r < 10)
        {
            _platform.r = 10;
        }
        if (_platform.center.x - _platform.r < 0)
        {
            _platform.r = _platform.center.x;
        }
        if (_platform.center.y - _platform.r < 0)
        {
            _platform.r = _platform.center.y;
        }
        if (_platform.center.x + _platform.r >= _platform_image_cols)
        {
            _platform.r = _platform_image_cols - _platform.center.x;
        }
        if (_platform.center.y + _platform.r >= _platform_image_rows)
        {
            _platform.r = _platform_image_rows - _platform.center.y;
        }
        _platform_origin = now;
    }
    switch (event)
    {
        case CV_EVENT_LBUTTONDOWN:
            _platform_origin = Point(x, y);
            if (_platform.contains(_platform_origin))
            {
                _move_platform = true;
            }
            else
            {
                _scale_platform = true;
            }
            break;
        case CV_EVENT_LBUTTONUP:
            _move_platform = false;
            _scale_platform = false;
            break;
    }
}

///选择水池

Rect _pool;	//水池位置 大小
Point _pool_origin;   // 确认水池时 鼠标上一帧位置
int _pool_image_cols,_pool_image_rows;  // 确认平台时 视频大小
bool _pool_select = false, _pool_selected=false;

Rect select_pool(string window_name, Scalar pool_color,Mat *image)
{
    Mat temp = image->clone();
    _pool_image_cols = image->cols;
    _pool_image_rows = image->rows;
    namedWindow(window_name, WINDOW_AUTOSIZE);
    setMouseCallback(window_name, on_pool_mouse, 0);
    imshow(window_name, temp);
    waitKey(30);
    while (!_pool_selected)
    {
        waitKey(30);
        temp = image->clone();
        if (_pool.width > 0 && _pool.height > 0)
        {
            ellipse(temp, RotatedRect(Point2f(_pool.x + _pool.width / 2, _pool.y + _pool.height / 2), _pool.size(), 0), pool_color, 1, CV_AA);
        }
        imshow(window_name, temp);
    }
    ellipse(temp, RotatedRect(Point2f(_pool.x + _pool.width / 2, _pool.y + _pool.height / 2), _pool.size(),0), pool_color, 1, CV_AA);
    imshow(window_name, temp);
    destroyWindow(window_name);
    return _pool;
}

static void on_pool_mouse(int event, int x, int y, int, void *)
{
    if (_pool_select)
    {
        _pool.x = MIN(x, _pool_origin.x);
        _pool.y = MIN(y, _pool_origin.y);
        _pool.width = abs(x - _pool_origin.x);
        _pool.height = abs(y - _pool_origin.y);

        _pool &= Rect(0, 0, _pool_image_cols, _pool_image_rows);
    }

    switch (event)
    {
        case CV_EVENT_LBUTTONDOWN:
            _pool_origin = Point(x, y);
            _pool = Rect(x, y, 0, 0);
            _pool_select = true;
            break;
        case CV_EVENT_LBUTTONUP:
            _pool_select = false;
            if (_pool.width > 0 && _pool.height > 0)
                _pool_selected = false;
            _pool_selected = true;
            break;
    }
}