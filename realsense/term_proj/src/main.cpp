#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>

#include <stdio.h>
#include <iostream>

#include "RssdkHandler.h"

ModelViewController::HandsModel* handModel;
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
int main(int argc, char** argv)
{
	bool isFullHand = false;
	if(argc == 2)
	{
		if (strcmp(argv[1],"-full")==0)
		{
			isFullHand = true;
		}
	}
	/*
	https://social.msdn.microsoft.com/Forums/windows/en-US/ccee7ae6-75ca-4916-89cf-a099d12074a7/send-message-to-other-process?forum=winforms
	find tmidi window : IntPtr FindWindow(string lpClassName, string lpWindowName);
	SendMessage (IntPtr hwnd, int wMsg, int wParam, int lParam);
	SendDlgItemMessage(hwndApp, IDC_TEMPO_SLIDER, TBM_SETPOS, TRUE, (int) (60000000.0f / new_tempo));
	
	// Check to see if another instance of the program is running
	COPYDATASTRUCT cds;
	HWND hwnd = FindWindow(NULL, "TMIDI: Tom's MIDI Player");
	cds.dwData = IPC_PLAY;
	cds.cbData = strlen(fnptr) + 1;
	cds.lpData = fnptr;
	if (hwnd)
	{
		// If so, tell that instance to play the specified file
		SendMessage(hwnd, WM_COPYDATA, (WPARAM) hwnd, (LPARAM) &cds);
		return 0;
	}
	else{
		playlist_add(fnptr);
	}
	*/
	COPYDATASTRUCT cds;
	HWND hwnd = FindWindow(NULL, TEXT("TMIDI: Tom's MIDI Player"));
	//HWND hwnd = FindWindow(NULL, TEXT("Tempo"));
	HWND hTempo=NULL,hVelocity=NULL;
	if (hwnd) {
		printf("hwnd=0x%x\n", hwnd);
		hTempo = FindWindowEx(hwnd, NULL, NULL, TEXT("Tempo"));
		printf("hTempo=0x%x\n", hTempo);
		SendDlgItemMessage(hwnd, 11054/*IDC_TEMPO_SLIDER*/, TBM_SETPOS, TRUE, 180);
		SendDlgItemMessage(hwnd, 11055/*IDC_TEMPO_SLIDER*/, TBM_SETTICFREQ, 500 / 10, 0);

		hVelocity = FindWindowEx(hwnd, NULL, NULL, TEXT("Velocity"));
		printf("hVelocity=0x%x\n", hVelocity);
		/*
		HWND hvelocity= GetDlgItem(
			_In_opt_ HWND hDlg,//IDD_MAIN
			velocity
		);
		(((HWND)lParam) == GetDlgItem(hDlg, IDC_VELOCITY_SLIDER));
		if (!get_scroll_value(wParam, lParam, &i))

		HWND htempo = GetDlgItem(
			_In_opt_ HWND hDlg, //IDD_MAIN
			tempo
		);
		SendDlgItemMessage(hwndMainDlg, IDC_VELOCITY_SLIDER, TBM_SETPOS, TRUE, 120);
		SendDlgItemMessage(hwndMainDlg, IDC_VELOCITY_SLIDER, TBM_SETTICFREQ, 500 / 10, 0);
		if (((HWND)lParam) == GetDlgItem(hDlg, IDC_TEMPO_SLIDER))
		{
			if (!get_scroll_value(wParam, lParam, &i) && i > 0)
				set_tempo((int)(60000000.0f / i));
		}
		//tempo
		SendDlgItemMessage(hwndMainDlg, IDC_TEMPO_SLIDER, TBM_SETPOS, TRUE, 120);
		SendDlgItemMessage(hwndMainDlg, IDC_TEMPO_SLIDER, TBM_SETTICFREQ, 500 / 10, 0);
		*/
	}else{
		printf("Not found! try to open it!\n");
		/*
		BOOL WINAPI CreateProcess(
			_In_opt_    LPCTSTR               lpApplicationName,
			_Inout_opt_ LPTSTR                lpCommandLine,
			_In_opt_    LPSECURITY_ATTRIBUTES lpProcessAttributes,
			_In_opt_    LPSECURITY_ATTRIBUTES lpThreadAttributes,
			_In_        BOOL                  bInheritHandles,
			_In_        DWORD                 dwCreationFlags,
			_In_opt_    LPVOID                lpEnvironment,
			_In_opt_    LPCTSTR               lpCurrentDirectory,
			_In_        LPSTARTUPINFO         lpStartupInfo,
			_Out_       LPPROCESS_INFORMATION lpProcessInformation
		);*/
	}
	exit(1);
	////////////////////////////////
	pxcStatus status = PXC_STATUS_ALLOC_FAILED;

	// Create hand model
	handModel = new ModelViewController::HandsModel();
	if(!handModel)
	{
		std::printf("Failed at Initialization\n");
		releaseAll();
		return -1;
	}

	// Create Openglview which implements IView (allows creations of different views)
	openGLView = new ModelViewController::OpenGLView(isFullHand, argv[1]);
	if(!openGLView)
	{
		std::printf("Failed at Initialization\n");
		releaseAll();
		return -1;
	}

	// When using sequence, change useSequence to true and apply sequencePath with sequence path
	bool useSequence = false;
	pxcCHAR* sequencePath = L"Insert Sequence Path Here";
	

	// Bind controller with model and view and start playing
	controller = new ModelViewController::HandsController(handModel,openGLView);
	if(!controller)
	{
		std::printf("Failed at Initialization\n");
		releaseAll();
		return -1;
	}

	rssdkHandler = new RssdkHandler(controller,handModel,openGLView);

	if(useSequence)
	{
		if(rssdkHandler->Init(isFullHand,sequencePath) == PXC_STATUS_NO_ERROR)
		{
			rssdkHandler->Start();
		}

		else
		{
			std::printf("Failed at Initialization\n");
			releaseAll();
			return -1;
		}
	}

	else
	{
		if(rssdkHandler->Init(isFullHand) == PXC_STATUS_NO_ERROR)
		{
			rssdkHandler->Start();
		}

		else
		{
			std::printf("Failed at Initialization\n");
			releaseAll();
			return -1;
		}
	}

	releaseAll();
    return 0;
}

void releaseAll()
{
	//delete all pointers
	if(handModel)
		delete handModel;
	if(openGLView)
		delete openGLView;
	if(controller)
		delete controller;
	if(rssdkHandler)
		delete rssdkHandler;	
}


