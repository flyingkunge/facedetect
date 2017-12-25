#pragma once

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

class STCTracker
{
public:
	STCTracker();
	~STCTracker();
	void init(const Mat frame, const Rect box);
	void reInit(const Mat frame, const Rect box);
	void tracking(const Mat frame);

private:
	void createHammingWin();
	void complexOperation(const Mat src1, const Mat src2, Mat &dst, int flag = 0);
	void getCxtPriorPosteriorModel(const Mat image);
	void learnSTCModel(const Mat image);

public:
	Rect trackBox;
	Rect cxtRegion;			// context region
	bool isLost;
	bool edgeLost;			// target is in the edge of image
	int  targetID;
	char targetName[256];
	bool isRecongized;
	unsigned int lostCount;
	Size edgeSize;

private:
	double sigma;			// scale parameter (variance)
	double alpha;			// scale parameter
	double beta;			// shape parameter
	double rho;				// learning parameter
	Point center;			// the object position


	Mat cxtPriorPro;		// prior probability
	Mat cxtPosteriorPro;	// posterior probability
	Mat STModel;			// conditional probability
	Mat STCModel;			// spatio-temporal context model
	Mat hammingWin;			// Hamming window
};