#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;


Point_<int> N8[8] = {
	{ -1, -1 }, { 0, -1 }, { 1, -1 },

	{ -1, 0 },   /*cc*/    { 1, 0 },

	{ -1, 1 },  { 0, 1 },  { 1, 1 }

}; // 8-neighbors

int cal_rgb_dis_square(unsigned char* p0, unsigned char* p1)
{
	return
		(p0[0] - p1[0])*(p0[0] - p1[0]) +
		(p0[1] - p1[1])*(p0[1] - p1[1]) +
		(p0[2] - p1[2])*(p0[2] - p1[2]);

}


//This colors the segmentations
static void floodFillPostprocess(Mat& img, const Scalar& colorDiff = Scalar::all(1))
{
	CV_Assert(!img.empty());
	RNG rng = theRNG();
	Mat mask(img.rows + 2, img.cols + 2, CV_8UC1, Scalar::all(0));
	for (int y = 0; y < img.rows; y++)
	{
		for (int x = 0; x < img.cols; x++)
		{
			if (mask.at<uchar>(y + 1, x + 1) == 0)
			{
				Scalar newVal(rng(256), rng(256), rng(256));
				floodFill(img, mask, Point(x, y), newVal, 0, colorDiff, colorDiff);
			}
		}
	}
}

#define RGB_DIS_LEVEL_SQUARE  20*20
#define MAX_ITERATE_NUM 5 
void mean_shift_seg(Mat src, Mat dst)
{
	int h = src.rows;
	int w = src.cols;

	int size = 15;
	int sx = 0, sy = 0;
	int x0 = 0, y0 = 0;
	int x1 = 0, y1 = 0;

	unsigned char C[3] = { 0 };
	unsigned char temp[3] = { 0 };

	// Filter
	for (int i = 0; i < h; i++){
		for (int j = 0; j < w; j++){

			unsigned char *center = src.data + i*w * 3 + j * 3;

			C[0] = center[0]; C[1] = center[1]; C[2] = center[2];
			sx = 0, sy = 0;
			x0 = j; y0 = i;
			for (int iter_count = 0; iter_count < MAX_ITERATE_NUM; iter_count++)
			{
				int sum_r = 0, sum_g = 0, sum_b = 0;
				int minx = (x0 - size);  minx = max(0, minx);
				int miny = (y0 - size);  miny = max(0, miny);
				int maxx = (x0 + size);  maxx = min(maxx, w - 1);
				int maxy = (y0 + size);  maxy = min(maxy, h - 1);

				int good_neighbors = 0;
				for (int k = miny; k <= maxy; k++){
					for (int q = minx; q <= maxx; q++){

						unsigned char *Nk = src.data + k*w * 3 + q * 3;
						int dis = cal_rgb_dis_square(C, Nk);
						if (dis <= RGB_DIS_LEVEL_SQUARE){
							sum_r += Nk[0]; sum_g += Nk[1]; sum_b += Nk[2];

							sx += q; sy += k; good_neighbors++;
						}
					}
				}

				if (good_neighbors == 0)
					break;

				double icount = 1. / good_neighbors;
				x1 = round(sx*icount);
				y1 = round(sy*icount);
				sum_r = round(sum_r*icount);
				sum_g = round(sum_g*icount);
				sum_b = round(sum_b*icount);

				int stop_flag;
				temp[0] = sum_r; temp[1] = sum_g; temp[2] = sum_b;
				stop_flag = (x0 == x1 && y0 == y1) || abs(x1 - x0) + abs(y1 - y0) +
					cal_rgb_dis_square(C, temp) <= 1.0f;

				x0 = x1; y0 = y1;
				C[0] = sum_r; C[1] = sum_g; C[2] = sum_b;

				if (stop_flag)
					break;
			}// iterate N times

			// mean
			unsigned char *dst_center = dst.data + i*w * 3 + j * 3;
			dst_center[0] = C[0]; dst_center[1] = C[1]; dst_center[2] = C[2];
		}
	}// for(int i = 0; i < h; i++)

	// color the result
	//floodFillPostprocess(dst,Scalar::all(2));
}