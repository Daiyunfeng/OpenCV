#include <opencv2/video/tracking.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <queue>
#include <cmath>
#include <ctype.h>
#include "Vector.h"
#include "Circle.h"
#include "utils.h"
#include "initiation.h"
#define INF 0x7f7f7f7f
using namespace cv;
using namespace std;
const double ANGLE_LIMIT = 30;
const int judgeTime = 2000;		//2000ms
const Scalar PLATFORM_COLOR = Scalar(0, 0, 255);	// 选取平台颜色
const Scalar POOL_COLOE = Scalar(0, 0, 255);	// 选取水池颜色
const string WHISHAW_WINDOW_NAME = "Whishaws", SPEED_WINDOW_NAME = "Speed", PLATFORM_WINDOW_NAME = "SelectPlatform", POOL_WINDOW_NAME = "SelectPool", HISTOGRAM_WINDOW = "Histogram", MAIN_WINDOW = "Histogram";
Vector vec, mouseHead, lastLegal;
Mat image;

bool backprojMode = false;
bool selectObject = false;
bool platform_confirm = false; // 是否已确认水迷宫平台的位置
int trackObject = 0;
bool showHist = true;

Rect selection;	// 标记小鼠位置
Circle platform; // 平台的位置 大小
Rect pool;	// 水池位置
//水池位置转椭圆参数
double vAxis, hAxis;
double centerX, centerY;

int vmin = 10, vmax = 110, smin = 50;
int tmin = 90, tmax = 200;
vector<double> speeds;
vector<double> whishaws;

//const string url = "G://视频追踪//1.mp4";
//const string url = "G://视频追踪//1.mp4";
const string url = "D://tencent//879404301//FileRecv//v1.wmv";
//const string url = "F://879404301//v1.wmv";
//const string url = "G://视频追踪//4.mpg";
//const string url = "G://视频追踪//morris.mp4"; 
//const string url = "C:\\Users\\Administrator\\source\\repos\\OpenCV\\x64\\Debug\\mean_shift_video.avi";


Point origin;   // 鼠标上一帧位置
/**
 * 确认小鼠位置时鼠标事件
 */
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

const char* keys =
{
	"{ 1 |  | 0 | camera number }"
};

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

/**
 * 参数全部输入后 初始化直方图 小鼠跟踪 速度 偏差 4个窗口 并添加滑动控件
 */
void init()
{
	namedWindow(HISTOGRAM_WINDOW, 0);
	namedWindow(MAIN_WINDOW, WINDOW_AUTOSIZE);
	namedWindow(SPEED_WINDOW_NAME, 0);
	namedWindow(WHISHAW_WINDOW_NAME, 0);
	setMouseCallback(MAIN_WINDOW, onMouse, 0);
	//createTrackbar滑动控件
	createTrackbar("Vmin", MAIN_WINDOW, &vmin, 256, 0);
	createTrackbar("Vmax", MAIN_WINDOW, &vmax, 256, 0);
	createTrackbar("Smin", MAIN_WINDOW, &smin, 256, 0);
	createTrackbar("Tmin", MAIN_WINDOW, &tmin, 256, 0);
	createTrackbar("Tmax", MAIN_WINDOW, &tmax, 256, 0);
}

int main(int argc, const char** argv)
{
	software_help();
	vector<Point2f> points;
	//CV_CAP_PROP_POS_MSEC 视频捕获时间戳
	VideoCapture cap;
	Rect trackWindow;
	int hsize = 16;//每一维上直方图的元素个数

	//https://zhidao.baidu.com/question/582446888429348885.html
	float hranges[] = { 0,180 };
	const float* phranges = hranges;

	cap.open(url);

	if (!cap.isOpened())
	{
		software_help();
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
			if (!platform_confirm)
			{
				frame.copyTo(image);
				// 选择平台
				platform = select_platform(PLATFORM_WINDOW_NAME,PLATFORM_COLOR,&frame);
				// 选择水池
				pool = select_pool(POOL_WINDOW_NAME,POOL_COLOE,&frame);
				// 将结果的矩形转换成椭圆
				hAxis = pool.width /2.0;
				vAxis = pool.height/2.0;
				centerX = pool.x + pool.width / 2.0;
				centerY = pool.y + pool.height / 2.0;

				init();
				platform_confirm = true;
			}
		}
		frame.copyTo(image);

		if (!paused)
		{
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
					CvMat temp_target = targetImage;
					findContourImage(&temp_target, &pcvMStorage, &pcvSeq);

					// 画轮廓图  
					IplImage *pOutlineImage = cvCreateImage(cvGetSize(&temp_target), IPL_DEPTH_8U, 3);
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
						double disMove = cal_distance(points[size - 1], points[size - 2]);
						totalMove += disMove;

						Mat target(mask, rectRange);

						Point2f longAxisDir = trackBox.center;
						longAxisDir.x += sin(to_rad(trackBox.angle)) * trackBox.size.width / 2;
						longAxisDir.y -= cos(to_rad(trackBox.angle)) * trackBox.size.height / 2;
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
						Point2f center = (Point2f)platform.center;
						double whishaw = get_angel_of_two_vector(points[size - 1], center, points[size - 2]);
						double speed = 1000 * disMove / (nowTime - lstTime);
						speeds.push_back(speed);
						whishaws.push_back(whishaw);
						//角度差
						cout << "Current speed:" << speed << "px/s		Whishaw:" << whishaw << endl;
						paint_speed_graph(SPEED_WINDOW_NAME,&speeds);
						paint_whishaw_graph(WHISHAW_WINDOW_NAME,&whishaws);
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

		circle(image, platform.center, platform.r, PLATFORM_COLOR);
		imshow(MAIN_WINDOW, image);
		imshow(HISTOGRAM_WINDOW, histimg);

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
				destroyWindow(HISTOGRAM_WINDOW);
			else
				namedWindow(HISTOGRAM_WINDOW, 1);
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
