#ifndef _VIDIOCAP_H_
#define _VIDIOCAP_H_

#include <iostream>
#include "Config.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include "globalAPI.h"

using namespace cv;


class haiChangCam
{
public:
	haiChangCam();
	virtual ~haiChangCam();
	void startCap();


private:
	NET_DVR_DEVICEINFO_V30 struDeviceInfo;
	LONG lUserID;
	NET_DVR_CLIENTINFO ClientInfo;
	LONG lHaiChangRealPlayHandle;


};

class vidioCap
{
public:
	vidioCap();
	void init(frameList** head, int freq);
	virtual ~vidioCap();
	static DWORD WINAPI startCap(LPVOID param);
	void vidioCap::startCapD();

	
private:
	int frameFreq;
	frameList* frameBuf;
#ifndef HKCAM_CAP
	VideoCapture* device;
#else
	haiChangCam device;
#endif
	Mat tmpData;
	LONG lHaiChangRealPlayHandle;

	void openHaiChangCam();
};




#endif //_VIDIOCAP_H_