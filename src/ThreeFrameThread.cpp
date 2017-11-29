#include "threeFrameThread.h"
threeFrameThread::threeFrameThread(){
	frameBuf = NULL;
}
threeFrameThread::~threeFrameThread(){
	frameBuf = NULL;
}
void threeFrameThread::init(frameList* head)   //     初始化
{
	int cnt = 0;
	int col, row;
	frameBuf = head;
	pframeNext = frameBuf;
	//循环等待读入视频数据，最长等待3s
	while (pframeNext->rawFrame.empty() && cnt < 30)
	{
		cnt++;
		Sleep(100);
	}
	if (pframeNext->rawFrame.empty())
	{
		//TODO init failed       初始化失败
		return;
	}
	else
	{
		col = (int)floor(pframeNext->rawFrame.cols*0.5);     //结构体指针其实就代表了一个结构体变量     所以col是列的中间值
		row = (int)floor(pframeNext->rawFrame.rows*0.5);     //cols 与rows是mat的自带函数                   row是行的中间值
		transitFrame.create(row, col, CV_8UC3);              //构造一个1/2大小的mat
		grayNext.create(row, col, CV_BGR2GRAY);              //构造了很多个1/2原始大小的mat
		grayCur.create(row, col, CV_BGR2GRAY);
		grayPre.create(row, col, CV_BGR2GRAY);
		threeDiffFrame.create(row, col, CV_BGR2GRAY);
	}
}
void threeFrameThread::jumpFrame()    //  确定当前帧，之前帧与之后帧之间的关系，与后面的算法相关
{
	int frameCnt = 0;                                        //     标志位
	int midCnt = (int)floor((double)JUMP_FRAME / 2);         //     中间值
	pframeNext = frameBuf;                                   //     vidiobuf的framelist变量
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
				grayCur = pframeCur->pyDownGrayFram;           //pframeCur是framelist的一个结构体变量
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
//对图片进行膨胀处理
void threeFrameThread::diffFrameMorph(Mat dstFrame)   
{
	if (dstFrame.empty())
		return;
	Point p1(1,1);
	Size s1(3,3);
	//! returns structuring element of the specified shape and size
	//返回结构元素的类型与其size
	Mat element = getStructuringElement(CV_SHAPE_RECT, s1, p1);
	//膨胀函数   给图像中的对象边界添加像素  使边界更加明显
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
	//上边得到种子生长的两个阈值
	//! applies fixed threshold to the image
	threshold(dstFrame, dstFrame, lowThresh, 255, CV_THRESH_TOZERO);//把低于小阈值的点设置为0
	threshold(dstFrame, dstFrame, upThresh, 255, CV_THRESH_TRUNC);//把高于大阈值的点设置为大阈值
	return true;
}


void threeFrameThread::diffFrameCal()
{
	Mat formDiffFrameA;
	Mat formDiffFrameB;
	formDiffFrameA.create(grayNext.size(), CV_BGR2GRAY);
	formDiffFrameB.create(grayNext.size(), CV_BGR2GRAY);
	//! computes element-wise absolute difference of two arrays (dst = abs(src1 - src2))
	//比较两个元素的绝对差  formDiffFrameB=abs（grayPre-grayCur）
	absdiff(grayPre, grayCur, formDiffFrameB);
	absdiff(grayNext, grayCur, formDiffFrameA);
	//进行膨胀处理，使边缘更加明显
	diffFrameMorph(formDiffFrameB);
	diffFrameMorph(formDiffFrameA);
	//! computes per-element minimum of two arrays (dst = min(src1, src2))
	//比较两个向量最小的那个
	cv::min(formDiffFrameB, formDiffFrameA, transitFrame);
	DiffAndFastSeedGrow(transitFrame);
}

//用掩膜法确定运动的方框
bool threeFrameThread::drawRectMask(){
	int flag = 0, flag1 = 1;//区域变化标志
	flag = flag1;
	Rect movRect;
	Size movSize;
	Point startRect;
	vector< vector<Point> >  contours;//初始轮廓
	vector<Vec4i> hierarchy;//保存轮廓之间关系的数组
	//! retrieves contours and the hierarchical information from black-n-white image.
	//从灰度图片中得到轮廓与分层的信息
	//第二个参数返回轮廓的的值
	findContours(transitFrame, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	for (size_t i = 0; i < contours.size(); i++)
	{
		//计算轮廓的方框
		movRect = boundingRect(Mat(contours[i]));
		//确保得到的轮廓方框在允许的轮廓范围内
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
		//push_back在vector类中作用为在vector尾部加入一个数据
		frameBuf->pMovContours.push_back(movRect);
	}
#if 1 //DEBUG_SHOW
	//frameBuf->framDiff = rectImg;
	
#endif
	return true;
}

/******************获取运动区域线程**************/
DWORD WINAPI threeFrameThread::threeFramefun(LPVOID param)
{
	unsigned int intimes = 0;
	unsigned int currentframes = 0;//当前线程处理的帧
	threeFrameThread* obj = (threeFrameThread*)param;  //创建对象
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
/******************获取运动区域线程**************/
void threeFrameThread::threeFramefunD()
{
	unsigned int intimes = 0;
	unsigned int currentframes = 0;//当前线程处理的帧
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

/******************获取运动区域线程**************/
void threeFrameThread::threeFramefunD()
{
	unsigned int intimes = 0;
	unsigned int currentframes = 0;//当前线程处理的帧
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