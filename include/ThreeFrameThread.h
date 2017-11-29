/***************************************************
三帧差线程类，三帧差法得到运动区域，取得图片并保存在
队列中供下一工作流读取队列
****************************************************/
#ifndef MY_THREEFRAMETHREAD_H_  
#define MY_THREEFRAMETHREAD_H_

#include "globalAPI.h"
#include "Config.h"
//#include "MovingAearProcess.h"
#include "cv.h"
#include "highgui.h"
#include "opencv2/opencv.hpp"



//指示当前的三帧差图与其对应的顺序，否则插入的顺序会乱
using namespace cv;
class threeFrameThread{
public:
		
	threeFrameThread();
	virtual ~threeFrameThread();
	void init(frameList* head);

	int work();//开始创建进程
	void jumpFrame();
	void diffFrameMorph(Mat dstFrame);
	void diffFrameCal();
	bool DiffAndFastSeedGrow();
	bool DiffAndFastSeedGrow(Mat dstFrame);
	bool drawRectMask();

	static DWORD WINAPI threeFramefun(LPVOID param);//三帧差法的线程函数
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