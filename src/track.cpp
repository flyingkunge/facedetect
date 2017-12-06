#include "track.h"
#include "globalAPI.h"


Size recgSize(47, 55);
mutiTarTrack::mutiTarTrack(void)
{
	frameBuf = NULL;
	pFaceContours = NULL;
	pGender = NULL;
	scale = 1.4;
	if (!face_cascade.load("haarcascade_frontalface_alt.xml"))//从指定的文件目录中加载级联分类器
	{
		//cerr << "ERROR: Could not load classifier face_cascade" << endl;
		printf("跟踪模块检测器加载失败");
		//return 0;
	}
	if (!eyes_cascade.load("haarcascade_eye.xml"))//从指定的文件目录中加载级联分类器
	{
		//cerr << "ERROR: Could not load classifier face_cascade" << endl;
		printf("跟踪模块检测器加载失败");
		//return 0;
	}

}
mutiTarTrack::~mutiTarTrack(void)
{

}
vector<STCTracker*>* mutiTarTrack::init(frameList* vidioBuf, frameList* sendListHead, vector<Rect>* faceContour, vector<int>* gender)
{
	//if (!eyes_cascade.load("haarcascade_eye.xml")){ printf("eye_cascade_name加载失败\n");};
	if (!face_cascade.load("haarcascade_frontalface_alt.xml")){ printf("face_cascade_name加载失败\n"); };
	frameBuf = vidioBuf;
	sendList = sendListHead;
	pFaceContours = faceContour;
	pGender = gender;
	return &FaceTrack;
}

DWORD WINAPI mutiTarTrack::targetTrackThread(LPVOID param)
{
	unsigned int frameCount = 0;
	mutiTarTrack* obj = (mutiTarTrack*)param;
	Rect trackBox, facesPredict;
	int tarID = 0;
	Mat display;
	Mat trackFrame;
	Size minSize(MIN_FACE_SIZE, MIN_FACE_SIZE);

	time_t lt;
	lt = time(NULL);
	time_t timeSub;
	int cnt = 0;
#ifdef FACE_IMG_DIR
	int saveCnt = 0;
	char saveImgDir[1024];
#endif
	while (true)
	{
		if (obj->frameBuf->rawFrame.empty() || obj->frameBuf->flag < FACE_DECTED)
		{
			Sleep(100);
			//printf("targetTrackThread flag:%d serNum:%d\n", obj->frameBuf->flag, obj->frameBuf->serNum);
			continue;
		}
#ifndef DOWN_SAMPLED
		trackFrame = obj->frameBuf->grayFram;
#else
		trackFrame = obj->frameBuf->pyDownGrayFram;
#endif
		display = obj->frameBuf->rawFrame;
		frameCount++;
		DEBUG_INFO(debugFS);
		for (vector<STCTracker*>::iterator pTracker = obj->FaceTrack.begin(); pTracker != obj->FaceTrack.end();)
		{
    
			// 跟踪缩小的灰度帧
			(*pTracker)->tracking(trackFrame);
			//facePredict为得到的跟踪区域
			facesPredict = (*pTracker)->trackBox;
			// 如果目标在图像在边缘，则进入丢失临界区
			//tl() const {return Point_<_Tp>(x, y);
			//br() const { return Point_<_Tp>(x+width, y+height);
			if (facesPredict.tl().x < EDGE_PIX_SIZE || facesPredict.br().x >(*pTracker)->edgeSize.width ||
				facesPredict.tl().y < EDGE_PIX_SIZE || facesPredict.br().y >(*pTracker)->edgeSize.height)
			{
				(*pTracker)->edgeLost = true;
			}
			// 每16帧检测一次跟踪的人脸，确保没有发生偏移，如果发生偏移则更新参数
			//if (((frameCount & 0x0F) ^ 0x0F) == 0){
			if (frameCount % 25 == 0){
				vector<Rect> faceRect;

				//直接人脸检测算法   检测的边缘输入到faceRect
				obj->face_cascade.detectMultiScale(trackFrame((*pTracker)->cxtRegion), faceRect, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(MIN_FACE_SIZE / 2, MIN_FACE_SIZE / 2), Size(MAX_FACE_SIZE / 2, MAX_FACE_SIZE));
				imshow("check", trackFrame((*pTracker)->cxtRegion));
				if (faceRect.size() != 0)
				{
					faceRect[0].x += (*pTracker)->cxtRegion.x;
					faceRect[0].y += (*pTracker)->cxtRegion.y;
					//判断两个矩形是否相交，以及交叠部分占较大框区域的比例
					float rrt = bbOverlapofMax(faceRect[0], (*pTracker)->trackBox);
					if (0.9 > rrt)
					{
						(*pTracker)->reInit(trackFrame, faceRect[0]);
					}
					(*pTracker)->isLost = false;
					(*pTracker)->lostCount = 0;
				}
				else
				{
					(*pTracker)->lostCount++;
					if ((*pTracker)->lostCount > 0)
						(*pTracker)->isLost = true;
					cout << (*pTracker)->targetID << "is lost!!!" << endl;
				}
			}


			// 显示标定目标
			if (!(*pTracker)->isLost)
			{
#ifndef DOWN_SAMPLED

#else //DOWN_SAMPLED
				//trackBox就是其判断出来的图像的范围   其格式为Rect
				Rect pyUpTrackRect = (*pTracker)->trackBox + (*pTracker)->trackBox.tl();//返回（x,y）
				pyUpTrackRect += pyUpTrackRect.size();//返回(width, height)
				Rect pyUpCxtRegion = (*pTracker)->cxtRegion + (*pTracker)->cxtRegion.tl();
				pyUpCxtRegion += pyUpCxtRegion.size();
				if (obj->sendList->flag == USED)
				{
					if (obj->sendList->grayFram.empty())
					{
						obj->sendList->grayFram.create(recgSize, CV_8UC1);
					}
					                         //    输入              输出
					Rect faceSize = resizeRect(pyUpTrackRect, obj->frameBuf->rawFrame.size(), 2.0);//1.2

					//将数据装进sendlist准备发送   输入              输出
					obj->sendList->count = obj->frameBuf->count;
					//用faceSize来确定所截取的头像图片的大小
					resize(obj->frameBuf->rawFrame(faceSize), obj->sendList->rawFrame, recgSize);//(47,55)
					obj->sendList->serNum = (*pTracker)->targetID;
					obj->sendList->flag = UN_USED;
					obj->sendList = obj->sendList->next;
				}

				if ((*pTracker)->isRecongized){
					stringstream buf;
					buf << (*pTracker)->targetID << " " << (*pTracker)->targetName;
					string num = buf.str();
					putText(display, num, Point(pyUpCxtRegion.x, pyUpCxtRegion.y),
						FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 1);
				}
				rectangle(display, pyUpTrackRect, Scalar(255, 0, 255), 2);//显示矩形方框
				rectangle(display, pyUpCxtRegion, Scalar(0, 0, 255));
#endif //DOWN_SAMPLED

			}

			// 删除丢失的目标
			if ((((*pTracker)->isLost) && (*pTracker)->edgeLost) ||
				(*pTracker)->lostCount > 4)  //5 X 16帧，2.5秒
			{
				//cout << "delete targetID " << (*pTracker)->targetID << endl;
				cout << (*pTracker)->targetID << endl;
				STCTracker* deleteTar = *pTracker;
				obj->FaceTrack.erase(pTracker);
				free(deleteTar);
				continue;
			}
			else
				pTracker++;

		}//for 结束

		//判断是不是有新的目标出现
		if (FACE_FOUND == obj->frameBuf->flag)
		{
			// 如果新增加的人脸位置与当前跟踪的区域没有重叠，则认为有新目标出现
			for (vector<Rect>::iterator pFace = obj->frameBuf->pFaceContours.begin(); pFace != obj->frameBuf->pFaceContours.end(); pFace++)
			{
				bool newTarget = true;
				for (vector<STCTracker*>::iterator pTracker = obj->FaceTrack.begin(); pTracker != obj->FaceTrack.end(); pTracker++)
				{
					// 如果和当前跟踪区域重叠，则证明该目标已经被跟踪
					if (0.2 < bbOverlap(*pFace, (*pTracker)->trackBox))
					{
						newTarget = false;
						break;
					}
				}
				// 没有和任何一个已经跟踪的目标重叠，则该目标是新增目标
				if (!newTarget)
				{
					continue;
				}
				STCTracker* newTar = new STCTracker;
				trackBox = resizeRect(*pFace, trackFrame.size(), 1);
				newTar->init(trackFrame, trackBox);//赋予其位置值，以及其他初始值	
				newTar->targetID = tarID++;
				newTar->edgeSize = trackFrame.size() - Size(EDGE_PIX_SIZE, EDGE_PIX_SIZE);  // 设定边缘区域
				obj->FaceTrack.push_back(newTar);//入列
				printf("add a new person.\n");
				/* Mat newFace(obj->frameBuf->grayFram(resizeRect(*pFace, obj->frameBuf->grayFram.size(), 1.5)));
				faceDected.push_back(newFace);*/
			}
		
		}
		DEBUG_INFO(debugFS);
		imshow("Tracker", display);
		cnt++;
		if (cnt % 30 == 0)
		{
			timeSub = time(NULL);
			printf("FPS:%f\n", (float)cnt / (timeSub - lt));
		}
		waitKey(1);
		obj->frameBuf->flag = USED;
		obj->frameBuf = obj->frameBuf->next;
		obj->pFaceContours->clear();
	}
}


void mutiTarTrack::targetTrackThreadD()
{
	unsigned int frameCount = 0;
	Rect trackBox, facesPredict;
	int tarID = 0;
	Mat display;
	Size minSize(MIN_FACE_SIZE, MIN_FACE_SIZE);
	while (true)
	{
		if (frameBuf->rawFrame.empty() || frameBuf->flag < FACE_DECTED)
		{
			Sleep(100);
			//printf("targetTrackThread flag:%d serNum:%d\n", frameBuf->flag, frameBuf->serNum);
			continue;
		}
		display = frameBuf->rawFrame;
		frameCount++;
		DEBUG_INFO(debugFS);
		for (vector<STCTracker*>::iterator pTracker = FaceTrack.begin(); pTracker != FaceTrack.end();)
		{
			// 跟踪
			(*pTracker)->tracking(frameBuf->grayFram);

			facesPredict = (*pTracker)->trackBox;
			// 如果目标在图像在边缘，则进入丢失临界区
			if (facesPredict.tl().x < EDGE_PIX_SIZE || facesPredict.br().x >(*pTracker)->edgeSize.width ||
				facesPredict.tl().y < EDGE_PIX_SIZE || facesPredict.br().y >(*pTracker)->edgeSize.height)
			{
				(*pTracker)->edgeLost = true;
			}


			// 每16帧检测一次跟踪的人脸，确保没有发生偏移，如果发生偏移则更新参数
			if (((frameCount & 0x0F) ^ 0x0F) == 0){
				vector<Rect> faceRect;
				face_cascade.detectMultiScale(frameBuf->grayFram((*pTracker)->cxtRegion), faceRect, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(MIN_FACE_SIZE, MIN_FACE_SIZE));
				if (faceRect.size() != 0)
				{
					faceRect[0].x += (*pTracker)->cxtRegion.x;
					faceRect[0].y += (*pTracker)->cxtRegion.y;
					float rrt = bbOverlapofMax(faceRect[0], (*pTracker)->trackBox);
					if (0.9 > rrt)
					{
						(*pTracker)->reInit(frameBuf->grayFram, faceRect[0]);
					}
					(*pTracker)->isLost = false;
					(*pTracker)->lostCount = 0;
					rectangle(frameBuf->rawFrame, faceRect[0], Scalar(0, 255, 0));
				}
				else
				{
					(*pTracker)->lostCount++;
					if ((*pTracker)->lostCount > 0)
						(*pTracker)->isLost = true;
				}
			}

			// 显示标定目标
			if (!(*pTracker)->isLost)
			{
				stringstream buf;
				buf << (*pTracker)->targetID;
				string num = buf.str();
				putText(display, num, Point((*pTracker)->trackBox.x, (*pTracker)->trackBox.y - 10),
					FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
				rectangle(display, (*pTracker)->trackBox, Scalar(255, 0, 255), 2);
				rectangle(display, (*pTracker)->cxtRegion, Scalar(0, 0, 255));
			}

			// 删除丢失的目标
			if ((((*pTracker)->isLost) && (*pTracker)->edgeLost) ||
				(*pTracker)->lostCount > 1)
			{
				cout << "delete targetID " << (*pTracker)->targetID << endl;
				STCTracker* deleteTar = *pTracker._Ptr;
				FaceTrack.erase(pTracker);
				free(deleteTar);
				continue;
			}
			else
				pTracker++;

		}
		if (FACE_FOUND == frameBuf->flag)
		{
			// 如果新增加的人脸位置与当前跟踪的区域没有重叠，则认为有新目标出现
			for (vector<Rect>::iterator pFace = pFaceContours->begin(); pFace != pFaceContours->end(); pFace++)
			{
				bool newTarget = true;
				for (vector<STCTracker*>::iterator pTracker = FaceTrack.begin(); pTracker != FaceTrack.end(); pTracker++)
				{
					// 如果和当前跟踪区域重叠，则证明该目标已经被跟踪
					if (0.1 < bbOverlap(*pFace, (*pTracker)->trackBox))
					{
						newTarget = false;
						break;
					}
				}
				// 没有和任何一个已经跟踪的目标重叠，则该目标是新增目标
				if (!newTarget)
				{
					continue;
				}
				STCTracker* newTar = new STCTracker;
				trackBox = resizeRect(*pFace, frameBuf->grayFram.size(), 1);
				newTar->init(frameBuf->grayFram, trackBox);//赋予其位置值，以及其他初始值	
				newTar->targetID = tarID++;
				newTar->edgeSize = frameBuf->grayFram.size() - Size(EDGE_PIX_SIZE, EDGE_PIX_SIZE);  // 设定边缘区域
				FaceTrack.push_back(newTar);//入列
				printf("add a new person.\n");
				/* Mat newFace(frameBuf->grayFram(resizeRect(*pFace, frameBuf->grayFram.size(), 1.5)));
				faceDected.push_back(newFace);*/
			}

		}
		DEBUG_INFO(debugFS);
		imshow("Tracker", display);
		waitKey(1);
		frameBuf->flag = USED;
		frameBuf = frameBuf->next;
		pFaceContours->clear();
	}

}