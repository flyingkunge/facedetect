#include "track.h"
#include "globalAPI.h"


Size recgSize(47, 55);
mutiTarTrack::mutiTarTrack(void)
{
	frameBuf = NULL;
	pFaceContours = NULL;
	pGender = NULL;
	scale = 1.4;
	if (!face_cascade.load("haarcascade_frontalface_alt.xml"))//��ָ�����ļ�Ŀ¼�м��ؼ���������
	{
		//cerr << "ERROR: Could not load classifier face_cascade" << endl;
		printf("����ģ����������ʧ��");
		//return 0;
	}
	if (!eyes_cascade.load("haarcascade_eye.xml"))//��ָ�����ļ�Ŀ¼�м��ؼ���������
	{
		//cerr << "ERROR: Could not load classifier face_cascade" << endl;
		printf("����ģ����������ʧ��");
		//return 0;
	}

}
mutiTarTrack::~mutiTarTrack(void)
{

}
vector<STCTracker*>* mutiTarTrack::init(frameList* vidioBuf, frameList* sendListHead, vector<Rect>* faceContour, vector<int>* gender)
{
	//if (!eyes_cascade.load("haarcascade_eye.xml")){ printf("eye_cascade_name����ʧ��\n");};
	if (!face_cascade.load("haarcascade_frontalface_alt.xml")){ printf("face_cascade_name����ʧ��\n"); };
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
    
			// ������С�ĻҶ�֡
			(*pTracker)->tracking(trackFrame);
			//facePredictΪ�õ��ĸ�������
			facesPredict = (*pTracker)->trackBox;
			// ���Ŀ����ͼ���ڱ�Ե������붪ʧ�ٽ���
			//tl() const {return Point_<_Tp>(x, y);
			//br() const { return Point_<_Tp>(x+width, y+height);
			if (facesPredict.tl().x < EDGE_PIX_SIZE || facesPredict.br().x >(*pTracker)->edgeSize.width ||
				facesPredict.tl().y < EDGE_PIX_SIZE || facesPredict.br().y >(*pTracker)->edgeSize.height)
			{
				(*pTracker)->edgeLost = true;
			}
			// ÿ16֡���һ�θ��ٵ�������ȷ��û�з���ƫ�ƣ��������ƫ������²���
			//if (((frameCount & 0x0F) ^ 0x0F) == 0){
			if (frameCount % 25 == 0){
				vector<Rect> faceRect;

				//ֱ����������㷨   ���ı�Ե���뵽faceRect
				obj->face_cascade.detectMultiScale(trackFrame((*pTracker)->cxtRegion), faceRect, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(MIN_FACE_SIZE / 2, MIN_FACE_SIZE / 2), Size(MAX_FACE_SIZE / 2, MAX_FACE_SIZE));
				imshow("check", trackFrame((*pTracker)->cxtRegion));
				if (faceRect.size() != 0)
				{
					faceRect[0].x += (*pTracker)->cxtRegion.x;
					faceRect[0].y += (*pTracker)->cxtRegion.y;
					//�ж����������Ƿ��ཻ���Լ���������ռ�ϴ������ı���
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


			// ��ʾ�궨Ŀ��
			if (!(*pTracker)->isLost)
			{
#ifndef DOWN_SAMPLED

#else //DOWN_SAMPLED
				//trackBox�������жϳ�����ͼ��ķ�Χ   ���ʽΪRect
				Rect pyUpTrackRect = (*pTracker)->trackBox + (*pTracker)->trackBox.tl();//���أ�x,y��
				pyUpTrackRect += pyUpTrackRect.size();//����(width, height)
				Rect pyUpCxtRegion = (*pTracker)->cxtRegion + (*pTracker)->cxtRegion.tl();
				pyUpCxtRegion += pyUpCxtRegion.size();
				if (obj->sendList->flag == USED)
				{
					if (obj->sendList->grayFram.empty())
					{
						obj->sendList->grayFram.create(recgSize, CV_8UC1);
					}
					                         //    ����              ���
					Rect faceSize = resizeRect(pyUpTrackRect, obj->frameBuf->rawFrame.size(), 2.0);//1.2

					//������װ��sendlist׼������   ����              ���
					obj->sendList->count = obj->frameBuf->count;
					//��faceSize��ȷ������ȡ��ͷ��ͼƬ�Ĵ�С
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
				rectangle(display, pyUpTrackRect, Scalar(255, 0, 255), 2);//��ʾ���η���
				rectangle(display, pyUpCxtRegion, Scalar(0, 0, 255));
#endif //DOWN_SAMPLED

			}

			// ɾ����ʧ��Ŀ��
			if ((((*pTracker)->isLost) && (*pTracker)->edgeLost) ||
				(*pTracker)->lostCount > 4)  //5 X 16֡��2.5��
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

		}//for ����

		//�ж��ǲ������µ�Ŀ�����
		if (FACE_FOUND == obj->frameBuf->flag)
		{
			// ��������ӵ�����λ���뵱ǰ���ٵ�����û���ص�������Ϊ����Ŀ�����
			for (vector<Rect>::iterator pFace = obj->frameBuf->pFaceContours.begin(); pFace != obj->frameBuf->pFaceContours.end(); pFace++)
			{
				bool newTarget = true;
				for (vector<STCTracker*>::iterator pTracker = obj->FaceTrack.begin(); pTracker != obj->FaceTrack.end(); pTracker++)
				{
					// ����͵�ǰ���������ص�����֤����Ŀ���Ѿ�������
					if (0.2 < bbOverlap(*pFace, (*pTracker)->trackBox))
					{
						newTarget = false;
						break;
					}
				}
				// û�к��κ�һ���Ѿ����ٵ�Ŀ���ص������Ŀ��������Ŀ��
				if (!newTarget)
				{
					continue;
				}
				STCTracker* newTar = new STCTracker;
				trackBox = resizeRect(*pFace, trackFrame.size(), 1);
				newTar->init(trackFrame, trackBox);//������λ��ֵ���Լ�������ʼֵ	
				newTar->targetID = tarID++;
				newTar->edgeSize = trackFrame.size() - Size(EDGE_PIX_SIZE, EDGE_PIX_SIZE);  // �趨��Ե����
				obj->FaceTrack.push_back(newTar);//����
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
			// ����
			(*pTracker)->tracking(frameBuf->grayFram);

			facesPredict = (*pTracker)->trackBox;
			// ���Ŀ����ͼ���ڱ�Ե������붪ʧ�ٽ���
			if (facesPredict.tl().x < EDGE_PIX_SIZE || facesPredict.br().x >(*pTracker)->edgeSize.width ||
				facesPredict.tl().y < EDGE_PIX_SIZE || facesPredict.br().y >(*pTracker)->edgeSize.height)
			{
				(*pTracker)->edgeLost = true;
			}


			// ÿ16֡���һ�θ��ٵ�������ȷ��û�з���ƫ�ƣ��������ƫ������²���
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

			// ��ʾ�궨Ŀ��
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

			// ɾ����ʧ��Ŀ��
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
			// ��������ӵ�����λ���뵱ǰ���ٵ�����û���ص�������Ϊ����Ŀ�����
			for (vector<Rect>::iterator pFace = pFaceContours->begin(); pFace != pFaceContours->end(); pFace++)
			{
				bool newTarget = true;
				for (vector<STCTracker*>::iterator pTracker = FaceTrack.begin(); pTracker != FaceTrack.end(); pTracker++)
				{
					// ����͵�ǰ���������ص�����֤����Ŀ���Ѿ�������
					if (0.1 < bbOverlap(*pFace, (*pTracker)->trackBox))
					{
						newTarget = false;
						break;
					}
				}
				// û�к��κ�һ���Ѿ����ٵ�Ŀ���ص������Ŀ��������Ŀ��
				if (!newTarget)
				{
					continue;
				}
				STCTracker* newTar = new STCTracker;
				trackBox = resizeRect(*pFace, frameBuf->grayFram.size(), 1);
				newTar->init(frameBuf->grayFram, trackBox);//������λ��ֵ���Լ�������ʼֵ	
				newTar->targetID = tarID++;
				newTar->edgeSize = frameBuf->grayFram.size() - Size(EDGE_PIX_SIZE, EDGE_PIX_SIZE);  // �趨��Ե����
				FaceTrack.push_back(newTar);//����
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