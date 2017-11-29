/***************************************************
��֡���߳��࣬��֡��õ��˶�����ȡ��ͼƬ��������
�����й���һ��������ȡ����
****************************************************/
#ifndef MY_THREEFRAMETHREAD_H_  
#define MY_THREEFRAMETHREAD_H_

#include "globalAPI.h"
#include "Config.h"
//#include "MovingAearProcess.h"
#include "cv.h"
#include "highgui.h"
#include "opencv2/opencv.hpp"



//ָʾ��ǰ����֡��ͼ�����Ӧ��˳�򣬷�������˳�����
using namespace cv;
class threeFrameThread{
public:
		
	threeFrameThread();
	virtual ~threeFrameThread();
	void init(frameList* head);

	int work();//��ʼ��������
	void jumpFrame();
	void diffFrameMorph(Mat dstFrame);
	void diffFrameCal();
	bool DiffAndFastSeedGrow();
	bool DiffAndFastSeedGrow(Mat dstFrame);
	bool drawRectMask();

	static DWORD WINAPI threeFramefun(LPVOID param);//��֡����̺߳���
	void threeFramefunD();
private:
	frameList* frameBuf;   
	frameList* pframePre;
	frameList* pframeNext;
	frameList* pframeCur;
	Mat grayPre;
	Mat grayNext;
	Mat grayCur;
	Mat threeDiffFrame;
	Mat transitFrame;

};
#endif