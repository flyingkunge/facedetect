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
	creatFrameList(head, JUMP_FRAME*3);  //������JUMP_FRAME*3��framelist�Ķ�����Ϊ��β����
	frameBuf = *head;   //head����һ�����ָ�������������ĵ�һ��
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
//��֡����ʱ��
DWORD WINAPI vidioCap::startCap(LPVOID param){
	unsigned int intimes = 0;
	unsigned int currentframes = 0;//��ǰ�̴߳����֡
	vidioCap* obj = (vidioCap*)param;//objΪһ��vidioCap��Ķ���
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
			//��֡ת��Ϊ�Ҷ�֡
			cvtColor(obj->frameBuf->rawFrame, obj->frameBuf->grayFram, CV_RGB2GRAY);
			//��֡���в�������С
			pyrDown(obj->frameBuf->grayFram, obj->frameBuf->pyDownGrayFram);
			//��֡���ΪUN_USED��Ϊ�´�ѭ����׼��
			obj->frameBuf->flag = UN_USED;
			obj->frameBuf->pFaceContours.clear();
			obj->frameBuf->pMovContours.clear();
			//
			//printf("%d", obj->frameBuf->count);
			//������ָ��ָ����һ������
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
			waitKey(1);            //inshow�����û��waitkey�Ͳ�����������ʾ����
			frameBuf->flag = USED;

			frameBuf = frameBuf->next;
			continue;
		}
	}
	//֡�ʿ���
	//TODO
}










haiChangCam::haiChangCam()
{
	lUserID = -1;
	lHaiChangRealPlayHandle = -1;
}
haiChangCam::~haiChangCam()
{
	//�ر�Ԥ��
	if (!NET_DVR_StopRealPlay(lHaiChangRealPlayHandle))
	{
		printf("NET_DVR_StopRealPlay error! Error number: %d\n", NET_DVR_GetLastError());
		return;
	}
	//ע���û�
	NET_DVR_Logout(lUserID);
	NET_DVR_Cleanup();
}
void haiChangCam::startCap() {

	//---------------------------------------
	// ��ʼ��
	NET_DVR_Init();
	//��������ʱ��������ʱ��
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);

	//---------------------------------------
	// ע���豸
	
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
	//�����쳣��Ϣ�ص�����
	NET_DVR_SetExceptionCallBack_V30(0, NULL, g_ExceptionCallBack, NULL);


	//cvNamedWindow("IPCamera");
	//---------------------------------------
	//����Ԥ�������ûص������� 
	
	ClientInfo.lChannel = 1;        //Channel number �豸ͨ����
	ClientInfo.hPlayWnd = NULL;     //����Ϊ�գ��豸SDK������ֻȡ��
	ClientInfo.lLinkMode = 0;       //Main Stream
	ClientInfo.sMultiCastIP = NULL;

	
	lHaiChangRealPlayHandle = NET_DVR_RealPlay_V30(lUserID, &ClientInfo, fRealDataCallBack, NULL, TRUE);
	if (lHaiChangRealPlayHandle<0)
	{
		printf("NET_DVR_RealPlay_V30 failed! Error number: %d\n", NET_DVR_GetLastError());
		return;
	}
	//---------------------------------------
	//��Ƶ����
	/*char RECNAME[256] = { 0 };
	sprintf(RECNAME,"1_2.mp4");
	if (!NET_DVR_SaveRealData(lHaiChangRealPlayHandle, RECNAME))
	{
		return;
	}*/

	return;
}
