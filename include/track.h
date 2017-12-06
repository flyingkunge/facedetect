#pragma once

#include <cv.h>
#include <highgui.h>
#include "Config.h"
#include "globalAPI.h"
#include "STCTracker.h"

using namespace std;
using namespace cv;
#define MEANNUM 10

class mutiTarTrack
{
public:
	vector<STCTracker*> FaceTrack;
	frameList* frameBuf;
	frameList* sendList;
	vector<Rect>* pMovContours;
	vector<Rect>* pFaceContours;
	vector<int>* pGender;

	float scale;
	CascadeClassifier eyes_cascade;
	CascadeClassifier face_cascade;
	vector<int> m_lostTargetID;//表示哪些是暂时丢失的目标
public:
	mutiTarTrack(void);
	~mutiTarTrack(void);

	vector<STCTracker*>* mutiTarTrack::init(frameList* vidioBuf, frameList* sendListHead, vector<Rect>* faceContour, vector<int>* gender);
	void dataAssociation(Mat image, vector<Rect> faces, vector<int> gender);
	bool eyedetect(Mat& gray, Rect& faces);//人眼的检测，对齐脸部
	bool skinDetect(Mat& image, int& x, int& y, int lenght);//目标暂时丢失时使用肤色检测
	//void DetectAllImage(Mat& img,vector<Rect> &faces);//人脸重新检测函数
	static DWORD WINAPI targetTrackThread(LPVOID param);
	void targetTrackThreadD();
};