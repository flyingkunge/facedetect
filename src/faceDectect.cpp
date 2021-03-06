#include "faceDectect.h"
faceDectect::faceDectect(){

}
void faceDectect::init(frameList* head)
{
	frameBuf = head;
	//加载识别器
	if (!m_face_cascade.load("haarcascade_frontalface_alt.xml")){
		return;
	}
	if (!eye_Cascade.load("haarcascade_eye.xml")){
		return;
	}
}
faceDectect::~faceDectect()
{
	//TODO
}

DWORD WINAPI faceDectect::threadFaceDectect(LPVOID param)
{
	faceDectect* obj = (faceDectect*)param;
	vector<Rect> facesTmp;
	vector<int> genderTmp;
	vector<Rect> contoursTmp;
	Rect faceRect, oriFaceRect;
	Size faceSize;

	Size minSize(MIN_FACE_SIZE, MIN_FACE_SIZE);
	while (true)
	{
		if (obj->frameBuf->rawFrame.empty() || (obj->frameBuf->flag != THREE_DIFF && obj->frameBuf->flag != FRAME_DIFF))
		{
			Sleep(100);
			//printf("threadFaceDectect flag:%d\n", obj->frameBuf->flag);
			continue;
		}
		DEBUG_INFO(debugFS);
		//cvtColor(obj->frameBuf->rawFrame, grayFrame, CV_BGR2GRAY);
		//grayFrame = obj->frameBuf->grayFram;
		//equalizeHist(grayFrame, grayFrame);
		facesTmp.clear();
		DEBUG_INFO(debugFS);
		if (THREE_DIFF == obj->frameBuf->flag)
		{
			// 搜索新的运动区域是否有人脸
			for (int i = 0; i < obj->frameBuf->pMovContours.size(); i++)                  //.size()找到元素的个数
			{
				vector <Rect> skinCont;
				Mat  grayFrame;
#ifndef DOWN_SAMPLED
				Rect skinResize = resizeRect((obj->frameBuf->pMovContours)[i], obj->frameBuf->rawFrame.size(), 1.5);
				cvSkinYUV2Gray(obj->frameBuf->rawFrame(skinResize), grayFrame);
				getContours(grayFrame, minSize, &skinCont, Point(skinResize.x, skinResize.y));
				for (int k = 0; k < skinCont.size(); k++)
				{
					//eyes_cascade.detectMultiScale(faceFrame, eyes, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));
					obj->m_face_cascade.detectMultiScale(obj->frameBuf->grayFram(skinCont[k]), contoursTmp, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));
					for (int j = 0; j < contoursTmp.size(); j++)
					{
						//faceRect = contoursTmp[j];
						faceRect.x = contoursTmp[j].x + skinCont[k].x;
						faceRect.y = contoursTmp[j].y + skinCont[k].y;
						faceRect.width = contoursTmp[j].width;
						faceRect.height = contoursTmp[j].height;
						facesTmp.push_back(faceRect);
						genderTmp.push_back(1);
					}
				}
#else
				obj->m_face_cascade.detectMultiScale(obj->frameBuf->pyDownGrayFram((obj->frameBuf->pMovContours)[i]), contoursTmp, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(MIN_FACE_SIZE / 2, MIN_FACE_SIZE / 2), Size(MAX_FACE_SIZE / 2, MAX_FACE_SIZE / 2));
				for (int j = 0; j < contoursTmp.size(); j++)
				{
					//faceRect = contoursTmp[j];
					faceRect.x = contoursTmp[j].x + (obj->frameBuf->pMovContours)[i].x;
					faceRect.y = contoursTmp[j].y + (obj->frameBuf->pMovContours)[i].y;
					faceRect.width = contoursTmp[j].width;
					faceRect.height = contoursTmp[j].height;

					//rectangle(obj->frameBuf->pyDownGrayFram, faceRect, Scalar(255, 0, 0), 2);
					facesTmp.push_back(faceRect);
				}
#endif
			}
		}
		DEBUG_INFO(debugFS);
		//obj->pFaceContours->clear();
		//obj->pGender->clear();

		//imshow("down", obj->frameBuf->pyDownGrayFram);
		//waitKey(1);

		obj->frameBuf->pFaceContours.assign(facesTmp.begin(), facesTmp.end());//assign将区间内的值赋值到当前的容器中
		DEBUG_SHOW_IMG_RECT(obj->frameBuf->rawFrame, *(obj->frameBuf->pFaceContours), "face decteted");
		//obj->pGender->assign(genderTmp.begin(), genderTmp.end());
		if (obj->frameBuf->pFaceContours.size() > 0)
			obj->frameBuf->flag = FACE_FOUND;
		else
			obj->frameBuf->flag = FACE_DECTED;
		obj->frameBuf = obj->frameBuf->next;
		DEBUG_INFO(debugFS);
	}
	return SUCCESS;
}

int  faceDectect::threadFaceDectectD()
{
	vector<Rect> facesTmp;
	vector<Rect> contoursTmp;
	Rect faceRect, oriFaceRect;
	Size faceSize;
	Mat  grayFrame;
	Size minSize(MIN_FACE_SIZE, MIN_FACE_SIZE);
	while (true)
	{
		if (frameBuf->rawFrame.empty() || frameBuf->flag < FRAME_DIFF)
		{
			Sleep(100);
			continue;
		}
		DEBUG_INFO(debugFS);
		//cvtColor(frameBuf->rawFrame, grayFrame, CV_BGR2GRAY);
		grayFrame = frameBuf->grayFram;
		equalizeHist(grayFrame, grayFrame);
		facesTmp.clear();
		DEBUG_INFO(debugFS);
		Mat display = frameBuf->rawFrame;
		if (THREE_DIFF == frameBuf->flag)
		{
			// 搜索新的运动区域是否有人脸
			for (int i = 0; i < frameBuf->pMovContours.size(); i++)
			{
				vector <Rect> skinCont;
				Mat  grayFrame;
				Rect skinResize = resizeRect((frameBuf->pMovContours)[i], frameBuf->rawFrame.size(), 2);
				cvSkinYUV2Gray(frameBuf->rawFrame(skinResize), grayFrame);
				getContours(grayFrame, minSize, &skinCont, Point(skinResize.x, skinResize.y));
				for (int k = 0; k < skinCont.size(); k++)
				{
					//eyes_cascade.detectMultiScale(faceFrame, eyes, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));
					m_face_cascade.detectMultiScale(frameBuf->grayFram(skinCont[k]), contoursTmp, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));
					for (int j = 0; j < contoursTmp.size(); j++)
					{
						//faceRect = contoursTmp[j];
						faceRect.x = contoursTmp[j].x + skinCont[k].x;
						faceRect.y = contoursTmp[j].y + skinCont[k].y;
						faceRect.width = contoursTmp[j].width;
						faceRect.height = contoursTmp[j].height;
						facesTmp.push_back(faceRect);
					}
				}
			}
		}
		DEBUG_INFO(debugFS);
		frameBuf->pFaceContours.clear();
		frameBuf->pFaceContours.assign(facesTmp.begin(), facesTmp.end());
		if (frameBuf->pFaceContours.size() > 0)
			frameBuf->flag = FACE_FOUND;
		else
			frameBuf->flag = FACE_DECTED;
		DEBUG_SHOW_IMG_RECT(frameBuf->rawFrame, frameBuf->pFaceContours, "face decteted");
		frameBuf->flag = USED;
		frameBuf = frameBuf->next;
		DEBUG_INFO(debugFS);
	}
	return SUCCESS;
}