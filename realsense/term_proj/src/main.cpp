#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>

#include <stdio.h>
#include <iostream>

#include "RssdkHandler.h"
#include "HandsModel.h"
#include "CursorModel.h"

ModelViewController::IModel* model;
ModelViewController::IView* openGLView;
ModelViewController::HandsController* controller;
RssdkHandler* rssdkHandler;


void releaseAll();


/** @brief The Hand Module consists two separate tracking modes (Full-hand & Extremities) , 
each designed to solve a different use-case, while the Cursor Module has a single tracking mode
Tracking mode:
cursor : 2 hands, fast speed, SR300 ??110 cm
	Cursor point in 2D and 3D, alerts, gestures
Full Hand : 2 hands, slower hand speed, SR300 ??85 cm
	Segmentation image, extremity points, hand side, alerts, 
	joints info, fingers info, openness, gestures
Extremities : 2 hands, medium hand speed, SR300 ??120 cm
	Segmentation image, extremity points, hand side, alerts

command line  : -full , full hand mode
*/

bool start(bool isFullHand, char *assimp_mpath, char *texture_path)
{
	pxcStatus status = PXC_STATUS_ALLOC_FAILED;

	if(isFullHand)
	{
		// Create hand model
		model = new ModelViewController::HandsModel();
	}
	else
	{
		// Create cursor model
		model = new ModelViewController::CursorModel();
	}

	if(!model)
	{
		return false;
	}

	// Create Openglview which implements IView (allows creations of different views)
	openGLView = new ModelViewController::OpenGLView(isFullHand, assimp_mpath, texture_path);
	if(!openGLView)
	{
		return false;
	}

	// When using sequence, change useSequence to true and apply sequencePath with sequence path
	bool useSequence = false;
	pxcCHAR* sequencePath = L"Insert Sequence Path Here";

	// Bind controller with model and view and start playing
	controller = new ModelViewController::HandsController(model,openGLView);
	if(!controller)
	{
		return false;
	}

	rssdkHandler = new RssdkHandler(controller,model,openGLView);

	if(useSequence)
	{
		if(rssdkHandler->Init(isFullHand,sequencePath) == PXC_STATUS_NO_ERROR)
		{
			rssdkHandler->Start();
		}

		else
		{
			return false;
		}
	}

	// Live Camera
	else
	{
		status = rssdkHandler->Init(isFullHand);
		if(status == PXC_STATUS_NO_ERROR)
		{
			rssdkHandler->Start();
		}
		else
		{
			return false;
		}
	}

	return true;
}

int main(int argc, char** argv)
{
	bool isFullHand = false;
	char *modelFile = NULL;
	char *textureFile = NULL;
	////////////////////////////////////////////////
	int i=1;

	while (i < argc) {
		printf("%d: %s\n", i, argv[i]);

		// Free memory allocated for CommandLineToArgvW arguments.
		if (strcmp(argv[i], "-fullhand") == 0)
		{
			isFullHand = true;
		}
		else if (strcmp(argv[i], "-m") == 0) {
			if (i < argc) {
				i++;
				modelFile = argv[i];
			}
		}else if (strcmp(argv[i], "-t") == 0) {
			if (i < argc) {
				i++;
				textureFile = argv[i];
			}
		}
		i++;
	}
	//////////////////////////////////////////////////////////

	// Full hand mode
	if(isFullHand)
	{
		if(!start(isFullHand, modelFile, textureFile))
		{
			std::printf("Failed at Initialization\n");
			releaseAll();
		}
	}
	// Cursor hand
	else
	{
		if(!start(isFullHand, modelFile, textureFile))
		{
			releaseAll();
			if(!start(!isFullHand, modelFile, textureFile))
			{
				std::printf("Failed at Initialization\n");
				releaseAll();
			}			
		}
	}
	
	releaseAll();
    return 0;
}

void releaseAll()
{
	//delete all pointers
	if(model)
	{
		delete model;
		model = NULL;
	}
	if(openGLView)
	{
		delete openGLView;
		openGLView = NULL;
	}
	if(controller)
	{
		delete controller;
		controller = NULL;
	}
	if(rssdkHandler)
	{
		delete rssdkHandler;
		rssdkHandler = NULL;
	}
}


