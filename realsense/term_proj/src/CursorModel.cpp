#include "CursorModel.h"
#include "pxchandcursormodule.h"
#include "pxccursorconfiguration.h"
#include "pxchanddata.h"
#include "play_sound.h"
#include "oglModel.h"
using namespace ModelViewController;

#define MAX_NUMBER_OF_JOINTS 1

//===========================================================================//
//the hand position of farest and nearest
//m_farZPos : farest z pos, right handed coordination, z toward human face
//m_nearZPos :nearest z pos, right handed coordination, z toward human face
CursorModel::CursorModel() : m_rightHandExist(false),m_leftHandExist(false),m_depthmap(0),m_imageWidth(640),m_imageHeight(480),
m_farZPos{ { 0,0,1000000.0f },{ 0,0,1000000.0f } }, m_nearZPos{ { 0,0,-1000000.0f },{ 0,0,-1000000.0f } }
{
	m_skeletonTree = new Tree<PointData>[MAX_NUMBER_OF_HANDS];
	m_fullHandMode = false;
	m_isPaused = false;
	initSkeletonTree(&m_skeletonTree[0]);
	initSkeletonTree(&m_skeletonTree[1]);
}

//===========================================================================//

CursorModel::CursorModel(const CursorModel& src)
{
	*this = src;
}

//===========================================================================//

CursorModel& CursorModel::operator=(const CursorModel& src)
{
	if (&src == this) return *this;
	int treeSize = sizeof(Tree<PointData>) * MAX_NUMBER_OF_HANDS;
	int depthMapLength = sizeof(src.m_depthmap) / sizeof(src.m_depthmap[0]);
	memcpy_s(m_skeletonTree,treeSize,src.m_skeletonTree,treeSize);
	m_depthmap = new pxcBYTE[depthMapLength];
	memcpy_s(m_depthmap, sizeof(src.m_depthmap), src.m_depthmap, sizeof(src.m_depthmap));
	m_handCursorModule = src.m_handCursorModule;
	m_cursorData = src.m_cursorData;
	m_imageHeight = src.m_imageHeight;
	m_imageWidth = src.m_imageWidth;
	m_leftHandExist = src.m_leftHandExist;
	m_rightHandExist = src.m_rightHandExist;
	m_senseManager = src.m_senseManager;
	return *this;
}

//===========================================================================//

pxcStatus CursorModel::Init(PXCSenseManager* senseManager)
{
	m_senseManager = senseManager;

	// Error checking Status
	pxcStatus status = PXC_STATUS_INIT_FAILED;

	// Enable hands cursor module in the multi modal pipeline
	status = senseManager->EnableHandCursor();
	if(status != PXC_STATUS_NO_ERROR)
	{
		return status;
	}
	
	// Retrieve hand cursor module if ready - called in the setup stage before AcquireFrame
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

	// Apply desired hand configuration
	PXCCursorConfiguration* config = m_handCursorModule->CreateActiveConfiguration();
	config->EnableAllGestures();
	config->ApplyChanges();
	config->Release();
	config = NULL;
	

	//If we got to this stage return success
	return PXC_STATUS_NO_ERROR;
}

//===========================================================================//

void CursorModel::pause(bool isPause,bool isModel)
{
	m_senseManager->QueryCaptureManager()->SetPause(isPause);
	if(!isModel)
	{
		//If true, the pipeline will stop delivering samples to the module; 
		//if false, the pipeline resumes delivering samples to the module.
		m_senseManager->PauseHandCursor(isPause);

		if(isPause)
			m_isPaused = false;
	}
	else
		m_isPaused = true;
}

//===========================================================================//

void CursorModel::update2DImage()
{
	// Get camera streams
	PXCCapture::Sample *sample;
	sample = (PXCCapture::Sample*)m_senseManager->QueryHandCursorSample();
	
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

bool CursorModel::get2DImage(pxcBYTE* depthmap)
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

pxcI32 CursorModel::get2DImageHeight()
{
	return m_imageHeight;
}

//===========================================================================//

pxcI32 CursorModel::get2DImageWidth()
{
	return m_imageWidth;
}


//===========================================================================//

bool CursorModel::hasRightHand()
{
	return m_rightHandExist;
}

//===========================================================================//

bool CursorModel::hasLeftHand()
{
	return m_leftHandExist;
}

//===========================================================================//

void CursorModel::initSkeletonTree(Tree<PointData>* tree)
{
	PointData jointData;
	memset(&jointData,0,sizeof(PointData));
	Node<PointData> rootDataNode(jointData);

	tree->setRoot(rootDataNode);
}

//===========================================================================//

bool CursorModel::updateModel()
{
	// Update cursors data with current frame information
	if(m_cursorData->Update()!= PXC_STATUS_NO_ERROR)
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

bool CursorModel::isModelPaused()
{
	return m_isPaused;
}

//===========================================================================//
//capture gesture
void CursorModel::updateGestureData()
{
	PXCCursorData::GestureData gestureData;

	m_rightHandExist = false;
	m_leftHandExist = false;

	// Iterate over cursors
	int numOfCursors = m_cursorData->QueryNumberOfCursors();
	for (int index = 0; index < numOfCursors; ++index)
	{
		// retrieve the hand cursor identifier
		//pxcUID cursorId[2];
		//m_cursorData->QueryHandId(PXCCursorData::ACCESS_ORDER_NEAR_TO_FAR, 0, cursorId[index]);
		PXCCursorData::ICursor* cursor;
		if (m_cursorData->QueryCursorData(PXCCursorData::ACCESS_ORDER_NEAR_TO_FAR /*ACCESS_ORDER_BY_TIME*/, index, cursor) == PXC_STATUS_NO_ERROR)
		{
			// Get hand body side (left, right, unknown)
			int side = 0;
			if (cursor->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_RIGHT)
			{
				m_rightHandExist = true;
				side = 0;
				//printf("RH 0:");
			}
			else if (cursor->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_LEFT)
			{
				m_leftHandExist = true;
				side = 1;
				//printf("LH 1:");
			}
			PointData cursorData = {};
			PXCPoint3DF32 point = cursor->QueryCursorWorldPoint();
			cursorData.positionWorld = point;
			m_curPos[side] = point;
			//printf("\np [%d]: %f,%f,%f\n",  side, point.x, point.y, point.z);
			m_skeletonTree[side].setRoot(cursorData);

			if (point.z < m_farZPos[side].z) {
				m_farZPos[side] = point;//farest z pos, right handed coordination, z toward human face
				//printf("%d:CFar(%f,%f,%f)\n", side, point.x, point.y, point.z);
			}
			if (point.z > m_nearZPos[side].z) {
				m_nearZPos[side] = point;//nearest z pos, right handed coordination, z toward human face
				//printf("%d:Cnear(%f,%f,%f)\n", side, point.x, point.y, point.z);
			}
			//////////////////////////////////////////////
			//FMOD play by distance and gesture
			////////////////////////////////////////////
			//IsGestureFiredByHand
			//if (m_cursorData->IsGestureFired(PXCCursorData::CURSOR_CLICK, gestureData))
			
			if (m_cursorData->IsGestureFiredByHand(PXCCursorData::CURSOR_HAND_CLOSING, cursor->QueryUniqueId(),
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
					int depth = FMOD_NoteByDepth(point.z);
					//point.z = (point.z < 0.25f) ? 0. : point.z - 0.25f;
					//int depth = ceilf(point.z / 0.05);
					printf("\n\nL depth:%d\n\n", depth);
					FMOD_Play(KeyNote_C + depth);
					
				}

			}
			if (m_cursorData->IsGestureFiredByHand(PXCCursorData::CURSOR_CLOCKWISE_CIRCLE, 
				cursor->QueryUniqueId(), gestureData)) {
				modelZooming(1, 0.8f);//zoom in model
			}
			if (m_cursorData->IsGestureFiredByHand(PXCCursorData::CURSOR_COUNTER_CLOCKWISE_CIRCLE, 
				cursor->QueryUniqueId(), gestureData)) {
				modelZooming(0, 0.8f);//zoom out model
			}

#if 0
			pxcI32 firedGestures=m_cursorData->QueryFiredGesturesNumber();
			//for (int i = 0;i < firedGestures;i++)
			int i = 0;//side;/////
			{
				pxcStatus sts = m_cursorData->QueryFiredGestureData(i, gestureData);
				if (PXC_STATUS_NO_ERROR == sts)	{
					//gestureData.handId
					switch (gestureData.label) {
					case PXCCursorData::CURSOR_CLICK:
					{
						m_gestureFired = !m_gestureFired;
#if 0
						//pause(m_gestureFired,true);

						float temp = distance(point.x, point.y, point.z,
							m_lastPos[side].x, m_lastPos[side].y, m_lastPos[side].z);
						printf("CL delta=(%f,%f,%f): %f\n", point.x, point.y, point.z, temp);
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
							point.z = (point.z < 0.25f) ? 0. : point.z - 0.25f;
							int depth = ceilf(point.z / 0.05);
							printf("\nL depth:%d\n", depth);
							FMOD_Play(KeyNote_C + depth);
						}
#endif
					}
					break;
					case PXCCursorData::CURSOR_CLOCKWISE_CIRCLE:
						modelZooming(1,0.8f);//zoom in model
						break;
					case PXCCursorData::CURSOR_COUNTER_CLOCKWISE_CIRCLE:
						modelZooming(0,0.8f);//zoom out model
						break;
					case PXCCursorData::CURSOR_HAND_CLOSING:
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
							point.z = (point.z < 0.25f) ? 0. : point.z - 0.25f;
							int depth = ceilf(point.z / 0.05);
							printf("\n\nL depth:%d\n\n", depth);
							FMOD_Play(KeyNote_C + depth);
						}

					}
						break;
					case PXCCursorData::CURSOR_HAND_OPENING:
						break;
					default:
						break;
					}
				}
			}
#endif
		}
	}
	/*
	int numOfCursors = m_cursorData->QueryNumberOfCursors();
	if(numOfCursors == 2)
	{
		if(m_cursorData->IsGestureFired(PXCCursorData::CURSOR_CLICK,gestureData))
		{
			if(gestureData.label == PXCCursorData::CURSOR_CLICK)
			{
				m_gestureFired = !m_gestureFired;

				//pause(m_gestureFired,true);
			}
		}
	}
	*/
}

//===========================================================================//

void CursorModel::updateskeletonTree()
{
	m_rightHandExist = false;
	m_leftHandExist = false;

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
				//printf("RH 0:");
			}
			else if (cursor->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_LEFT)
			{
				m_leftHandExist = true;
				side = 1;
				//printf("LH 1:");
			}
			PointData cursorData = {};
			PXCPoint3DF32 point = cursor->QueryCursorWorldPoint();
			cursorData.positionWorld = point;
			m_curPos[side] = point;
			//printf("%s:Cpos %f,%f,%f\n",__func__,point.x, point.y, point.z);
			m_skeletonTree[side].setRoot(cursorData);

			if (point.z < m_farZPos[side].z) {
				m_farZPos[side]=point;//farest z pos, right handed coordination, z toward human face
				//printf("%d:CFar(%f,%f,%f)\n", side, point.x, point.y,point.z);
			}
			if (point.z > m_nearZPos[side].z) {
				m_nearZPos[side]=point;//nearest z pos, right handed coordination, z toward human face
				//printf("%d:Cnear(%f,%f,%f)\n", side, point.x, point.y, point.z);
			}
#if 0
			//////////////////////////////////////////////
			//FMOD play by distance and gesture
			////////////////////////////////////////////
			float temp = distance(point.x, point.y, point.z,
				m_lastPos[side].x, m_lastPos[side].y, m_lastPos[side].z);
			printf("CL delta=(%f,%f,%f): %f\n", point.x, point.y, point.z, temp);
			//if (temp >= 0.05f) 
			if( abs(m_lastPos[side].z - point.z) >= 0.049f ){
				//translation distance
				m_lastPos[side].x = point.x;
				m_lastPos[side].y = point.y;
				m_lastPos[side].z = point.z;
				//min z:0.22, max z:0.7, 0.7-0.2=0.5, 0.5/10=0.05
				//0.2-0.25 :C
				//0.25-0.3 :D
				point.z = (point.z < 0.25f) ? 0. : point.z - 0.25f;
				int depth = ceilf(point.z / 0.05);
				printf("\nL depth:%d\n", depth);
				FMOD_Play(KeyNote_C + depth);
			}
#endif
		}		
	}	
}

//===========================================================================//

Tree<PointData>* CursorModel::getSkeletonTree()
{
	return m_skeletonTree;
}

//===========================================================================//

void CursorModel::setSkeleton(Tree<PointData>* skeletonTree)
{
	m_skeletonTree = skeletonTree;
}

//===========================================================================//

CursorModel::~CursorModel()
{
	if(m_skeletonTree)
		delete [] m_skeletonTree;
	if(m_depthmap)
		delete [] m_depthmap;
}