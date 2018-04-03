#include "initiation.h"

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