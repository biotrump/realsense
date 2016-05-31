#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <stdio.h>
#include <iostream>

#include "tresource.h"
#include "TMIDI_CTRL.h"

HWND hTMIDI = NULL;
HWND hTempo = NULL, hVelocity = NULL;
HWND tmidi_init(void)
{
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
	hTMIDI = FindWindow(NULL, TEXT("TMIDI: Tom's MIDI Player"));
	//HWND hwnd = FindWindow(NULL, TEXT("Tempo"));
	if (hTMIDI) {
		printf("hTMIDI=0x%x\n", hTMIDI);
		hTempo = FindWindowEx(hTMIDI, NULL, NULL, TEXT("Tempo"));
		printf("hTempo=0x%x\n", hTempo);
		SendDlgItemMessage(hTMIDI, IDC_TEMPO_SLIDER, TBM_SETPOS, TRUE, 180);//default 180
		SendDlgItemMessage(hTMIDI, IDC_TEMPO_SLIDER, TBM_SETTICFREQ, 500 / 10, 0);

		hVelocity = FindWindowEx(hTMIDI, NULL, NULL, TEXT("Velocity"));
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
	}
	else {
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
	////////////////////////////////
	return hTMIDI;
}

void tmidi_tempo(int pos)
{
	//SendDlgItemMessage(hTMIDI, IDC_TEMPO_SLIDER, TBM_SETPOS, TRUE, pos);//default 180
	//SendDlgItemMessage(hTMIDI, IDC_TEMPO_SLIDER, TBM_SETTICFREQ, 500 / 10, 0);//???
	//simulate the tempo change scroll
	PostMessage(hTMIDI, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, pos), (LPARAM)hTempo);
}

void tmidi_velocity(int pos)
{
	//SendDlgItemMessage(hTMIDI, IDC_VELOCITY_SLIDER, TBM_SETPOS, TRUE, pos);
	//SendDlgItemMessage(hTMIDI, IDC_VELOCITY_SLIDER, TBM_SETTICFREQ, 500 / 10, 0);
	//PostMessage(hTMIDI, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)pos);
	//PostMessage(hTMIDI, WM_COMMAND, MAKEWPARAM(IDC_VELOCITY_SLIDER, WM_HSCROLL), (LPARAM)GetDlgItem(hTMIDI, IDC_VELOCITY_SLIDER));
	//simulate the velocity change scroll
	PostMessage(hTMIDI, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION,10), (LPARAM)hVelocity);
	//Invalidate();
	//UpdateWindow(hTMIDI);
	tmidi_pause(true);//test...
}

void tmidi_play(bool fclicked)
{
	PostMessage(hTMIDI, WM_COMMAND, MAKEWPARAM(IDC_PLAY, BN_CLICKED), (LPARAM)GetDlgItem(hTMIDI, IDC_PLAY));
}

void tmidi_pause(bool fclicked)
{
	PostMessage(hTMIDI, WM_COMMAND, MAKEWPARAM(IDC_PAUSE, BN_CLICKED), (LPARAM)GetDlgItem(hTMIDI, IDC_PAUSE));
}

void tmidi_stop(bool fclicked)
{
	PostMessage(hTMIDI, WM_COMMAND, MAKEWPARAM(IDC_STOP, BN_CLICKED), (LPARAM)GetDlgItem(hTMIDI, IDC_STOP));
}