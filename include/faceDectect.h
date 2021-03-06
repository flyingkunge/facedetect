#ifndef _FACEDECTECT_H_  
#define _FACEDECTECT_H_   

/***********************************************************
处理高斯、帧差等方法标注的运动区域
************************************************************/
//#include 
#include "globalAPI.h"
#include "Config.h"

class faceDectect
{
public:
	faceDectect();
	void init(frameList* head);
	virtual ~faceDectect();
	static DWORD WINAPI threadFaceDectect(LPVOID param);//人脸检测的线程函数

	int  threadFaceDectectD();
private:
	frameList* frameBuf;
	CascadeClassifier m_face_cascade, eye_Cascade;//分类器
};


#endif //_FACEDECTECT_H_