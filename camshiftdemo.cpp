#include <opencv2/video/tracking.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <queue>
#include <cmath>
#include <ctype.h>
#include "Vector.h"
#define INF 0x7f7f7f7f
using namespace cv;
using namespace std;
const int WIDTH = 400;
const int HEIGTH = 400;
const double EPS = 1e-6;
const double ANGLE_LIMIT = 30;
const int maxShow = 50;	//同时显示50个
const int judgeTime = 2000;		//2000ms
const Point2f EMPTY_POINT = Point2f(-1.0f, -1.0f);
const Scalar platformColor = Scalar(0, 0, 255);
Vector vec, mouseHead, lastLegal;
Mat image;

//椭圆参数
double vAxis, hAxis;
double centerX, centerY;

bool backprojMode = false;
bool selectObject = false;
bool movePlatform = false, scalePlatform = false;
bool platformConfirm = false, poolConfirm = false;
int trackObject = 0;
bool showHist = true;
Point origin;
Rect selection;
int vmin = 10, vmax = 110, smin = 50;
int tmin = 90, tmax = 200;
vector<double> speeds;
vector<double> whishaws;

//const string url = "G://视频追踪//1.mp4";
//const string url = "G://视频追踪//1.mp4";
const string url = "F://879404301//v1.wmv";
//const string url = "G://视频追踪//4.mpg";
//const string url = "G://视频追踪//morris.mp4"; 
//const string url = "C:\\Users\\Administrator\\source\\repos\\OpenCV\\x64\\Debug\\mean_shift_video.avi";

double calDistance(Point2f p1, Point2f p2);

struct Circle
{
	Point center;
	double r;
	Circle() {};
	Circle(Point center, double r)
	{
		this->center = center;
		this->r = r;
	}
	bool contains(Point pt)
	{
		double dis = calDistance(pt, center);
		return dis >= r ? false : true;
	}
};
Circle platform;

struct relativePoint
{
	Point point;
	double distance;
	relativePoint(Point point, double distance)
	{
		this->point = point;
		this->distance = distance;
	}
	friend bool operator<(relativePoint p1,relativePoint p2)
	{
		return p1.distance > p2.distance;
	}
};

float toRad(float angle)
{
	return angle * CV_PI / 180.0f;
}

double calDistance(Point2f p1, Point2f p2)
{
	return sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));
}

static void onMouse(int event, int x, int y, int, void*)
{
	if (selectObject)
	{
		selection.x = MIN(x, origin.x);
		selection.y = MIN(y, origin.y);
		selection.width = abs(x - origin.x);
		selection.height = abs(y - origin.y);

		selection &= Rect(0, 0, image.cols, image.rows);
	}

	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		origin = Point(x, y);
		selection = Rect(x, y, 0, 0);
		selectObject = true;
		break;
	case CV_EVENT_LBUTTONUP:
		selectObject = false;
		if (selection.width > 0 && selection.height > 0)
			trackObject = -1;
		break;
	}
}

static void onSelectPlatform(int event, int x, int y, int, void*)
{
	Point now = Point(x, y);
	if (movePlatform)
	{
		platform.center.x += x - origin.x;
		platform.center.y += y - origin.y;
		if (platform.center.x - platform.r < 0)
		{
			platform.center.x = platform.r;
		}
		if (platform.center.y - platform.r < 0)
		{
			platform.center.y = platform.r;
		}
		if (platform.center.x + platform.r >= image.cols)
		{
			platform.center.x = image.cols - platform.r;
		}
		if (platform.center.y + platform.r >= image.rows)
		{
			platform.center.y = image.rows - platform.r;
		}
		origin = now;
	}
	if (scalePlatform)
	{
		platform.r += x - origin.x;
		if (platform.r < 10)
		{
			platform.r = 10;
		}
		if (platform.center.x - platform.r < 0)
		{
			platform.r = platform.center.x;
		}
		if (platform.center.y - platform.r < 0)
		{
			platform.r = platform.center.y;
		}
		if (platform.center.x + platform.r >= image.cols)
		{
			platform.r = image.cols - platform.center.x;
		}
		if (platform.center.y + platform.r >= image.rows)
		{
			platform.r = image.rows - platform.center.y;
		}
		origin = now;
	}
	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		origin = Point(x, y);
		if (platform.contains(origin))
		{
			movePlatform = true;
		}
		else
		{
			scalePlatform = true;
		}
		break;
	case CV_EVENT_LBUTTONUP:
		movePlatform = false;
		scalePlatform = false;
		break;
	}
}

///找頭
vector<relativePoint> findHeadSortPoint(Mat *image)
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
		distance = calDistance(v[i], center);
		res.push_back(relativePoint(v[i], distance));
	}
	sort(res.begin(), res.end());
	return res;
}

Point findHead(Mat *image,Mat *image2 ,float x, float y)
{
	vector<relativePoint> res = findHeadSortPoint(image);
	for (int i = 0; i < res.size(); ++i)
	{
		circle(*image2, res[i].point, 2, Scalar(0, 0, 255), -1);
	}
	return Point(res[0].point.x + x, res[0].point.y + y);
}

Point findHead(Mat *image, float x, float y)
{
	vector<relativePoint> res = findHeadSortPoint(image);

	return Point(res[0].point.x + x ,res[0].point.y + y);
}

Point findHead(Mat *image)
{
	vector<relativePoint> res = findHeadSortPoint(image);

	return res[0].point;
}


///選擇平台
void selectPlatform(Mat *image)
{
	Mat temp = image->clone();
	platform = Circle(Point(10, 10), 10);
	namedWindow("selectPlatform", WINDOW_AUTOSIZE);
	setMouseCallback("selectPlatform", onSelectPlatform, 0);
	imshow("selectPlatform", temp);
	int c = waitKey(30);
	//回车
	while (c != 13 || movePlatform || scalePlatform)
	{
		c = waitKey(10);
		temp = image->clone();
		circle(temp, platform.center, platform.r, platformColor);
		imshow("selectPlatform", temp);
	}

	destroyWindow("selectPlatform");
	platformConfirm = true;
}

Rect pool;
bool poolSelect = false, poolSelected=false;
static void onPoolMouse(int event, int x, int y, int, void*)
{
	if (poolSelect)
	{
		pool.x = MIN(x, origin.x);
		pool.y = MIN(y, origin.y);
		pool.width = abs(x - origin.x);
		pool.height = abs(y - origin.y);

		pool &= Rect(0, 0, image.cols, image.rows);
	}

	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		origin = Point(x, y);
		pool = Rect(x, y, 0, 0);
		poolSelect = true;
		break;
	case CV_EVENT_LBUTTONUP:
		poolSelect = false;
		if (pool.width > 0 && pool.height > 0)
			poolSelected = false;
		poolSelected = true;
		break;
	}
}

///選擇水池
void selectPool(Mat *image)
{
	Mat temp = image->clone();
	namedWindow("selectPool", WINDOW_AUTOSIZE);
	setMouseCallback("selectPool", onPoolMouse, 0);
	imshow("selectPool", temp);
	waitKey(30);
	while (!poolSelected)
	{
		waitKey(30);
		temp = image->clone();
		if (pool.width > 0 && pool.height > 0)
		{
			ellipse(temp, RotatedRect(Point2f(pool.x + pool.width / 2, pool.y + pool.height / 2), pool.size(), 0), Scalar(0, 0, 255), 1, CV_AA);
		}
		imshow("selectPool", temp);
	}
	ellipse(temp, RotatedRect(Point2f(pool.x + pool.width / 2, pool.y + pool.height / 2), pool.size(),0), Scalar(0, 0, 255), 1, CV_AA);
	imshow("selectPool", temp);
	//waitKey(100000);
	hAxis = pool.width /2.0;
	vAxis = pool.height/2.0;
	centerX = pool.x + pool.width / 2.0;
	centerY = pool.y + pool.height / 2.0;

	destroyWindow("selectPool");
	poolConfirm = true;
}

static void help()
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

const char* keys =
{
	"{ 1 |  | 0 | camera number }"
};


/*
元素==点 对应的元素
通道 元素对应的表示方式
elemSize() 每个元素大小，单位字节
elemSize1() 每个通道大小，单位字节
例8UC3
每个通道大小为1B 8bit
每个元素大小1B*3
image.data[3*k,3*k+1,3*k+2]代表第k个点 对应3个通道
uchar*
*/

// BGR2GRAY by hjc

void colorReduce(const Mat& image, Mat& outImage, int div)
{
	int nr = image.rows;
	int nc = image.cols;
	outImage.create(image.size(), image.depth());
	if (image.isContinuous() && outImage.isContinuous())
	{
		nr = 1;
		nc = nc * image.rows*image.channels();
	}
	for (int i = 0; i < nr; i++)
	{
		const uchar* inData = image.ptr<uchar>(i);
		uchar* outData = outImage.ptr<uchar>(i);

		for (int j = 0; j < nc / 3; j++)
		{
			int tmp;
			//tmp = ((*inData++) + (*inData++) + (*inData++)) / 3;	//Æ½¾ùÖµ·¨
			tmp = ((*inData++) * 77 + (*inData++) * 151 + (*inData++) * 28) >> 8;
			*outData++ = tmp;
		}
	}
}

//void resizeRect(Rect2f *rectRange, Mat *image)
//{
//	if (rectRange->x > image->cols)
//	{
//		rectRange->x = image->cols;
//	}
//	if (rectRange->y > image->rows)
//	{
//		rectRange->y = image->rows;
//	}
//	if (rectRange->x < 0)
//	{
//		rectRange->x = 0;
//	}
//	if (rectRange->y < 0)
//	{
//		rectRange->y = 0;
//	}
//	if (rectRange->x + rectRange->width > image->cols)
//	{
//		rectRange->width = image->cols - rectRange->x;
//	}
//	if (rectRange->y + rectRange->height > image->rows)
//	{
//		rectRange->height = image->rows - rectRange->y;
//	}
//}

//计算2个向量cpt1 cpt2的夹角
float getAngelOfTwoVector(Point2f &pt1, Point2f &pt2, Point2f &c)
{
	float theta = atan2(pt1.x - c.x, pt1.y - c.y) - atan2(pt2.x - c.x, pt2.y - c.y);
	if (theta > CV_PI)
		theta -= 2 * CV_PI;
	if (theta < -CV_PI)
		theta += 2 * CV_PI;

	theta = theta * 180.0 / CV_PI;
	return theta;
}

void paintSpeedGraph()
{
	Mat speedMat = Mat(WIDTH, HEIGTH, CV_8SC3,Scalar(255,255,255));
	double dx = WIDTH;
	dx = 1.0*dx /50;
	int size = speeds.size();
	if (size < 2)
	{
		return;
	}
	int index = (size - maxShow) < 0 ? 0 : (size - maxShow);
	for (int i = index; i < size - 1; i++)
	{
		line(speedMat, Point2f(dx*(i + 1 - index), HEIGTH - speeds[i]), Point2f(dx*(i + 2 - index), HEIGTH - speeds[i + 1]), Scalar(0, 255, 0), 2);
	}
	imshow("Speed", speedMat);
}

void paintWhishawGraph()
{
	Mat whishawMat = Mat(WIDTH, HEIGTH, CV_8SC3, Scalar(255, 255, 255));
	double dx = WIDTH;
	dx = 1.0*dx / 50;
	int size = speeds.size();
	if (size < 2)
	{
		return;
	}
	int index = (size - maxShow) < 0 ? 0 : (size - maxShow);
	for (int i = index; i < size - 1; i++)
	{
		line(whishawMat, Point2f(dx*(i + 1 - index), HEIGTH - speeds[i]/180*400), Point2f(dx*(i + 2 - index), HEIGTH - speeds[i + 1] / 180 * 400), Scalar(0, 255, 0), 2);
	}
	imshow("Whishaw", whishawMat);
}

void findContourImage(CvMat* srcImage, CvMemStorage **pcvMStorage, CvSeq **pcvSeq)
{
	IplImage *g_pGrayImage = cvCreateImage(cvGetSize(srcImage), IPL_DEPTH_8U, 1);
	cvCvtColor(srcImage, g_pGrayImage, CV_BGR2GRAY);
	IplImage *g_pBinaryImage = cvCreateImage(cvGetSize(srcImage), IPL_DEPTH_8U, 1);
	//CV_THRESH_BINARY_INV CV_THRESH_OTSU
	cvThreshold(g_pGrayImage, g_pBinaryImage, 0, 255, CV_THRESH_OTSU);

	char *c = g_pBinaryImage->imageData;
	if (*c == -1)
	{
		for(int i = 0; i < g_pBinaryImage->imageSize; i++)
		{
			
				*c = (*c==-1?0:-1);
				c++;
		}
	}
	cvNamedWindow("二值图", 0);
	cvShowImage("二值图", g_pBinaryImage);
	//CV_RETR_CCOMP   CV_RETR_TREE
	int size = cvFindContours(g_pBinaryImage, *pcvMStorage, pcvSeq, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
	cvReleaseImage(&g_pGrayImage);
	cvReleaseImage(&g_pBinaryImage);

	if (pcvSeq != NULL && (*pcvSeq)->first->count == 4 && size > 1)
	{
		(*pcvSeq) = (*pcvSeq)->v_next;
	}
}

bool judgeInEllipse(int x, int y)
{
	double res = ((x - centerX)*(x - centerX)) / (hAxis*hAxis) + ((y-centerY)*(y-centerY)) /(vAxis*vAxis);
	if (res > 1.0)
	{
		return false;
	}
	return true;
}

int main(int argc, const char** argv)
{
	help();

	vector<Point2f> points;
	//CV_CAP_PROP_POS_MSEC 视频捕获时间戳
	VideoCapture cap;
	Rect trackWindow;
	int hsize = 16;//每一维上直方图的元素个数

	//https://zhidao.baidu.com/question/582446888429348885.html
	float hranges[] = { 0,180 };
	const float* phranges = hranges;


	//CommandLineParser parser(argc, argv, keys);
	//int camNum = parser.get<int>("1");

	//cap.open(camNum);
	cap.open(url);

	if (!cap.isOpened())
	{
		help();
		cout << "***Could not initialize capturing...***\n";
		cout << "Current parameter's value: \n";
		//parser.printErrors();
		return -1;
	}



	Mat frame, hsv, hue, mask, hist, histimg = Mat::zeros(200, 320, CV_8UC3), backproj;
	int lstTime = -1, nowTime = -1, inPlatform = INF;	//ms
	double totalMove = 0;
	bool paused = false, over = false;
	for (;;)
	{
		if (!paused)
		{
			cap >> frame;
			//cout <<"channels"<< frame.channels()<<endl;
			if (frame.empty())
			{
				lstTime = -1;
				nowTime = -1;
				totalMove = 0;
				over = false;
				points.clear();
				speeds.clear();
				whishaws.clear();
				cap.release();
				cap.open(url);
				cap >> frame;
			}
			if (!platformConfirm)
			{
				frame.copyTo(image);
				selectPlatform(&frame);
				selectPool(&frame);
				namedWindow("Histogram", 0);
				namedWindow("CamShift Demo", WINDOW_AUTOSIZE);
				namedWindow("Speed", 0);
				namedWindow("Whishaw", 0);
				setMouseCallback("CamShift Demo", onMouse, 0);
				//createTrackbar滑动控件
				createTrackbar("Vmin", "CamShift Demo", &vmin, 256, 0);
				createTrackbar("Vmax", "CamShift Demo", &vmax, 256, 0);
				createTrackbar("Smin", "CamShift Demo", &smin, 256, 0);
				createTrackbar("Tmin", "CamShift Demo", &tmin, 256, 0);
				createTrackbar("Tmax", "CamShift Demo", &tmax, 256, 0);
			}
		}
		frame.copyTo(image);

		/*{uchar *c = image.data;
		for (int i = 0; i < image.cols; i++)
		{
			for (int j = 0; j < image.cols; j++)
			{
				cout << (int)*c << " ";
				c++;
				if (!(j % 8))
				{
					waitKey(100000);
				}
			}
			cout << endl;
			waitKey(100000);
		}}*/

		if (!paused)
		{
			//把image 从BGR转成HSV 存到hsv  抓帧出来后图片是BGR顺序储存的元素
			//hsv 色调H 饱和度S 明度V
			/*
			max=max(R,G,B)；
			min=min(R,G,B)；
			V=max(R,G,B)；
			S=(max-min)/max；
			if (R = max) H = (G-B)/(max-min)* 60；
			if (G = max) H = 120+(B-R)/(max-min)* 60；
			if (B = max) H = 240 +(R-G)/(max-min)* 60；
			if (H < 0) H = H+ 360；
			*/

			///删除水池外的点
			for (int i = 0; i < image.rows; i++)
			{
				uchar* inData = image.ptr<uchar>(i);
				for (int j = 0; j < image.cols; j++)
				{
					if (!judgeInEllipse(j, i))
					{
						(*inData) = 255;
						inData++;
						(*inData) = 255;
						inData++;
						(*inData) = 255;
						inData++;
						continue;
					}
					inData+=3;
				}
			}

			cvtColor(image, hsv, COLOR_BGR2HSV);


			if (trackObject)
			{
				int _vmin = vmin, _vmax = vmax;
				//inRange hsv[i]的三个通道的每个元素是否位于[0,180][smin,256][MIN(_vmin, _vmax),MAX(_vmin, _vmax)]区间，如果是，mask[i]设为255，否则为0。
				inRange(hsv, Scalar(0, smin, MIN(_vmin, _vmax)),
					Scalar(180, 256, MAX(_vmin, _vmax)), mask);

				int ch[] = { 0, 0 };
				//depth每一个像素的位数(bits)enum { CV_8U=0, CV_8S=1, CV_16U=2, CV_16S=3, CV_32S=4, CV_32F=5, CV_64F=6 };
				hue.create(hsv.size(), hsv.depth());
				//ch 为将hsv的ch[2*k]通道拷贝到hue的第[2*k+1]通道 1代表k[0,1)
				mixChannels(&hsv, 1, &hue, 1, ch, 1);
				Mat roi(hue, selection), maskroi(mask, selection);
				if (trackObject < 0)
				{
					//重新选中清空
					points.clear();
					//提取选中区域

					//计算roi第0通道 maskroi对应点的 1维每一维16个元素直方图 phranges上限180下限0
					calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
					//归一化 http://blog.csdn.net/solomon1558/article/details/44689611
					normalize(hist, hist, 0, 255, CV_MINMAX);

					trackWindow = selection;
					trackObject = 1;

					histimg = Scalar::all(0);
					int binW = histimg.cols / hsize;
					//CV_ + (位数）+（数据类型）+（通道数）
					Mat buf(1, hsize, CV_8UC3);
					for (int i = 0; i < hsize; i++)
						buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180. / hsize), 255, 255);
					cvtColor(buf, buf, CV_HSV2BGR);

					//画图
					for (int i = 0; i < hsize; i++)
					{
						int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows / 255);
						rectangle(histimg, Point(i*binW, histimg.rows),
							Point((i + 1)*binW, histimg.rows - val),
							Scalar(buf.at<Vec3b>(i)), -1, 8);
					}
				}
				//反向投影 根据hue.data[i] 对应区域的对应hist.data的值 重新赋值
				calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
				backproj &= mask;

				RotatedRect trackBox = CamShift(backproj, trackWindow,
					TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1));
				if (trackWindow.area() <= 1)
				{
					int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5) / 6;
					trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
						trackWindow.x + r, trackWindow.y + r) &
						Rect(0, 0, cols, rows);
				}

				if (backprojMode)
					cvtColor(backproj, image, COLOR_GRAY2BGR);
				//ellipse(image, trackBox, Scalar(0, 0, 255), 1, CV_AA);

				Size size = trackBox.size;
				if (size.width != 0 && size.height != 0)
				{
					points.push_back(trackBox.center);
					vec = Vector(vec.end, trackBox.center);

					size.width += 15;
					size.height += 15;
					Point tmpPoint = trackBox.center;
					trackBox = RotatedRect(tmpPoint, size, trackBox.angle);
					Rect2f rectRange;
					rectRange = trackBox.boundingRect2f();

					rectRange &= Rect2f(0, 0, image.cols, image.rows);
					//resizeRect(&rectRange, &image);

					Mat targetImage = Mat(image, rectRange);

					CvMemStorage *pcvMStorage = cvCreateMemStorage();
					CvSeq *pcvSeq = NULL;
					findContourImage(&(CvMat)targetImage, &pcvMStorage, &pcvSeq);

					// 画轮廓图  
					IplImage *pOutlineImage = cvCreateImage(cvGetSize(&(CvMat)targetImage), IPL_DEPTH_8U, 3);
					int nLevels = 2;
					// 填充成白色  
					cvRectangle(pOutlineImage, cvPoint(0, 0), cvPoint(pOutlineImage->width, pOutlineImage->height), CV_RGB(255, 255, 255), CV_FILLED);
					cvDrawContours(pOutlineImage, pcvSeq, CV_RGB(255, 0, 0), CV_RGB(0, 255, 0), nLevels, 1);
					// 显示轮廓图
					cvNamedWindow("cvFindContours轮廓图", 0);
					cvShowImage("cvFindContours轮廓图", pOutlineImage);

					nowTime = cap.get(CV_CAP_PROP_POS_MSEC);
					if (points.size() > 1 && !over)
					{
						int size = points.size();
						for (int i = 0; i < size - 1; i++)
						{
							line(image, points[i], points[i + 1], Scalar(255, 255, 0));
						}
						double disMove = calDistance(points[size - 1], points[size - 2]);
						totalMove += disMove;

						Mat target(mask, rectRange);
						
						//计算头部
						//Point head = findHead(&target, &image,rectRange.x, rectRange.y);

						//circle(image, head, 1, Scalar(0, 0, 255), -1);
						double angle = trackBox.angle;
						double kx, ky;
						kx = trackBox.center.x + 1.0*trackBox.size.height / 2 * sin(angle*CV_PI / 180);
						ky = trackBox.center.y - 1.0*trackBox.size.width / 2 * cos(angle*CV_PI / 180);
						//double angleHead = getAngelOfTwoVector(Point2f(kx,ky), points[size - 2], points[size - 1]);
						//Point2f head;
						//if (abs(angleHead) > 90)
						//{
						//	head = Point2f(kx, ky);
						//}
						//else
						//{
						//	kx = trackBox.center.x - 1.0*trackBox.size.height / 2 * sin(angle*CV_PI / 180);
						//	ky = trackBox.center.y + 1.0*trackBox.size.width / 2 * cos(angle*CV_PI / 180);
						//	head = Point2f(kx, ky);
						//}
						//cout << "size" << trackBox.size << trackBox.angle<< endl;
						//ellipse(image, trackBox, Scalar(0, 0, 255), 1, CV_AA);
						//cvLine(&(CvMat)image, trackBox.center, head, CV_RGB(255, 255, 255), 2);
						


						Point2f longAxisDir = trackBox.center;
						longAxisDir.x += sin(toRad(trackBox.angle)) * trackBox.size.width / 2;
						longAxisDir.y -= cos(toRad(trackBox.angle)) * trackBox.size.height / 2;
						mouseHead = Vector(trackBox.center, longAxisDir);

						if (vec.start.x != 0 && vec.start.y != 0)
						{
							double angle = mouseHead.getAngel(vec);
							if (angle > 90)
							{
								mouseHead.axisymmetric(mouseHead.start);
								angle = mouseHead.getAngel(vec);
							}

							bool trust = true;

							if (angle < 0)
							{
								double temp = lastLegal.getAngel(mouseHead);
								if (lastLegal.start.x != 0 && lastLegal.start.y != 0 && (temp < 0 || temp > ANGLE_LIMIT)) trust = false;
							}
							else if (angle > ANGLE_LIMIT)
							{
								trust = false;
							}

							if (trust)
							{
								line(image, mouseHead.start, mouseHead.end, Scalar(0, 0, 255));
								circle(image, mouseHead.end, 2, Scalar(0, 0, 255), -1);
								lastLegal = mouseHead;
							}
							else
							{
								mouseHead.end.x = mouseHead.start.x + lastLegal.end.x - lastLegal.start.x;
								mouseHead.end.y = mouseHead.start.y + lastLegal.end.y - lastLegal.start.y;
								line(image, mouseHead.start, mouseHead.end, Scalar(0, 0, 255));
								circle(image, mouseHead.end, 2, Scalar(0, 0, 255), -1);
							}
						}
						double whishaw = getAngelOfTwoVector(points[size - 1], (Point2f)platform.center, points[size - 2]);
						double speed = 1000 * disMove / (nowTime - lstTime);
						speeds.push_back(speed);
						whishaws.push_back(whishaw);
						//角度差
						cout << "Current speed:" << speed << "px/s		Whishaw:" << whishaw << endl;
						paintSpeedGraph();
						paintWhishawGraph();
					}
					if (platform.contains(points.back()))
					{
						if (inPlatform == INF)
						{
							inPlatform = nowTime;
						}
						if (!over&&nowTime - inPlatform >= judgeTime)
						{
							over = true;
							cout << "Total distance:" << totalMove << "px	In platform time" << nowTime - judgeTime << "ms	Average speed:" << totalMove / (nowTime - judgeTime) * 1000 << "px/s" << endl;
						}
					}
					else
					{
						inPlatform = nowTime;
					}
					lstTime = nowTime;
					cvReleaseMemStorage(&pcvMStorage);
					cvReleaseImage(&pOutlineImage);
				}
			}
		}
		else if (trackObject < 0)
			paused = false;

		if (selectObject && selection.width > 0 && selection.height > 0)
		{
			Mat roi(image, selection);
			bitwise_not(roi, roi);
		}

		circle(image, platform.center, platform.r, platformColor);
		imshow("CamShift Demo", image);
		imshow("Histogram", histimg);

		char c = (char)waitKey(25);
		if (c == 27) break; // Esc
		switch (c)
		{
		case 'b':
			backprojMode = !backprojMode;
			break;
		case 'c':
			trackObject = 0;
			histimg = Scalar::all(0);
			break;
		case 'h':
			showHist = !showHist;
			if (!showHist)
				destroyWindow("Histogram");
			else
				namedWindow("Histogram", 1);
			break;
		case 'p':
			paused = !paused;
			break;
		default:
			;
		}
	}
	return 0;
}
