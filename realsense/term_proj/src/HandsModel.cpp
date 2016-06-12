#include "HandsModel.h"
#include "pxchandmodule.h"
#include "pxchanddata.h"
#include "pxchandconfiguration.h"
#include "pxccursorconfiguration.h"
#include "play_sound.h"
#include "oglModel.h"
using namespace ModelViewController;

#define MAX_NUMBER_OF_JOINTS 22

//===========================================================================//
//the hand position of farest and nearest
//m_farZPos : farest z pos, right handed coordination, z toward human face
//m_nearZPos :nearest z pos, right handed coordination, z toward human face
HandsModel::HandsModel() : m_rightHandExist(false),m_leftHandExist(false),m_depthmap(0),m_imageWidth(640),m_imageHeight(480),
m_farZPos{ { 0,0,1000000.0f },{ 0,0,1000000.0f } }, m_nearZPos{ { 0,0,-1000000.0f },{ 0,0,-1000000.0f } }
{
	m_skeletonTree = new Tree<PointData>[MAX_NUMBER_OF_HANDS];
	m_fullHandMode = false;
	m_isPaused = false;
	initSkeletonTree(&m_skeletonTree[0]);
	initSkeletonTree(&m_skeletonTree[1]);
}

//===========================================================================//

HandsModel::HandsModel(const HandsModel& src)
{
	*this = src;
}

//===========================================================================//

HandsModel& HandsModel::operator=(const HandsModel& src)
{
	if (&src == this) return *this;
	int treeSize = sizeof(Tree<PointData>) * MAX_NUMBER_OF_HANDS;
	int depthMapLength = sizeof(src.m_depthmap) / sizeof(src.m_depthmap[0]);
	memcpy_s(m_skeletonTree,treeSize,src.m_skeletonTree,treeSize);

	m_depthmap = new pxcBYTE[depthMapLength];
	memcpy_s(m_depthmap, sizeof(src.m_depthmap), src.m_depthmap, sizeof(src.m_depthmap));
	m_handModule = src.m_handModule;
	
	m_handData = src.m_handData;
	m_imageHeight = src.m_imageHeight;
	m_imageWidth = src.m_imageWidth;
	m_leftHandExist = src.m_leftHandExist;
	m_rightHandExist = src.m_rightHandExist;
	m_senseManager = src.m_senseManager;
	return *this;
}

//===========================================================================//

pxcStatus HandsModel::Init(PXCSenseManager* senseManager)
{
	m_senseManager = senseManager;

	// Error checking Status
	pxcStatus status = PXC_STATUS_INIT_FAILED;

	// Enable hands module in the multi modal pipeline
	status = senseManager->EnableHand();
	if(status != PXC_STATUS_NO_ERROR)
	{
		return status;
	}

	// Retrieve hand module if ready - called in the setup stage before AcquireFrame
	m_handModule = senseManager->QueryHand();
	if(!m_handModule)
	{
		return status;
	}

	// Retrieves an instance of the PXCHandData interface
	m_handData = m_handModule->CreateOutput();
	if(!m_handData)
	{
		return PXC_STATUS_INIT_FAILED;
	}

	// Apply desired hand configuration
	PXCHandConfiguration* config = m_handModule->CreateActiveConfiguration();
	config->EnableAllGestures();
	config->EnableSegmentationImage(false);
	config->ApplyChanges();
	config->Release();
	config = NULL;
	

	//If we got to this stage return success
	return PXC_STATUS_NO_ERROR;
}

//===========================================================================//

void HandsModel::pause(bool isPause,bool isModel)
{
	m_senseManager->QueryCaptureManager()->SetPause(isPause);
	if(!isModel)
	{
		m_senseManager->PauseHand(isPause);
		if(isPause)
			m_isPaused = false;
	}
	else
		m_isPaused = true;
}

//===========================================================================//

void HandsModel::update2DImage()
{
	// Get camera streams
	PXCCapture::Sample *sample;
	sample = (PXCCapture::Sample*)m_senseManager->QueryHandSample();
	
	if(sample && sample->depth)
	{
		PXCImage::ImageInfo imageInfo = sample->depth->QueryInfo();	
		m_imageHeight = imageInfo.height;
		m_imageWidth = imageInfo.width;

		// Get camera depth stream
		PXCImage::ImageData imageData;
		sample->depth->AcquireAccess(PXCImage::ACCESS_READ,PXCImage::PIXEL_FORMAT_RGB32, &imageData);
		{
			if(m_depthmap != 0)
				delete[] m_depthmap;

			int bufferSize = m_imageWidth * m_imageHeight * 4;
			m_depthmap = new pxcBYTE[bufferSize];
			memcpy_s(m_depthmap,bufferSize, imageData.planes[0], bufferSize);
		}
		sample->depth->ReleaseAccess(&imageData);
	}
}

//===========================================================================//

bool HandsModel::get2DImage(pxcBYTE* depthmap)
{
	if(m_depthmap)
	{
		int bufferSize = m_imageWidth * m_imageHeight * 4;
		memcpy_s(depthmap, bufferSize, m_depthmap, bufferSize);		
		return true;
	}

	return false;
}

//===========================================================================//

pxcI32 HandsModel::get2DImageHeight()
{
	return m_imageHeight;
}

//===========================================================================//

pxcI32 HandsModel::get2DImageWidth()
{
	return m_imageWidth;
}


//===========================================================================//

bool HandsModel::hasRightHand()
{
	return m_rightHandExist;
}

//===========================================================================//

bool HandsModel::hasLeftHand()
{
	return m_leftHandExist;
}

//===========================================================================//

void HandsModel::initSkeletonTree(Tree<PointData>* tree)
{
	PointData jointData;
	memset(&jointData,0,sizeof(PointData));
	Node<PointData> rootDataNode(jointData);
	
	for(int i = 2 ; i < MAX_NUMBER_OF_JOINTS - 3 ; i+=4)
	{				
		Node<PointData> dataNode(jointData);
		Node<PointData> dataNode1(jointData);
		Node<PointData> dataNode2(jointData);
		Node<PointData> dataNode3(jointData);
		
		dataNode1.add(dataNode);
		dataNode2.add(dataNode1);
		dataNode3.add(dataNode2);
		rootDataNode.add(dataNode3);
	}

	tree->setRoot(rootDataNode);
}

//===========================================================================//

bool HandsModel::updateModel()
{

	// Update hands data with current frame information
	if(m_handData->Update()!= PXC_STATUS_NO_ERROR)
		return false;
	
	// Update skeleton tree model
	updateskeletonTree();

	// Update gesture data
	updateGestureData();

	// Update image
	update2DImage();

	return true;
}

//===========================================================================//

bool HandsModel::isModelPaused()
{
	return m_isPaused;
}

//===========================================================================//

void HandsModel::updateskeletonTree()
{
	m_rightHandExist = false;
	m_leftHandExist = false;

	// Iterate over hands
	int numOfHands = m_handData->QueryNumberOfHands();
	for(int index = 0 ; index < numOfHands ; ++index)
	{
		// Get hand by access order of entering time
		PXCHandData::IHand* handOutput = NULL;
		if(m_handData->QueryHandData(PXCHandData::ACCESS_ORDER_BY_TIME,index,handOutput) == PXC_STATUS_NO_ERROR)
		{
			// Get hand body side (left, right, unknown)
			int side = 0;
			if(handOutput->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_RIGHT)
			{
				m_rightHandExist = true;
				side = 0;
				//printf("RH 0:");
			}
			else if (handOutput->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_LEFT)
			{
				m_leftHandExist = true;
				side = 1;
				//printf("LH 1:");
			}

			PXCHandData::JointData jointData;
			handOutput->QueryTrackedJoint(PXCHandData::JointType::JOINT_WRIST,jointData);
			PointData pointData;
			copyJointToPoint(pointData,jointData);

			Node<PointData> rootDataNode(pointData);
			PXCPoint3DF32 point = pointData.positionWorld;
			m_curPos[side] = point;
			//printf("%s:pos %f,%f,%f", __func__, point.x, point.y, point.z);
			
			if (point.z < m_farZPos[side].z) {
				m_farZPos[side] = point;//farest z pos, right handed coordination, z toward human face
				//printf("%d:Far(%f,%f,%f)\n", side, point.x, point.y, point.z);
			}
			if (point.z > m_nearZPos[side].z) {
				m_nearZPos[side] = point;//nearest z pos, right handed coordination, z toward human face
				//printf("%d:near(%f,%f,%f)\n", side, point.x, point.y, point.z);
			}

			// Iterate over hand joints
			for(int i = 2 ; i < MAX_NUMBER_OF_JOINTS - 3 ; i+=4)
			{				
				handOutput->QueryTrackedJoint((PXCHandData::JointType)(i+3),jointData);
				copyJointToPoint(pointData,jointData);
				Node<PointData> dataNode(pointData);
				handOutput->QueryTrackedJoint((PXCHandData::JointType)(i+2),jointData);
				copyJointToPoint(pointData,jointData);
				Node<PointData> dataNode1(pointData);
				handOutput->QueryTrackedJoint((PXCHandData::JointType)(i+1),jointData);
				copyJointToPoint(pointData,jointData);
				Node<PointData> dataNode2(pointData);
				handOutput->QueryTrackedJoint((PXCHandData::JointType)(i),jointData);
				copyJointToPoint(pointData,jointData);
				Node<PointData> dataNode3(pointData);

				dataNode1.add(dataNode);
				dataNode2.add(dataNode1);
				dataNode3.add(dataNode2);
				rootDataNode.add(dataNode3);
			}

			m_skeletonTree[side].setRoot(rootDataNode);

			//////////////////////////////////////////////
			//FMOD play by distance and gesture
			////////////////////////////////////////////
			float temp = distance(point.x, point.y, point.z, 
				m_lastPos[side].x, m_lastPos[side].y, m_lastPos[side].z);
			//printf("CL delta=(%f,%f,%f): %f\n", point.x, point.y, point.z, temp);
			if (temp >= 0.05f) {
				//translation distance
				m_lastPos[side].x = point.x;
				m_lastPos[side].y = point.y;
				m_lastPos[side].z = point.z;
				//min z:0.22, max z:0.7, 0.7-0.2=0.5, 0.5/10=0.05
				//0.2-0.25 :C
				//0.25-0.3 :D
				point.z = (point.z < 0.25f) ? 0. : point.z - 0.25f;
				//int depth = ceilf(point.z / 0.05);
				//printf("\nL depth:%d\n", depth);
				//FMOD_Play(KeyNote_C + depth);
			}
		}
	}
}

//===========================================================================//

//capture gesture
void HandsModel::updateGestureData()
{
	// poll for gestures
	PXCHandData::GestureData gestureData;
	m_rightHandExist = false;
	m_leftHandExist = false;

	// Iterate over hands
	int numOfHands = m_handData->QueryNumberOfHands();
	for (int index = 0; index < numOfHands; ++index)
	{
		// Get hand by access order of entering time
		PXCHandData::IHand* handOutput = NULL;
		if (m_handData->QueryHandData(PXCHandData::ACCESS_ORDER_BY_TIME, index, handOutput) == PXC_STATUS_NO_ERROR)
		{
			// Get hand body side (left, right, unknown)
			int side = 0;
			if (handOutput->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_RIGHT)
			{
				m_rightHandExist = true;
				side = 0;
				//printf("RH 0:");
			}
			else if (handOutput->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_LEFT)
			{
				m_leftHandExist = true;
				side = 1;
				//printf("LH 1:");
			}

			PXCHandData::JointData jointData;
			handOutput->QueryTrackedJoint(PXCHandData::JointType::JOINT_WRIST, jointData);
			PXCPoint3DF32 point = jointData.positionWorld;
			m_curPos[side] = point;
			//			printf("%s:pos %f,%f,%f", __func__, point.x, point.y, point.z);

			if (point.z < m_farZPos[side].z) {
				m_farZPos[side] = point;//farest z pos, right handed coordination, z toward human face
										//printf("%d:Far(%f,%f,%f)\n", side, point.x, point.y, point.z);
			}
			if (point.z > m_nearZPos[side].z) {
				m_nearZPos[side] = point;//nearest z pos, right handed coordination, z toward human face
										 //printf("%d:near(%f,%f,%f)\n", side, point.x, point.y, point.z);
			}



			if (m_handData->IsGestureFired(L"tap", gestureData)) {

				// handle tap gesture

				printf("%C:tap\n", side ? 'R' : 'L');

			}

			if (m_handData->IsGestureFired(L"wave", gestureData)) {

				// handle wave gesture
				printf("%C:wave\n", side ? 'R' : 'L');

			}

			if (m_handData->IsGestureFired(L"click", gestureData)) {

				// handle wave gesture
				printf("%C:click\n", side ? 'R' : 'L');

			}

			if (m_handData->IsGestureFired(L"fist", gestureData)) {

				// handle wave gesture
				printf("%C:fist\n", side ? 'R' : 'L');

			}

			if (m_handData->IsGestureFired(L"v_sign", gestureData)) {

				// handle wave gesture
				printf("%C:v_sign\n", side ? 'R' : 'L');

			}
			if (m_handData->IsGestureFired(L"spreadfingers", gestureData)) {

				// handle wave gesture
				printf("%C:spreadfingers\n", side ? 'R' : 'L');

			}
			//////////////////////////////////////////////
			//FMOD play by distance and gesture
			////////////////////////////////////////////
			//IsGestureFiredByHand
			//if (m_cursorData->IsGestureFired(PXCCursorData::CURSOR_CLICK, gestureData))

			if (m_handData->IsGestureFiredByHand(L"full_pinch", handOutput->QueryUniqueId(),
				gestureData))
			{
				printf("\np [%d]: %f,%f,%f\n", side, point.x, point.y, point.z);

				//m_gestureFired = !m_gestureFired;

				//pause(m_gestureFired,true);

				float temp = distance(point.x, point.y, point.z,
					m_lastPos[side].x, m_lastPos[side].y, m_lastPos[side].z);
				//printf("CL delta=(%f,%f,%f): %f\n", point.x, point.y, point.z, temp);

				//if (temp >= 0.05f) 
				//if (abs(m_lastPos[side].z - point.z) >= 0.049f) 
				{
					//translation distance
					m_lastPos[side].x = point.x;
					m_lastPos[side].y = point.y;
					m_lastPos[side].z = point.z;
					//min z:0.22, max z:0.7, 0.7-0.2=0.5, 0.5/10=0.05
					//0.2-0.25 :C
					//0.25-0.3 :D
					int depth = 0;
					if (point.z < 0.21f)
						depth = 0;
					else if (point.z < 0.27f)
						depth = 1;
					else if (point.z < 0.34f)
						depth = 2;
					else if (point.z < 0.438f)
						depth = 3;
					else if (point.z < 0.5f)
						depth = 4;
					else if (point.z < 0.58f)
						depth = 5;
					else if (point.z < 0.63f)
						depth = 6;
					else if (point.z < 0.66f)
						depth = 7;
					else depth = 8;
					//point.z = (point.z < 0.25f) ? 0. : point.z - 0.25f;
					//int depth = ceilf(point.z / 0.05);
					printf("\n\nL depth:%d\n\n", depth);
					FMOD_Play(KeyNote_C + depth);

				}

			}
			if (m_handData->IsGestureFiredByHand(L"swipe_left",
				handOutput->QueryUniqueId(), gestureData)) {
				modelZooming(1, 0.8f);//zoom in model
			}
			if (m_handData->IsGestureFiredByHand(L"swipe_right",
				handOutput->QueryUniqueId(), gestureData)) {
				modelZooming(0, 0.8f);//zoom out model
			}
		}
	}
}

//===========================================================================//

void HandsModel::copyJointToPoint(PointData & dst,const PXCHandData::JointData & src)
{
	dst.confidence = src.confidence;
	dst.globalOrientation = src.globalOrientation;
	dst.localRotation = src.localRotation;
	dst.positionImage = src.positionImage;
	dst.positionWorld = src.positionWorld;
	dst.speed = src.speed;
}

//===========================================================================//

Tree<PointData>* HandsModel::getSkeletonTree()
{
	return m_skeletonTree;
}

//===========================================================================//

void HandsModel::setSkeleton(Tree<PointData>* skeletonTree)
{
	m_skeletonTree = skeletonTree;
}

//===========================================================================//

HandsModel::~HandsModel()
{
	if(m_skeletonTree)
		delete [] m_skeletonTree;
	if(m_depthmap)
		delete [] m_depthmap;
}