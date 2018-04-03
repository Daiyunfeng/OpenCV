#ifndef MEAN_SHIFT_H
#define MEAN_SHIFT_H

#include "opencv2/opencv.hpp"

using namespace cv;

class MeanShiftTracker
{
public:
	MeanShiftTracker();
	//~MeanShiftTracker();
public:
	Rect MeanShift(Mat &probImg, Rect track_window);


	//private:
	float kernel_value(int value, int pos, float sigma);
	float scotts_factor(int N, int dims);
	void caculate_variance(Mat &roi, int offset_x, int offset_y);
	void weighted_sample_variance(Mat &roi, int offset_x, int offset_y);

	//private:
	float h; // bandwidth
	float sigma_x; // variance
	float sigma_y; // variance
	bool sigma_called;

	size_t non_zero_points;

};
#endif