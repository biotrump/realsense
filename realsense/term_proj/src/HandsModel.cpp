/*
The Hand Module consists two separate tracking modes (Full-hand & Extremities) , 
each designed to solve a different use-case, while the Cursor Module has a single tracking mode.. 
These modes differ in the information they provide and the computation resources that they require:

· Cursor Module – returns a single point on the hand, allowing very accurate and 
	responsive tracking and a limited set of gestures. The Cursor Module was designed to 
	solve the hand-based UI control use-case.

· Extremities mode (Hand Module) – returns the general location of the hand, its silhouette, 
	and the extremities of the hand: the hand’s top-most, bottom-most, right-most, left-most, 
	center and closest (to the sensor) points. This mode was designed to provide a light-weight method 
	of tracking the user’s hand.

· Full-hand mode (Hand Module) – returns the full 3D skeleton of the hand, including all 22 joints, 
fingers information, gestures, and more. This mode was designed to provide a full set of features 
for each tracked hand.

While the Hand Module and Cursor Module are the best choice when hand-specific features are required, 
they are not the only option for incorporating hand tracking into your application. 
Depending on the requirements of your application, you may wish to use the Blob Module instead.
If your application requires tracking of any object and not necessarily the hand, 
you may prefer to use the Blob Module. If you specifically require hand identification and 
more detailed information such as hand side, joint positions and gesture recognition, 
you will need to use the Hand Module.

*/
#include "HandsModel.h"
#include "pxccursorconfiguration.h"

using namespace ModelViewController;

//===========================================================================//

HandsModel::HandsModel() : m_rightHandExist(false),m_leftHandExist(false),m_depthmap(0),m_imageWidth(640),m_imageHeight(480)
{
	m_skeletonTree = new Tree<PXCHandData::JointData>[MAX_NUMBER_OF_HANDS];
	m_fullHandMode = false;
	m_isPaused = false;
	initSkeletonTree(&m_skeletonTree[0]);
	initSkeletonTree(&m_skeletonTree[1]);
}

//===========================================================================//

HandsModel::HandsModel(const HandsModel& src)
{
	int treeSize = sizeof(Tree<PXCHandData::JointData>) * MAX_NUMBER_OF_HANDS;
	m_skeletonTree = new Tree<PXCHandData::JointData>[MAX_NUMBER_OF_HANDS];
	memcpy_s(m_skeletonTree,treeSize,src.m_skeletonTree,treeSize);

	m_handModule = src.m_handModule;
	m_depthmap = src.m_depthmap;
	m_handData = src.m_handData;
	m_imageHeight = src.m_imageHeight;
	m_imageWidth = src.m_imageWidth;
	m_leftHandExist = src.m_leftHandExist;
	m_rightHandExist = src.m_rightHandExist;
	m_senseManager = src.m_senseManager;
}

//===========================================================================//

HandsModel& HandsModel::operator=(const HandsModel& src)
{
	if (&src == this) return *this;
	int treeSize = sizeof(Tree<PXCHandData::JointData>) * MAX_NUMBER_OF_HANDS;
	memcpy_s(m_skeletonTree,treeSize,src.m_skeletonTree,treeSize);
	m_handModule = src.m_handModule;
	m_depthmap = src.m_depthmap;
	m_handData = src.m_handData;
	m_imageHeight = src.m_imageHeight;
	m_imageWidth = src.m_imageWidth;
	m_leftHandExist = src.m_leftHandExist;
	m_rightHandExist = src.m_rightHandExist;
	m_senseManager = src.m_senseManager;
	return *this;
}

//===========================================================================//

//The Hand Module consists two separate tracking modes(Full - hand & Extremities)
//Cursor Module has a single tracking mode
pxcStatus HandsModel::Init(PXCSenseManager* senseManager,bool isFullHand)
{
	m_senseManager = senseManager;

	m_fullHandMode = isFullHand;

	// Error checking Status
	pxcStatus status = PXC_STATUS_INIT_FAILED;
	bool enableCursor = false;
	
	// Enable hands module in the multi modal pipeline
	if(m_fullHandMode==false)
	{
		//The HandCursorModule interface provides member functions to perform hand cursor tracking and gesture recognition
		status = senseManager->EnableHandCursor();
		if(status != PXC_STATUS_NO_ERROR)
		{
			m_fullHandMode = true;
		}else
		{
			enableCursor = true;
		}
	}
	
	if(m_fullHandMode)
	{	
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
	}
	else
	{
		if(enableCursor==false)
		{
			// Enable hand cursor module in the multi modal pipeline
			status = senseManager->EnableHandCursor();
			if(status != PXC_STATUS_NO_ERROR)
			{
				return status;
			}
		}
		
		// Retrieve cursor module if ready - called in the setup stage before AcquireFrame
		m_handCursorModule = senseManager->QueryHandCursor();
		if(!m_handCursorModule)
		{
			return status;
		}

		// Retrieves an instance of the PXCCursorData interface
		m_cursorData = m_handCursorModule->CreateOutput();
		
		if(!m_cursorData)
		{
			return PXC_STATUS_INIT_FAILED;
		}

		// Apply desired hand cursor configuration
		PXCCursorConfiguration* config = m_handCursorModule->CreateActiveConfiguration();
		config->EnableAllGestures();
		config->ApplyChanges();
		config->Release();
		config = NULL;
	}

	//If we got to this stage return success
	return PXC_STATUS_NO_ERROR;
}

//===========================================================================//

bool HandsModel::isFullHandMode()
{
	return m_fullHandMode;
}

//===========================================================================//

void HandsModel::pause(bool isPause,bool isModel)
{
	m_senseManager->QueryCaptureManager()->SetPause(isPause);
	if(!isModel)
	{
		if(m_fullHandMode)
			m_senseManager->PauseHand(isPause);//The PauseHand function pauses or resumes the hand tracking module. If paused, the pipeline no longer delivers any input samples to the module
		else
			m_senseManager->PauseHandCursor(isPause);//The PauseHandCursor function pauses or resumes the hand cursor module

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
	if(m_fullHandMode)
	{//returns the current images processed by the hand module.
		sample = (PXCCapture::Sample*)m_senseManager->QueryHandSample();
	}
	else
	{//returns the current images processed by the hand cursor module
		sample = (PXCCapture::Sample*)m_senseManager->QueryHandCursorSample();
	}
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

void HandsModel::initSkeletonTree(Tree<PXCHandData::JointData>* tree)
{
	PXCHandData::JointData jointData;
	memset(&jointData,0,sizeof(PXCHandData::JointData));
	Node<PXCHandData::JointData> rootDataNode(jointData);
	
	for(int i = 2 ; i < MAX_NUMBER_OF_JOINTS - 3 ; i+=4)
	{				
		Node<PXCHandData::JointData> dataNode(jointData);
		Node<PXCHandData::JointData> dataNode1(jointData);
		Node<PXCHandData::JointData> dataNode2(jointData);
		Node<PXCHandData::JointData> dataNode3(jointData);
		
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
	if(m_fullHandMode)
	{
		// Update hands data with current frame information
		if(m_handData->Update()!= PXC_STATUS_NO_ERROR)
			return false;
	}
	else
	{
		// Update cursors data with current frame information
		if(m_cursorData->Update()!= PXC_STATUS_NO_ERROR)
			return false;
	}
	
	// Update skeleton tree model
	updateskeletonTree();

	// Update gesture data
	if(m_fullHandMode==false)
	{
		updateCursorGestureData();
	}

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

//For all gestures, the user’s hand motion is tracked and “translated” into cursor motion.
void HandsModel::updateCursorGestureData()
{
	PXCCursorData::GestureData gestureData;
	int numOfCursors = m_cursorData->QueryNumberOfCursors();
	if(numOfCursors == 2)
	{
		for(int i = 0 ; i < m_cursorData->QueryFiredGesturesNumber() ; ++i)
		{
			//Handling gestures data
			if(m_cursorData->QueryFiredGestureData(i,gestureData) == PXC_STATUS_NO_ERROR)
			{
				//Open hand facing the camera, moves the index finger quickly toward the palm center
				if(gestureData.label == PXCCursorData::CURSOR_CLICK)
				{
					m_gestureFired = !m_gestureFired;

					pause(m_gestureFired,true);
				}
			}
		}
	}
}

//===========================================================================//

void HandsModel::updateskeletonTree()
{
	m_rightHandExist = false;
	m_leftHandExist = false;

	if(m_fullHandMode)
	{
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
				}
				else if (handOutput->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_LEFT)
				{
					m_leftHandExist = true;
					side = 1;
				}

				PXCHandData::JointData jointData;

				handOutput->QueryTrackedJoint(PXCHandData::JointType::JOINT_WRIST,jointData);

				Node<PXCHandData::JointData> rootDataNode(jointData);

				// Iterate over hand joints
				for(int i = 2 ; i < MAX_NUMBER_OF_JOINTS - 3 ; i+=4)
				{				
					handOutput->QueryTrackedJoint((PXCHandData::JointType)(i+3),jointData);
					Node<PXCHandData::JointData> dataNode(jointData);
					handOutput->QueryTrackedJoint((PXCHandData::JointType)(i+2),jointData);
					Node<PXCHandData::JointData> dataNode1(jointData);
					handOutput->QueryTrackedJoint((PXCHandData::JointType)(i+1),jointData);
					Node<PXCHandData::JointData> dataNode2(jointData);
					handOutput->QueryTrackedJoint((PXCHandData::JointType)(i),jointData);
					Node<PXCHandData::JointData> dataNode3(jointData);

					dataNode1.add(dataNode);
					dataNode2.add(dataNode1);
					dataNode3.add(dataNode2);
					rootDataNode.add(dataNode3);
				}

				m_skeletonTree[side].setRoot(rootDataNode);

			}
		}
	}

	else
	{
		// Iterate over cursors
		int numOfCursors = m_cursorData->QueryNumberOfCursors();
		for(int index = 0 ; index < numOfCursors ; ++index)
		{
			PXCCursorData::ICursor* cursor;
			if(m_cursorData->QueryCursorData(PXCCursorData::ACCESS_ORDER_BY_TIME,index,cursor) == PXC_STATUS_NO_ERROR)
			{
				// Get hand body side (left, right, unknown)
				int side = 0;
				if(cursor->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_RIGHT)
				{
					m_rightHandExist = true;
					side = 0;
				}
				else if (cursor->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_LEFT)
				{
					m_leftHandExist = true;
					side = 1;
				}
				PXCHandData::JointData cursorData = {};
				PXCPoint3DF32 point = cursor->QueryCursorWorldPoint();
				cursorData.positionWorld = point;
				m_skeletonTree[side].setRoot(cursorData);
			}		
		}
	}
}

//===========================================================================//

Tree<PXCHandData::JointData>* HandsModel::getSkeletonTree()
{
	return m_skeletonTree;
}

//===========================================================================//

void HandsModel::setSkeleton(Tree<PXCHandData::JointData>* skeletonTree)
{
	m_skeletonTree = skeletonTree;
}

//===========================================================================//

HandsModel::~HandsModel()
{
	if(m_skeletonTree)
		delete [] m_skeletonTree;
}