#include "threeFrameThread.h"
threeFrameThread::threeFrameThread(){
	frameBuf = NULL;
}
threeFrameThread::~threeFrameThread(){
	frameBuf = NULL;
}
void threeFrameThread::init(frameList* head)   //     ��ʼ��
{
	int cnt = 0;
	int col, row;
	frameBuf = head;
	pframeNext = frameBuf;
	//ѭ���ȴ�������Ƶ���ݣ���ȴ�3s
	while (pframeNext->rawFrame.empty() && cnt < 30)
	{
		cnt++;
		Sleep(100);
	}
	if (pframeNext->rawFrame.empty())
	{
		//TODO init failed       ��ʼ��ʧ��
		return;
	}
	else
	{
		col = (int)floor(pframeNext->rawFrame.cols*0.5);     //�ṹ��ָ����ʵ�ʹ�����һ���ṹ�����     ����col���е��м�ֵ
		row = (int)floor(pframeNext->rawFrame.rows*0.5);     //cols ��rows��mat���Դ�����                   row���е��м�ֵ
		transitFrame.create(row, col, CV_8UC3);              //����һ��1/2��С��mat
		grayNext.create(row, col, CV_BGR2GRAY);              //�����˺ܶ��1/2ԭʼ��С��mat
		grayCur.create(row, col, CV_BGR2GRAY);
		grayPre.create(row, col, CV_BGR2GRAY);
		threeDiffFrame.create(row, col, CV_BGR2GRAY);
	}
}
void threeFrameThread::jumpFrame()    //  ȷ����ǰ֡��֮ǰ֡��֮��֮֡��Ĺ�ϵ���������㷨���
{
	int frameCnt = 0;                                        //     ��־λ
	int midCnt = (int)floor((double)JUMP_FRAME / 2);         //     �м�ֵ
	pframeNext = frameBuf;                                   //     vidiobuf��framelist����
	while (frameCnt < JUMP_FRAME)
	{
		if (pframeNext->flag == UN_USED && !pframeNext->rawFrame.empty())
		{
			if (0 == frameCnt) 
			{
				pframePre = pframeNext;
				grayPre = pframePre->pyDownGrayFram;
			}
			if (midCnt == frameCnt)
			{
				pframeCur = pframeNext;     
				grayCur = pframeCur->pyDownGrayFram;           //pframeCur��framelist��һ���ṹ�����
			}
			if (JUMP_FRAME-1 == frameCnt)
			{
				grayNext = pframeNext->pyDownGrayFram;
				break;
			}
			pframeNext = pframeNext->next;
			frameCnt++;
			continue;
		}
		else{
			Sleep(100);
		}
		
	}
	
}
//��ͼƬ�������ʹ���
void threeFrameThread::diffFrameMorph(Mat dstFrame)   
{
	if (dstFrame.empty())
		return;
	Point p1(1,1);
	Size s1(3,3);
	//! returns structuring element of the specified shape and size
	//���ؽṹԪ�ص���������size
	Mat element = getStructuringElement(CV_SHAPE_RECT, s1, p1);
	//���ͺ���   ��ͼ���еĶ���߽��������  ʹ�߽��������
	dilate(dstFrame, dstFrame, element, p1);
	element.release();
}

bool threeFrameThread::DiffAndFastSeedGrow(Mat dstFrame)
{
	Point p1(1, 1);
	Size s1(3, 3);
	//! returns structuring element of the specified shape and size
	Mat element = getStructuringElement(CV_SHAPE_RECT, s1, p1);
	dilate(dstFrame, dstFrame, element, p1);
	if (dstFrame.empty())
		return false;
	float lowThresh = 7;
	float upThresh = 25;
	//�ϱߵõ�����������������ֵ
	//! applies fixed threshold to the image
	threshold(dstFrame, dstFrame, lowThresh, 255, CV_THRESH_TOZERO);//�ѵ���С��ֵ�ĵ�����Ϊ0
	threshold(dstFrame, dstFrame, upThresh, 255, CV_THRESH_TRUNC);//�Ѹ��ڴ���ֵ�ĵ�����Ϊ����ֵ
	return true;
}


void threeFrameThread::diffFrameCal()
{
	Mat formDiffFrameA;
	Mat formDiffFrameB;
	formDiffFrameA.create(grayNext.size(), CV_BGR2GRAY);
	formDiffFrameB.create(grayNext.size(), CV_BGR2GRAY);
	//! computes element-wise absolute difference of two arrays (dst = abs(src1 - src2))
	//�Ƚ�����Ԫ�صľ��Բ�  formDiffFrameB=abs��grayPre-grayCur��
	absdiff(grayPre, grayCur, formDiffFrameB);
	absdiff(grayNext, grayCur, formDiffFrameA);
	//�������ʹ���ʹ��Ե��������
	diffFrameMorph(formDiffFrameB);
	diffFrameMorph(formDiffFrameA);
	//! computes per-element minimum of two arrays (dst = min(src1, src2))
	//�Ƚ�����������С���Ǹ�
	cv::min(formDiffFrameB, formDiffFrameA, transitFrame);
	DiffAndFastSeedGrow(transitFrame);
}

//����Ĥ��ȷ���˶��ķ���
bool threeFrameThread::drawRectMask(){
	int flag = 0, flag1 = 1;//����仯��־
	flag = flag1;
	Rect movRect;
	Size movSize;
	Point startRect;
	vector< vector<Point> >  contours;//��ʼ����
	vector<Vec4i> hierarchy;//��������֮���ϵ������
	//! retrieves contours and the hierarchical information from black-n-white image.
	//�ӻҶ�ͼƬ�еõ�������ֲ����Ϣ
	//�ڶ����������������ĵ�ֵ
	findContours(transitFrame, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	for (size_t i = 0; i < contours.size(); i++)
	{
		//���������ķ���
		movRect = boundingRect(Mat(contours[i]));
		//ȷ���õ������������������������Χ��
		if (movRect.width < MIN_FACE_SIZE || movRect.height < MIN_FACE_SIZE)
		{
			continue;
		}
#ifndef DOWN_SAMPLED
		startRect = movRect.tl();
		movSize.width = movRect.width;
		movSize.height = movRect.height;
		movRect += startRect;
		movRect += movSize;
#endif
		//push_back��vector��������Ϊ��vectorβ������һ������
		frameBuf->pMovContours.push_back(movRect);
	}
#if 1 //DEBUG_SHOW
	//frameBuf->framDiff = rectImg;
	
#endif
	return true;
}

/******************��ȡ�˶������߳�**************/
DWORD WINAPI threeFrameThread::threeFramefun(LPVOID param)
{
	unsigned int intimes = 0;
	unsigned int currentframes = 0;//��ǰ�̴߳����֡
	threeFrameThread* obj = (threeFrameThread*)param;  //��������
	unsigned int frameCnt;
	int midCnt = (int)floor((double)JUMP_FRAME / 2);
	obj->jumpFrame();
	frameCnt = 1;
	while(true){
		if (UN_USED != obj->frameBuf->flag)
		{
			Sleep(100);
			continue;
		}
		DEBUG_INFO(debugFS);
		if (frameCnt == JUMP_FRAME)
		{
			DEBUG_INFO(debugFS);
			obj->jumpFrame();
			obj->diffFrameCal();
			obj->drawRectMask();
			obj->frameBuf->flag = THREE_DIFF;
			obj->frameBuf = obj->frameBuf->next;
			frameCnt = 1;
			continue;
		}
		else
		{
			DEBUG_INFO(debugFS);
			//obj->frameBuf->flag = USED;
			obj->frameBuf->flag = FRAME_DIFF;
			obj->frameBuf = obj->frameBuf->next;
			frameCnt++;
			continue;
			//TODO
		}
		DEBUG_INFO(debugFS);
	}
	return 0;
}

#if 0
/******************��ȡ�˶������߳�**************/
void threeFrameThread::threeFramefunD()
{
	unsigned int intimes = 0;
	unsigned int currentframes = 0;//��ǰ�̴߳����֡
	unsigned int frameCnt;
	int midCnt = (int)floor((double)JUMP_FRAME / 2);
	jumpFrame();
	frameCnt = 1;
	while (true){
		if (frameBuf->flag == USED)
		{
			Sleep(100);
			continue;
		}
		if (frameCnt%midCnt == 0)
		{
			jumpFrame();
			diffFrameCal();
			drawRectMask();
			if (!threeDiffFrame.empty())
				imshow("three frame diff", threeDiffFrame);
			printf("Mov contours:%d\n", boundRect->size());
			DEBUG_SHOW_IMG(frameBuf->rawFrame, "vidio cap");
			frameBuf->flag = USED;
			frameBuf = frameBuf->next;
			waitKey(5);
		}
		else
		{
			if (!threeDiffFrame.empty())
				imshow("three frame diff", threeDiffFrame);
			DEBUG_SHOW_IMG(frameBuf->rawFrame, "vidio cap");
			frameBuf->flag = USED;
			frameBuf = frameBuf->next;
			waitKey(5);
			//TODO
		}
		frameCnt++;
	}
}

#else

/******************��ȡ�˶������߳�**************/
void threeFrameThread::threeFramefunD()
{
	unsigned int intimes = 0;
	unsigned int currentframes = 0;//��ǰ�̴߳����֡
	unsigned int frameCnt;
	int midCnt = (int)floor((double)JUMP_FRAME / 2);
	jumpFrame();
	frameCnt = 1;
	while (true){
		if (frameBuf->flag == USED)
		{
			Sleep(100);
			continue;
		}
		if (frameCnt == JUMP_FRAME)
		{
			jumpFrame();
			diffFrameCal();
			drawRectMask();
			if (!threeDiffFrame.empty()){
				imshow("three frame diff", threeDiffFrame);
				imshow("capture", frameBuf->rawFrame);
			}
				
			//DEBUG_SHOW_IMG(frameBuf->rawFrame, "vidio cap");
			printf("Mov contours:%d\n", frameBuf->pFaceContours.size());
			frameBuf->flag = USED;
			frameBuf = frameBuf->next;
			frameCnt = 1;
			waitKey(5);
			continue;
		}
		else
		{
			if (!threeDiffFrame.empty()){
				imshow("three frame diff", threeDiffFrame);
				imshow("capture", frameBuf->rawFrame);
			}
			//DEBUG_SHOW_IMG(frameBuf->rawFrame, "vidio cap");
			frameBuf->flag = USED;
			frameBuf = frameBuf->next;
			waitKey(5);

			frameCnt++;
			continue;
			//TODO
		}
	}
}

#endif