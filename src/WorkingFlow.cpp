#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include "globalClass.h"
#include "Config.h"
#include "vidioCap.h"
#include "ThreeFrameThread.h"
#include "faceDectect.h"
#include "track.h"
#include "inetSend.h"
#include "getConfig.h"

FILE* debugFS;
frameList* faceSendList;
BYTE camSerialNO[CAM_SERIALNO_LEN];
map<string, string> sysConfig;

int main()
{
	frameList* vidioBuf;
	vector<Rect> faceContour;
	vector<int> gender;
	vector<Rect> movContour;
	int frameFre = 1;
	vidioCap m_vidioCap;
	threeFrameThread m_threeFramethread;//三帧差法线程类
	faceDectect m_faceDectect;
	mutiTarTrack mTrack;
	//MovingAearProcess m_movingAearProcess;//运动区域处理类
	//MovingAreaProcessParam m_movingAearProcessParam;//传递给运动区域处理的参数
	//ResultShow m_resultShow;//结果显示实例
	//mtt m_targetTrack;//目标跟踪类实例
	//CvCapture* cv[1];//获取的视频对象数组，每种运动区域获取方法的队列的获取对应一个cv,可能日后修改成只要一个cv

	DWORD threadIDs;
	HANDLE hVidioCapThread;
	HANDLE hThread;
	DWORD m_threadID;
	HANDLE m_playhThread;//播放线程的句柄
	HANDLE m_resultShowThread;//结果显示线程句柄
	HANDLE m_targetTrackThread;//多目标跟踪线程句柄




#ifdef DEBUG_LEVEL1
	debugFS = fopen("debug.txt", "wb");
#endif
	DEBUG_INFO(debugFS);
	readConfig("config.txt", sysConfig);
	creatFrameList(&faceSendList, JUMP_FRAME * 10);

	//开启视频读取线程
	vidioBuf = NULL;                        //vidiobuf是一个指向framelist的对象的指针
	//m_vidiocap是一个vidiocap类的对象
	m_vidioCap.init(&vidioBuf, frameFre);   //第一个参数是指向链表对象第一个framelist的指针
	                                        //指向线程函数的指针                   向线程函数传递的参数  线程指标   保存新线程的ID
	//这个线程的作用是在帧可用时对其进行抓取，转换成灰度帧并进行缩小。
	//数据的输出为   obj->frameBuf->pyDownGrayFram
	hVidioCapThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)vidioCap::startCap, &m_vidioCap, CREATE_SUSPENDED, &threadIDs);
	ResumeThread(hVidioCapThread);    //激活线程
	//m_vidioCap.startCapD();



	//printf("end\n");
	while (true)
	{
		Sleep(50000);
	}


	//if (NULL != m_threeframethread.m_cv){
	//	if (SUCCESS != GetMovingArea(&m_threeframethread)){//运动区域获取线程开启	
	//		//m_statusbar.SetPaneText(0, _T("开启运动区域线程失败"));
	//		ResetEvent(handle[13]);
	//	}
	//	//objdlg->m_statusbar.SetPaneText(0, _T("开启运动区域处理线程..."));
	//	if (SUCCESS != MovingAearProcessing(&m_movingAearProcess)){//运动区域处理线程开启		
	//		//objdlg->m_statusbar.SetPaneText(0, _T("运动区域处理线程开启失败"));
	//		ResetEvent(handle[13]);
	//		return -1;
	//}
	//else{
	//	//objdlg->m_statusbar.SetPaneText(0, _T("视频读取失败"));
	//	//AfxMessageBox(_T("视频不能加载"));
	//	ResetEvent(handle[13]);
	//	return -1;
	//}
	//
	////int a =  (int)&objwork->m_targetTrack;
	////objwork->m_resultShow.m_targetTracking = &objwork->m_targetTrack;
	//m_resultShowThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ResultShow::show, &m_resultShow, CREATE_SUSPENDED, &threadIDs);
	//m_targetTrackThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)mtt::targetTrackfun, &m_targetTrack, CREATE_SUSPENDED, &threadIDs);
	////如果初始化都没问题了，则正式启动线程开始工作
	//for (int i = 0; i<THREADS_THREEFRAME; i++){
	//	ResumeThread(m_threeframethread.m_hThread[i]);
	//}
	//for (int i = 0; i<THREADS_MOVINGAERA; i++){
	//	ResumeThread(m_movingAearProcess.m_hThread[i]);
	//}
	////开启跟踪线程与结果显示线程
	//ResumeThread(m_resultShowThread);
	//ResumeThread(m_targetTrackThread);

	////objdlg->m_statusbar.SetPaneText(0, _T("正在检测目标"));
	//ResetEvent(handle[13]);
	//return 0;
}
