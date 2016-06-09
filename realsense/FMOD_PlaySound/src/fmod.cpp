#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <string>

#include "fmod.hpp"
#include "common.h"
//FMOD error 51 - Error initializing output device.
//This is possibly an issue with the AudioSrv service in Windows. 
//Go to the TaskManager. Go to the Services tab. 
//Find 'AudioSrv' in the list, right click and Restart it.
//http://forum.unity3d.com/threads/fmod-failed-to-initialize-error-initializing-output-device-on-wp8-device.266117/

using namespace std;

char *notes[] = {
"A.wav",     "B.wav",     "C.wav",     "D.wav",
"E.wav",     "F_d.wav",   "G_d2.wav",  "A_d.wav",   
"B_d.wav",   "C_d.wav",   "D_d.wav",   "E_d.wav",   
"F_u.wav",   "G_u.wav",	  "A_d2.wav",  "B_d2.wav",  
"C_u.wav",   "D_u.wav",   "E_u.wav",   "F_u2.wav",  
"G_u2.wav",	 "A_u.wav",   "B_u.wav",   "C_u2.wav",  
"D_u2.wav",  "E_u2.wav",  "G.wav",	   "A_u2.wav",  
"B_u2.wav",  "C_u3.wav",  "F.wav",     "G_d.wav"
};

static FMOD::System     *fmod_system;
//FMOD::Sound      *sound1, *sound2, *sound3;
//vector <FMOD::Sound *> fmod_sounds;
static FMOD::Sound **fmod_sounds;
static void   *extradriverdata = 0;

int notes_count;

FMOD::System* &FMOD_system(void)
{
	return fmod_system;
}

FMOD_RESULT fmod_init(void)
{
	
	//FMOD::System     *system;
	//FMOD::Sound      *sound1, *sound2, *sound3;
	//vector <FMOD::Sound *> fmod_sounds;
	//FMOD::Channel    *channel = 0;
	FMOD_RESULT       result;
	unsigned int      version;

	fmod_sounds = new FMOD::Sound *[notes_count];
	notes_count = sizeof(notes) / sizeof(char *);

	Common_Init(&extradriverdata);

	/*
	Create a System object and initialize
	*/
	result = FMOD::System_Create(&fmod_system);
	ERRCHECK(result);

	result = fmod_system->getVersion(&version);
	ERRCHECK(result);

	if (version < FMOD_VERSION)
	{
		Common_Fatal("FMOD lib version %08x doesn't match header version %08x", version, FMOD_VERSION);
	}

	result = fmod_system->init(32, FMOD_INIT_NORMAL, extradriverdata);
	ERRCHECK(result);

	for (int i = 0; i < notes_count; i++) {
		result = fmod_system->createSound(Common_MediaPath(notes[i]), FMOD_DEFAULT, 0, fmod_sounds+i);
		ERRCHECK(result);
		if (result != FMOD_OK)
			return result;
	}
#if 0
	result = fmod_system->createSound(Common_MediaPath("drumloop.wav"), FMOD_DEFAULT, 0, &sound1);
	ERRCHECK(result);

	result = sound1->setMode(FMOD_LOOP_OFF);    /* drumloop.wav has embedded loop points which automatically makes looping turn on, */
	ERRCHECK(result);                           /* so turn it off here.  We could have also just put FMOD_LOOP_OFF in the above CreateSound call. */

	result = fmod_system->createSound(Common_MediaPath("jaguar.wav"), FMOD_DEFAULT, 0, &sound2);
	ERRCHECK(result);

	result = fmod_system->createSound(Common_MediaPath("swish.wav"), FMOD_DEFAULT, 0, &sound3);
	ERRCHECK(result);
#endif

	return result;
}

FMOD_RESULT fmod_deinit(void)
{
	FMOD_RESULT       result;
	/*
	Shut down
	*/
	for (int i = 0; i < notes_count; i++) {
		if (fmod_sounds[i]) {
			result = fmod_sounds[i]->release();
			//fmod_sounds[i] = NULL;
			ERRCHECK(result);
			if (result != FMOD_OK)
				return result;
		}
	}
	delete[]fmod_sounds;
	if (fmod_system) {
		result = fmod_system->close();
		ERRCHECK(result);
		result = fmod_system->release();
		ERRCHECK(result);
		fmod_system = NULL;
	}
	Common_Close();
	return result;
}

FMOD_RESULT fmod_sysupdate(void)
{
	FMOD_RESULT result = FMOD_OK;
	if (fmod_system) {
		result = fmod_system->update();
		ERRCHECK(result);
	}
	return result;
}

FMOD_RESULT fmod_play(KEY_NOTE key_note, FMOD::Channel * &channel )
{
	FMOD_RESULT result= FMOD_OK;
	if (fmod_system) {
		result = fmod_system->playSound(fmod_sounds[key_note], 0, false, &channel);
		ERRCHECK(result);
		//FMOD_OK
	}
	return result;
}