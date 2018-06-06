//#include "opencv2/core.hpp"
//#include <opencv2/core/utility.hpp>
//#include "opencv2/imgproc.hpp"
//#include "opencv2/video/background_segm.hpp"
//#include "opencv2/videoio.hpp"
//#include "opencv2/highgui.hpp"
//#include <stdio.h>

//using namespace std;
//using namespace cv;
//const string url = "D://tencent//879404301//FileRecv//blackmouse.mp4";
//
////this is a sample for foreground detection functions
//int main(int argc, const char** argv)
//{
//
//	VideoCapture cap;
//	bool update_bg_model = true;
//
//	cap.open(url);
//	if (!cap.isOpened())
//	{
//		printf("can not open camera or video file\n");
//		return -1;
//	}
//
//	namedWindow("image", WINDOW_NORMAL);
//	namedWindow("foreground mask", WINDOW_NORMAL);
//	namedWindow("foreground image", WINDOW_NORMAL);
//	namedWindow("mean background image", WINDOW_NORMAL);
//
//	Ptr<BackgroundSubtractor> bg_model = createBackgroundSubtractorKNN().dynamicCast<BackgroundSubtractor>();	//以高斯混合模型为基础的背景/前景分割算法
//		//createBackgroundSubtractorMOG2().dynamicCast<BackgroundSubtractor>();
//
//	Mat img0, img, fgmask, fgimg;
//
//	for (;;)
//	{
//		cap >> img0;
//
//		if (img0.empty())
//			break;
//
//		resize(img0, img, Size(640, 640 * img0.rows / img0.cols), INTER_LINEAR);
//
//		if (fgimg.empty())
//			fgimg.create(img.size(), img.type());
//
//		//update the model
//		bg_model->apply(img, fgmask, update_bg_model ? -1 : 0);
//		if (true)
//		{
//			GaussianBlur(fgmask, fgmask, Size(11, 11), 3.5, 3.5);
//			threshold(fgmask, fgmask, 10, 255, THRESH_BINARY);
//		}
//
//		fgimg = Scalar::all(0);
//		img.copyTo(fgimg, fgmask);
//
//		Mat bgimg;
//		bg_model->getBackgroundImage(bgimg);
//
//		imshow("image", img);
//		imshow("foreground mask", fgmask);
//		imshow("foreground image", fgimg);
//		if (!bgimg.empty())
//			imshow("mean background image", bgimg);
//
//		char k = (char)waitKey(30);
//		if (k == 27) break;
//		if (k == ' ')
//		{
//			update_bg_model = !update_bg_model;
//			if (update_bg_model)
//				printf("Background update is on\n");
//			else
//				printf("Background update is off\n");
//		}
//	}
//
//	return 0;
//}

//using namespace std;
//using namespace cv;
//const string url = "D://tencent//879404301//FileRecv//whitemouse.mp4";
//
////this is a sample for foreground detection functions
//int main(int argc, const char** argv)
//{
//
//	VideoCapture cap;
//	bool update_bg_model = true;
//
//	cap.open(url);
//	if (!cap.isOpened())
//	{
//		printf("can not open camera or video file\n");
//		return -1;
//	}
//
//	Mat frame;
//	namedWindow("input", CV_WINDOW_AUTOSIZE);
//	namedWindow("MOG2", CV_WINDOW_AUTOSIZE);
//	namedWindow("KNN", CV_WINDOW_AUTOSIZE);
//	Mat maskMOG2, maskKNN;
//	Ptr<BackgroundSubtractor> pMOG2 = createBackgroundSubtractorMOG2(500, 25, true);
//	Ptr<BackgroundSubtractor> pKNN = createBackgroundSubtractorKNN();
//
//	Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
//	while (cap.read(frame))
//	{
//		imshow("input", frame);
//
//		pMOG2->apply(frame, maskMOG2);
//		pKNN->apply(frame, maskKNN);
//		//对处理后的帧进行开操作，减少视频中较小的波动造成的影响
//		morphologyEx(maskMOG2, maskMOG2, MORPH_OPEN, kernel, Point(-1, -1));
//		morphologyEx(maskKNN, maskKNN, MORPH_OPEN, kernel, Point(-1, -1));
//
//		imshow("MOG2", maskMOG2);
//		imshow("KNN", maskKNN);
//		waitKey(3);
//	}
//
//	cap.release();
//
//	return 0;
//}

#include <opencv2/video/tracking.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/calib3d.hpp"

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
const int judge_time = 2000;		//2000ms
const Scalar PLATFORM_COLOR = Scalar(0, 0, 255);	// 选取平台颜色
const Scalar POOL_COLOE = Scalar(0, 0, 255);	// 选取水池颜色
const string WHISHAW_WINDOW_NAME = "Whishaws", SPEED_WINDOW_NAME = "Speed", PLATFORM_WINDOW_NAME = "SelectPlatform", POOL_WINDOW_NAME = "SelectPool", HISTOGRAM_WINDOW = "Histogram", MAIN_WINDOW = "main";
Vector vec, mouse_head, last_legal;
Mat image;

bool backproj_mode = false;
bool select_object = false;		// 是否已选择老鼠位置
bool platform_confirm = false; // 是否已确认水迷宫平台的位置
int track_object = 0;
bool show_hist = true;
double total_move = 0;	// 记录总位移

Rect selection;	// 标记小鼠位置
Circle platform; // 平台的位置 大小
Rect pool;	// 水池位置

//水池位置转成椭圆参数
//double v_axis, h_axis;
//Point2f pool_center;

// hsv过滤范围
int v_min = 10, v_max = 110, s_min = 50;
vector<double> speeds;
vector<double> whishaws;

correction co;

//const string url = "G://视频追踪//1.mp4";
//const string url = "G://视频追踪//1.mp4";
//const string url = "D://tencent//879404301//FileRecv//Video 1.wmv";
//const string url = "D://tencent//879404301//FileRecv//blackmouse.mp4";
//const string url = "D://tencent//879404301//FileRecv//whitemouse.mp4";
//const string url = "F://879404301//v1.wmv";
const string url = "D://tencent//879404301//FileRecv//v2.wmv";
//const string url = "G://视频追踪//4.mpg";
//const string url = "G://视频追踪//morris.mp4"; 
//const string url = "D:\\tencent\\879404301\\FileRecv\\mouse1.mp4";


Point origin;   // 鼠标上一帧位置

/**
 * 确认小鼠位置时鼠标事件
 */
static void on_mouse(int event, int x, int y, int, void *)
{
	if (select_object)
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
		select_object = true;
		break;
	case CV_EVENT_LBUTTONUP:
		select_object = false;
		if (selection.width > 0 && selection.height > 0)
			track_object = -1;
		break;
	}
}

/**
 * 找轮廓
 * @param srcImage 原图片
 * @param pcvMStorage 存储空间
 * @param pcvSeq 轮廓序列
 */
void find_contour_image(CvMat *srcImage, CvMemStorage **pcvMStorage, CvSeq **pcvSeq)
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


/**
 * 参数全部输入后 初始化直方图 小鼠跟踪 速度 偏差 4个窗口 并添加滑动控件
 */
void init()
{
	namedWindow(HISTOGRAM_WINDOW, 0);
	namedWindow(MAIN_WINDOW, WINDOW_AUTOSIZE);
	namedWindow(SPEED_WINDOW_NAME, 0);
	namedWindow(WHISHAW_WINDOW_NAME, 0);
	setMouseCallback(MAIN_WINDOW, on_mouse, 0);
	//createTrackbar滑动控件
	createTrackbar("Vmin", MAIN_WINDOW, &v_min, 256, 0);
	createTrackbar("Vmax", MAIN_WINDOW, &v_max, 256, 0);
	createTrackbar("Smin", MAIN_WINDOW, &s_min, 256, 0);
}


int main(int argc, const char** argv)
{
	software_help();
	vector<Point2f> points;
	//CV_CAP_PROP_POS_MSEC 视频捕获时间戳
	VideoCapture cap;
	Rect track_window;
	int h_size = 16;//每一维上直方图的元素个数

	//https://zhidao.baidu.com/question/582446888429348885.html
	float h_ranges[] = { 0,180 };
	const float* ph_ranges = h_ranges;

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
	int lst_time = -1, now_time = -1, in_platform = INF;	//ms
	bool paused = false, over = false;
	for (;;)
	{
		if (!paused)
		{
			cap >> frame;
			if (frame.empty())
			{
				lst_time = -1;
				now_time = -1;
				total_move = 0;
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
				
				co.init(&image);

				// 选择水池
				//pool = select_pool(POOL_WINDOW_NAME,POOL_COLOE,&frame);
				// 将结果的矩形转换成椭圆
				//h_axis = pool.width /2.0;
				//v_axis = pool.height/2.0;
				//pool_center = Point2f(pool.x + pool.width / 2.0, pool.y + pool.height / 2.0);

				init();
				//co.work(&frame, &image);
				platform = select_platform(PLATFORM_WINDOW_NAME, PLATFORM_COLOR, &image);
				platform_confirm = true;
				frame.copyTo(image);
				
			}
		}
		
		//frame.copyTo(image);

		if (!paused)
		{
			co.work(&frame, &image);

			cvtColor(image, hsv, COLOR_BGR2HSV);
			
			if (track_object)
			{
				int _vmin = v_min, _vmax = v_max;
				//inRange hsv[i]的三个通道的每个元素是否位于[0,180][smin,256][MIN(_vmin, _vmax),MAX(_vmin, _vmax)]区间，如果是，mask[i]设为255，否则为0。
				inRange(hsv, Scalar(0, s_min, MIN(_vmin, _vmax)),
					Scalar(180, 256, MAX(_vmin, _vmax)), mask);

				int ch[] = { 0, 0 };
				//depth每一个像素的位数(bits)enum { CV_8U=0, CV_8S=1, CV_16U=2, CV_16S=3, CV_32S=4, CV_32F=5, CV_64F=6 };
				hue.create(hsv.size(), hsv.depth());
				//ch 为将hsv的ch[2*k]通道拷贝到hue的第[2*k+1]通道 1代表k[0,1)
				mixChannels(&hsv, 1, &hue, 1, ch, 1);
				if (track_object < 0)
				{
					Mat roi(hue, selection), mask_roi(mask, selection);
					//重新选中清空
					points.clear();
					//提取选中区域

					//计算roi第0通道 maskroi对应点的 1维每一维16个元素直方图 phranges上限180下限0
					calcHist(&roi, 1, 0, mask_roi, hist, 1, &h_size, &ph_ranges);
					//归一化 http://blog.csdn.net/solomon1558/article/details/44689611
					normalize(hist, hist, 0, 255, CV_MINMAX);

					track_window = selection;
					track_object = 1;

					histimg = Scalar::all(0);
					int binW = histimg.cols / h_size;
					//CV_ + (位数）+（数据类型）+（通道数）
					Mat buf(1, h_size, CV_8UC3);
					for (int i = 0; i < h_size; i++)
						buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180. / h_size), 255, 255);
					cvtColor(buf, buf, CV_HSV2BGR);

					//画图
					for (int i = 0; i < h_size; i++)
					{
						int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows / 255);
						rectangle(histimg, Point(i*binW, histimg.rows),
							Point((i + 1)*binW, histimg.rows - val),
							Scalar(buf.at<Vec3b>(i)), -1, 8);
					}
				}

				//反向投影 根据hue.data[i] 对应区域的对应hist.data的值 重新赋值
				calcBackProject(&hue, 1, 0, hist, backproj, &ph_ranges);

				imshow("backprok before", backproj);

				backproj &= mask;
			
				imshow("backprok", backproj);

				RotatedRect track_box = CamShift(backproj, track_window,
					TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1));
				if (track_window.area() <= 1)
				{
					int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5) / 6;
					track_window = Rect(track_window.x - r, track_window.y - r,
						track_window.x + r, track_window.y + r) &
						Rect(0, 0, cols, rows);
				}

				if (backproj_mode)
					cvtColor(backproj, image, COLOR_GRAY2BGR);
				//ellipse(image, track_box, Scalar(0, 0, 255), 1, CV_AA);

				Size size = track_box.size;
				if (size.width == 0 || size.height == 0)
				{
					printf("window %d %d %d\n", track_window.area(), track_window.width, track_window.height);
					printf("box %d %d %d\n", track_box.size.area(), track_box.size.width, track_box.size.height);

					//printf("area");
				}
				if (size.width != 0 && size.height != 0)
				{
					points.push_back(track_box.center);
					vec = Vector(vec.end, track_box.center);

					size.width += 15;
					size.height += 15;
					Point tmp_point = track_box.center;
					track_box = RotatedRect(tmp_point, size, track_box.angle);
					Rect2f rectRange;
					rectRange = track_box.boundingRect2f();

					rectRange &= Rect2f(0, 0, image.cols, image.rows);
					//resizeRect(&rectRange, &image);

					Mat targetImage = Mat(image, rectRange);

					CvMemStorage *pcvMStorage = cvCreateMemStorage();
					CvSeq *pcvSeq = NULL;
					CvMat temp_target = targetImage;
					find_contour_image(&temp_target, &pcvMStorage, &pcvSeq);

					// 画轮廓图  
					IplImage *p_outline_image = cvCreateImage(cvGetSize(&temp_target), IPL_DEPTH_8U, 3);
					int nLevels = 2;
					// 填充成白色  
					cvRectangle(p_outline_image, cvPoint(0, 0), cvPoint(p_outline_image->width, p_outline_image->height), CV_RGB(255, 255, 255), CV_FILLED);
					cvDrawContours(p_outline_image, pcvSeq, CV_RGB(255, 0, 0), CV_RGB(0, 255, 0), nLevels, 1);
					// 显示轮廓图
//					cvNamedWindow("cvFindContours轮廓图", 0);
//					cvShowImage("cvFindContours轮廓图", p_outline_image);

					now_time = cap.get(CV_CAP_PROP_POS_MSEC);
					if (points.size() > 1 && !over)
					{
						int size = points.size();
						double disMove = cal_distance(points[size - 1], points[size - 2]);
						total_move += disMove;

						Mat target(mask, rectRange);

						Point2f long_axis_dir = track_box.center;
						long_axis_dir.x += sin(to_rad(track_box.angle)) * track_box.size.width / 2;
						long_axis_dir.y -= cos(to_rad(track_box.angle)) * track_box.size.height / 2;
						mouse_head = Vector(track_box.center, long_axis_dir);

						if (vec.start.x != 0 && vec.start.y != 0)
						{
							double angle = mouse_head.getAngel(vec);
							if (angle > 90)
							{
								mouse_head.axisymmetric(mouse_head.start);
								angle = mouse_head.getAngel(vec);
							}

							bool trust = true;

							if (angle < 0)
							{
								double temp = last_legal.getAngel(mouse_head);
								if (last_legal.start.x != 0 && last_legal.start.y != 0 && (temp < 0 || temp > ANGLE_LIMIT)) trust = false;
							}
							else if (angle > ANGLE_LIMIT)
							{
								trust = false;
							}

							if (trust)
							{
								line(image, mouse_head.start, mouse_head.end, Scalar(0, 0, 255));
								circle(image, mouse_head.end, 2, Scalar(0, 0, 255), -1);
								last_legal = mouse_head;
							}
							else
							{
								mouse_head.end.x = mouse_head.start.x + last_legal.end.x - last_legal.start.x;
								mouse_head.end.y = mouse_head.start.y + last_legal.end.y - last_legal.start.y;
								line(image, mouse_head.start, mouse_head.end, Scalar(0, 0, 255));
								circle(image, mouse_head.end, 2, Scalar(0, 0, 255), -1);
							}
						}
						Point2f center = (Point2f)platform.center;
						double whishaw = get_angel_of_two_vector(points[size - 1], center, points[size - 2]);
						double speed = 1000 * disMove / (now_time - lst_time);
						speeds.push_back(speed);
						whishaws.push_back(whishaw);
						//角度差
						cout << "Current speed:" << speed << "px/s		Whishaw:" << whishaw << endl;
//						paint_speed_graph(SPEED_WINDOW_NAME,&speeds);
//						paint_whishaw_graph(WHISHAW_WINDOW_NAME,&whishaws);
					}
					if (platform.contains(points.back()))
					{
						if (in_platform == INF)
						{
							in_platform = now_time;
						}
						if (!over&&now_time - in_platform >= judge_time)
						{
							over = true;
							cout << "Total distance:" << total_move << "px	In platform time" << now_time - judge_time << "ms	Average speed:" << total_move / (now_time - judge_time) * 1000 << "px/s" << endl;
						}
					}
					else
					{
						in_platform = now_time;
					}
					lst_time = now_time;
					cvReleaseMemStorage(&pcvMStorage);
					cvReleaseImage(&p_outline_image);
				}
				int psize = points.size();
				for (int i = 0; i < psize - 1; i++)
				{
					line(image, points[i], points[i + 1], Scalar(255, 255, 0));
				}
				rectangle(image, track_window, Scalar(255, 255, 255));
				/*if (size.width > 0 && size.height > 0)
					ellipse(image, track_box, Scalar(0, 0, 0), 3, LINE_AA);*/
				waitKey(100);
			}
		}
		else if (track_object < 0)
			paused = false;

		if (select_object && selection.width > 0 && selection.height > 0)
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
			backproj_mode = !backproj_mode;
			break;
		case 'c':
			track_object = 0;
			histimg = Scalar::all(0);
			break;
		case 'h':
			show_hist = !show_hist;
			if (!show_hist)
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
