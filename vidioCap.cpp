#include "vidioCap.h"
#include <time.h>

extern frameList* haiCframeBuf;
extern BYTE camSerialNO[CAM_SERIALNO_LEN];
extern map<string, string> sysConfig;
vidioCap::vidioCap()
{
	//device = NULL;
	frameFreq = 0;
}
void vidioCap::init(frameList** head, int freq)
{
	creatFrameList(head, JUMP_FRAME*3);  //创建了JUMP_FRAME*3的framelist的对象，且为首尾相连
	frameBuf = *head;   //head在上一句过后指向建立的链表对象的第一个
	frameFreq = freq;
	haiCframeBuf = frameBuf;
	DEBUG_INFO(debugFS);
#ifndef HKCAM_CAP
	device = new VideoCapture(0);
#else
	device.startCap();
#endif	
}
vidioCap::~vidioCap()
{
	deleteFrameList(frameBuf);
	//device->release();
	//free(device);
}
//当帧可用时，
DWORD WINAPI vidioCap::startCap(LPVOID param){
	unsigned int intimes = 0;
	unsigned int currentframes = 0;//当前线程处理的帧
	vidioCap* obj = (vidioCap*)param;//obj为一个vidioCap类的对象
	int cnt = 0;

	while (true)
	{
		DEBUG_INFO(debugFS);
#ifndef HKCAM_CAP
		if (USED == obj->frameBuf->flag)
			*obj->device >> obj->frameBuf->rawFrame;
#else //HKCAM_CAP
		if (RAW_FRAME == obj->frameBuf->flag)
#endif //HKCAM_CAP
		{
			if (obj->frameBuf->rawFrame.empty())
				continue;
			//将帧转换为灰度帧
			cvtColor(obj->frameBuf->rawFrame, obj->frameBuf->grayFram, CV_RGB2GRAY);
			//对帧进行采样并缩小
			pyrDown(obj->frameBuf->grayFram, obj->frameBuf->pyDownGrayFram);
			//将帧标记为UN_USED，为下次循环做准备
			obj->frameBuf->flag = UN_USED;
			obj->frameBuf->pFaceContours.clear();
			obj->frameBuf->pMovContours.clear();
			//
			//printf("%d", obj->frameBuf->count);
			//将链表指针指向下一个对象
			obj->frameBuf = obj->frameBuf->next;
			//test
			//printf("%d",obj->frameBuf->count);
			//
			Sleep(10);
			continue;
		}
		Sleep(100);
	}
	
}
void vidioCap::startCapD(){
	//DWORD threadID = GetCurrentThreadId();
	printf("debug:%s,%d\n", __FUNCTIONW__, __LINE__);
	int cnt = 0;
	while (true)
	{
		if (UN_USED == frameBuf->flag)
		{
			//printf("frame cap %d\n", cnt++);
			printf("%d %p\n", cnt++, frameBuf->rawFrame.data);
			imshow("vidio Cap", frameBuf->rawFrame);
			waitKey(1);            //inshow后如果没有waitkey就不能正常的显示窗口
			frameBuf->flag = USED;

			frameBuf = frameBuf->next;
			continue;
		}
	}
	//帧率控制
	//TODO
}










haiChangCam::haiChangCam()
{
	lUserID = -1;
	lHaiChangRealPlayHandle = -1;
}
haiChangCam::~haiChangCam()
{
	//关闭预览
	if (!NET_DVR_StopRealPlay(lHaiChangRealPlayHandle))
	{
		printf("NET_DVR_StopRealPlay error! Error number: %d\n", NET_DVR_GetLastError());
		return;
	}
	//注销用户
	NET_DVR_Logout(lUserID);
	NET_DVR_Cleanup();
}
void haiChangCam::startCap() {

	//---------------------------------------
	// 初始化
	NET_DVR_Init();
	//设置连接时间与重连时间
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);

	//---------------------------------------
	// 注册设备
	
	lUserID = NET_DVR_Login_V30((char*)sysConfig.find("cammeraIP")->second.c_str(), 
								atoi(sysConfig.find("cammeraPort")->second.c_str()),
								(char*)sysConfig.find("cammeraUserName")->second.c_str(), 
								(char*)sysConfig.find("cammeraUserKey")->second.c_str(), &struDeviceInfo);
	if (lUserID < 0)
	{
		printf("Login error, %d\n", NET_DVR_GetLastError());
		NET_DVR_Cleanup();
		return;
	}
	memset(camSerialNO, 0, CAM_SERIALNO_LEN);
	memcpy(camSerialNO, struDeviceInfo.sSerialNumber, CAM_SERIALNO_LEN);
	//---------------------------------------
	//设置异常消息回调函数
	NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack, NULL);


	//cvNamedWindow("IPCamera");
	//---------------------------------------
	//启动预览并设置回调数据流 
	
	ClientInfo.lChannel = 1;        //Channel number 设备通道号
	ClientInfo.hPlayWnd = NULL;     //窗口为空，设备SDK不解码只取流
	ClientInfo.lLinkMode = 0;       //Main Stream
	ClientInfo.sMultiCastIP = NULL;

	
	lHaiChangRealPlayHandle = NET_DVR_RealPlay_V30(lUserID, &ClientInfo, fRealDataCallBack, NULL, TRUE);
	if (lHaiChangRealPlayHandle<0)
	{
		printf("NET_DVR_RealPlay_V30 failed! Error number: %d\n", NET_DVR_GetLastError());
		return;
	}
	//---------------------------------------
	//视频保存
	/*char RECNAME[256] = { 0 };
	sprintf(RECNAME,"1_2.mp4");
	if (!NET_DVR_SaveRealData(lHaiChangRealPlayHandle, RECNAME))
	{
		return;
	}*/

	return;
}
