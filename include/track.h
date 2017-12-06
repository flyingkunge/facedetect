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
	vector<int> m_lostTargetID;//��ʾ��Щ����ʱ��ʧ��Ŀ��
public:
	mutiTarTrack(void);
	~mutiTarTrack(void);

	vector<STCTracker*>* mutiTarTrack::init(frameList* vidioBuf, frameList* sendListHead, vector<Rect>* faceContour, vector<int>* gender);
	void dataAssociation(Mat image, vector<Rect> faces, vector<int> gender);
	bool eyedetect(Mat& gray, Rect& faces);//���۵ļ�⣬��������
	bool skinDetect(Mat& image, int& x, int& y, int lenght);//Ŀ����ʱ��ʧʱʹ�÷�ɫ���
	//void DetectAllImage(Mat& img,vector<Rect> &faces);//�������¼�⺯��
	static DWORD WINAPI targetTrackThread(LPVOID param);
	void targetTrackThreadD();
};