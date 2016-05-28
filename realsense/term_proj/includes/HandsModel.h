/*******************************************************************************
 
INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2011-2015 Intel Corporation. All Rights Reserved.
 
*******************************************************************************/
#pragma once

#ifndef HANDSMODEL_H
#define	HANDSMODEL_H

#include <thread>
#include <mutex> 
#include <iostream>

#include "pxcsession.h"
#include "pxcsensemanager.h"
#include "PXCHandConfiguration.h"

#include "Tree.h"



#define MAX_NUMBER_OF_HANDS 2
#define MAX_NUMBER_OF_JOINTS 22

namespace ModelViewController
{
	class HandsModel
	{
	public:
		HandsModel();
		HandsModel(const HandsModel& src);
		HandsModel& operator=(const HandsModel& src);
		~HandsModel();
		pxcStatus Init(PXCSenseManager* senseManager,bool isFullHand);
		void initSkeletonTree(Tree<PXCHandData::JointData>* tree);
		Tree<PXCHandData::JointData>* getSkeletonTree();
		void setSkeleton(Tree<PXCHandData::JointData>* skeletonTree);
		bool updateModel();
		bool hasRightHand();
		bool hasLeftHand();
		bool get2DImage(pxcBYTE* depthmap);
		pxcI32 get2DImageHeight();
		pxcI32 get2DImageWidth();
		bool isModelPaused();
		void pause(bool isPause, bool isModel);
		bool isFullHandMode();

	private:
		void updateSkeleton();
		Tree<PXCHandData::JointData>* m_skeletonTree;
		void updateskeletonTree();
		void update2DImage();
		void updateCursorGestureData();
		PXCHandModule* m_handModule;
		PXCHandCursorModule* m_handCursorModule;
		PXCHandData* m_handData;
		PXCCursorData* m_cursorData;
		bool m_rightHandExist;
		bool m_leftHandExist;
		pxcBYTE* m_depthmap;
		pxcI32 m_imageHeight;
		pxcI32 m_imageWidth;
		bool m_fullHandMode;
		bool m_isPaused;
		bool m_gestureFired;
		PXCSenseManager* m_senseManager;
	};

}

#endif	/* HANDSMODEL_H */