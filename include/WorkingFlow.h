/*******************************************************
工作调度类，这是总的任务调度控制中心
********************************************************/
#include "ThreeFrameThread.h"
#include "cv.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include "globalClass.h"
#include "Config.h"
class CReconiseAnalyzationSystemDlg;//声明CReconiseAnalyzationSystemDlg类，需要用到，而不能直接包含，因为会出现相互包含的情况
using namespace cv;
#ifndef MY_WORKINGFLOW_H_  
#define MY_WORKINGFLOW_H_   

/*****************工作流类**********************/
//class WorkingFlow{
//public:
//	WorkingFlow();
//	//WorkingFlow(VideoCapture cv[]);
//	virtual ~WorkingFlow();
//	CReconiseAnalyzationSystemDlg* m_dlg;//声明的是主界面的指针
//	threeFrameThread m_threeframethread;//三帧差法线程类
//	MovingAearProcess m_movingAearProcess;//运动区域处理类
//	MovingAreaProcessParam m_movingAearProcessParam;//传递给运动区域处理的参数
//	ResultShow m_resultShow;//结果显示实例
//	mtt m_targetTrack;//目标跟踪类实例
//	Gausstrain m_gauss;//高斯训练类
//	//GaussParam m_gaussParam;//高斯训练的全局参数
//	CvCapture* cv[1];//获取的视频对象数组，每种运动区域获取方法的队列的获取对应一个cv,可能日后修改成只要一个cv
//	//VideoCapture* cv;//获取的视频对象数组，每种运动区域获取方法的队列的获取对应一个cv,可能日后修改成只要一个cv
//
//	//void initialize(VideoCapture* cv,SDisplayObject* display);//做所有的初始化工作，包括从外部传进来的初始化数
//	void videoInitialize();//视频处理初始化
//	int GetMovingArea();//获取运动区域的工作流开启
//	int MovingAearProcessing();//处理得到的运动区域
//
//	HANDLE m_hThread;
//	DWORD m_threadID;
//	HANDLE m_playhThread;//播放线程的句柄
//	HANDLE m_resultShowThread;//结果显示线程句柄
//	HANDLE m_targetTrackThread;//多目标跟踪线程句柄
//	static DWORD WINAPI workListenProcessfun(LPVOID param);//监听所有事件的线程
//	//static DWORD WINAPI workListenProcessfun2(LPVOID param);//监听所有事件的线程,缓解另外一个线程的压力，做一些简单的界面响应
//	//static DWORD WINAPI videoStartProcessfun(LPVOID param);//点击开始时启动初始化视频的线程
//	static DWORD WINAPI gaussTrain(LPVOID param);//高斯训练的线程
//	static DWORD WINAPI play(LPVOID param);//纯粹的播放函数
//	static DWORD WINAPI headfocus(LPVOID param);//人头大小对准
//	static DWORD WINAPI imageRegernise(LPVOID param);//图像识别线程
//};
#endif