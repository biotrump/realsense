/*==============================================================================
Play Sound Example
Copyright (c), Firelight Technologies Pty, Ltd 2004-2016.

This example shows how to simply load and play multiple sounds, the simplest 
usage of FMOD. By default FMOD will decode the entire file into memory when it
loads. If the sounds are big and possibly take up a lot of RAM it would be
better to use the FMOD_CREATESTREAM flag, this will stream the file in realtime
as it plays.
==============================================================================*/
#include "fmod.hpp"
#include "common.h"
#include "play_sound.h"
//FMOD error 51 - Error initializing output device.
//This is possibly an issue with the AudioSrv service in Windows. 
//Go to the TaskManager. Go to the Services tab. 
//Find 'AudioSrv' in the list, right click and Restart it.
//http://forum.unity3d.com/threads/fmod-failed-to-initialize-error-initializing-output-device-on-wp8-device.266117/

static FMOD::System     *fmod_system;
static FMOD::Sound **fmod_sounds;
static FMOD::Channel    *channel = 0;
static void             *extradriverdata = 0;
static int notes_count;
char *notes[] = {
	"C.wav",     "D.wav", "E.wav",   "F.wav", "G.wav", "A.wav",     "B.wav",
	"C_u.wav",   "D_u.wav",   "E_u.wav","F_u.wav",   "G_u.wav","A_u.wav",   "B_u.wav",
   "C_u2.wav", "D_u2.wav",  "E_u2.wav", "F_u2.wav", "G_u2.wav", "A_u2.wav", "B_u2.wav"
   /*
   ,  "C_u3.wav",       "G_d.wav"
	"F_d.wav",     "A_d.wav",
	"B_d.wav",   "C_d.wav",   "D_d.wav",   "E_d.wav",
	"G_d2.wav", "A_d2.wav",  "B_d2.wav",
	*/
};


int play_sound()
{
    FMOD::Sound      *sound1, *sound2, *sound3;
    FMOD_RESULT       result;
    unsigned int      version;

	notes_count = sizeof(notes) / sizeof(char *);
	fmod_sounds = new FMOD::Sound *[notes_count];

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

	for (int i = 0; i < notes_count; i++) {
		result = fmod_system->createSound(Common_MediaPath(notes[i]), FMOD_DEFAULT, 0, fmod_sounds + i);
		ERRCHECK(result);
		if (result != FMOD_OK)
			return result;
	}

    /*
        Main loop
    */
    do
    {
        Common_Update();//capture keyboard

        if (Common_BtnPress(BTN_ACTION1))
        {
            result = fmod_system->playSound(fmod_sounds[KeyNote_C], 0, false, &channel);
            ERRCHECK(result);
        }

        if (Common_BtnPress(BTN_ACTION2))
        {
            result = fmod_system->playSound(fmod_sounds[KeyNote_D], 0, false, &channel);
            ERRCHECK(result);
        }

        if (Common_BtnPress(BTN_ACTION3))
        {
			result = fmod_system->playSound(fmod_sounds[KeyNote_E], 0, false, &channel);
            ERRCHECK(result);
        }

        result = fmod_system->update();
        ERRCHECK(result);

        {
            unsigned int ms = 0;
            unsigned int lenms = 0;
            bool         playing = 0;
            bool         paused = 0;
            int          channelsplaying = 0;

            if (channel)
            {
                FMOD::Sound *currentsound = 0;

                result = channel->isPlaying(&playing);
                if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
                {
                    ERRCHECK(result);
                }

                result = channel->getPaused(&paused);
                if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
                {
                    ERRCHECK(result);
                }

                result = channel->getPosition(&ms, FMOD_TIMEUNIT_MS);
                if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
                {
                    ERRCHECK(result);
                }
               
                channel->getCurrentSound(&currentsound);
                if (currentsound)
                {
                    result = currentsound->getLength(&lenms, FMOD_TIMEUNIT_MS);
                    if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
                    {
                        ERRCHECK(result);
                    }
                }
            }

            fmod_system->getChannelsPlaying(&channelsplaying, NULL);

            Common_Draw("==================================================");
            Common_Draw("Play Sound Example.");
            Common_Draw("Copyright (c) Firelight Technologies 2004-2016.");
            Common_Draw("==================================================");
            Common_Draw("");
            Common_Draw("Press %s to play a mono sound (drumloop)", Common_BtnStr(BTN_ACTION1));
            Common_Draw("Press %s to play a mono sound (jaguar)", Common_BtnStr(BTN_ACTION2));
            Common_Draw("Press %s to play a stereo sound (swish)", Common_BtnStr(BTN_ACTION3));
            Common_Draw("Press %s to quit", Common_BtnStr(BTN_QUIT));
            Common_Draw("");
            Common_Draw("Time %02d:%02d:%02d/%02d:%02d:%02d : %s", ms / 1000 / 60, ms / 1000 % 60, ms / 10 % 100, lenms / 1000 / 60, lenms / 1000 % 60, lenms / 10 % 100, paused ? "Paused " : playing ? "Playing" : "Stopped");
            Common_Draw("Channels Playing %d", channelsplaying);
        }

        Common_Sleep(50);
    } while (!Common_BtnPress(BTN_QUIT));

    /*
        Shut down
    */
	/*
    result = sound1->release();
    ERRCHECK(result);
    result = sound2->release();
    ERRCHECK(result);
    result = sound3->release();
    ERRCHECK(result);
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

    result = fmod_system->close();
    ERRCHECK(result);
    result = fmod_system->release();
    ERRCHECK(result);

    Common_Close();

    return 0;
}

int FMOD_Init(void)
{
	FMOD_RESULT       result;
	unsigned int      version;

	notes_count = sizeof(notes) / sizeof(char *);
	fmod_sounds = new FMOD::Sound *[notes_count];

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
		result = fmod_system->createSound(Common_MediaPath(notes[i]), FMOD_DEFAULT, 0, fmod_sounds + i);
		ERRCHECK(result);
		if (result != FMOD_OK)
			break;
	}
	return result;
}

int FMOD_ShutDown(void)
{
	FMOD_RESULT       result=FMOD_OK;
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

	result = fmod_system->close();
	ERRCHECK(result);
	result = fmod_system->release();
	ERRCHECK(result);

	Common_Close();
	return result;
}

int FMOD_NoteByDepth(float z)
{
	int depth = 0;
	if (z < 0.22f)
		depth = 0;
	else if (z < 0.27f)
		depth = 1;
	else if (z < 0.34f)
		depth = 2;
	else if (z < 0.438f)
		depth = 3;
	else if (z < 0.49f)
		depth = 4;
	else if (z < 0.55f)
		depth = 5;
	else if (z < 0.59f)
		depth = 6;
	else if (z < 0.62f)
		depth = 7;
	else if (z < 0.64f)
		depth = 8;
	else 
		depth = 9;
	return depth;
}

int FMOD_Play(int keynote)
{
	static int last_note = KeyNote_C;
	if (keynote < KeyNote_C)
		keynote = last_note;
	printf("\nnotes:%d\n", keynote);
	FMOD_RESULT result = fmod_system->playSound(fmod_sounds[keynote], 0, false, &channel);
	ERRCHECK(result);
	//Common_Sleep(500);
	last_note = keynote;
	return result;
}

int fmodKeyboardCB(unsigned char Key, int x, int y)
{
	int ret = 0;
	FMOD_RESULT       result;
	switch(Key)
	{
	case '1':
		result = fmod_system->playSound(fmod_sounds[KeyNote_C], 0, false, &channel);
		ERRCHECK(result);
		ret = 1;
		break;
	case '2':
		result = fmod_system->playSound(fmod_sounds[KeyNote_D], 0, false, &channel);
		ERRCHECK(result);
		ret = 1;
		break;
	case '3':
		result = fmod_system->playSound(fmod_sounds[KeyNote_E], 0, false, &channel);
		ERRCHECK(result);
		ret = 1;
		break;
	case '4':
		result = fmod_system->playSound(fmod_sounds[KeyNote_F], 0, false, &channel);
		ERRCHECK(result);
		ret = 1;
		break;
	case '5':
		result = fmod_system->playSound(fmod_sounds[KeyNote_G], 0, false, &channel);
		ERRCHECK(result);
		ret = 1;
		break;
	case '6':
		result = fmod_system->playSound(fmod_sounds[KeyNote_A], 0, false, &channel);
		ERRCHECK(result);
		ret = 1;
		break;
	case '7':
		result = fmod_system->playSound(fmod_sounds[KeyNote_B], 0, false, &channel);
		ERRCHECK(result);
		ret = 1;
		break;
	case '8':
		result = fmod_system->playSound(fmod_sounds[KeyNote_C_u], 0, false, &channel);
		ERRCHECK(result);
		ret = 1;
		break;
	default:
		break;
	}

	result = fmod_system->update();
	ERRCHECK(result);

	return ret;
}