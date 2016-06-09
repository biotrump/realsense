#ifndef FMOD_EXAMPLES_COMMON_H
#define FMOD_EXAMPLES_COMMON_H

#include "common_platform.h"
#include "fmod.h"

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

#define NUM_COLUMNS 50
#define NUM_ROWS 25

#ifndef Common_Sin
    #define Common_Sin sin
#endif

#ifndef Common_snprintf
    #define Common_snprintf snprintf
#endif

#ifndef Common_vsnprintf
    #define Common_vsnprintf vsnprintf
#endif

enum Common_Button
{
    BTN_ACTION1,
    BTN_ACTION2,
    BTN_ACTION3,
    BTN_ACTION4,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_UP,
    BTN_DOWN,
    BTN_MORE,
    BTN_QUIT
};
enum KEY_NOTE {
	KeyNote_A, KeyNote_B, KeyNote_C, KeyNote_D,
	KeyNote_E, KeyNote_F_d, KeyNote_G_d2, KeyNote_A_d,
	KeyNote_B_d, KeyNote_C_d, KeyNote_D_d, KeyNote_E_d,
	KeyNote_F_u, KeyNote_G_u, KeyNote_A_d2, KeyNote_B_d2,
	KeyNote_C_u, KeyNote_D_u, KeyNote_E_u, KeyNote_F_u2,
	KeyNote_G_u2, KeyNote_A_u, KeyNote_B_u, KeyNote_C_u2,
	KeyNote_D_u2, KeyNote_E_u2, KeyNote_G, KeyNote_A_u2,
	KeyNote_B_u2, KeyNote_C_u3, KeyNote_F, KeyNote_G_d
};

/* Cross platform functions (common) */
void Common_Fatal(const char *format, ...);
void Common_Draw(const char *format, ...);

void ERRCHECK_fn(FMOD_RESULT result, const char *file, int line);
#define ERRCHECK(_result) ERRCHECK_fn(_result, __FILE__, __LINE__)
#define Common_Max(_a, _b) ((_a) > (_b) ? (_a) : (_b))
#define Common_Min(_a, _b) ((_a) < (_b) ? (_a) : (_b))
#define Common_Clamp(_min, _val, _max) ((_val) < (_min) ? (_min) : ((_val) > (_max) ? (_max) : (_val)))

/* Functions with platform specific implementation (common_platform) */
void Common_Init(void **extraDriverData);
void Common_Close();
void Common_Update();
void Common_Sleep(unsigned int ms);
void Common_Exit(int returnCode);
void Common_DrawText(const char *text);
void Common_LoadFileMemory(const char *name, void **buff, int *length);
void Common_UnloadFileMemory(void *buff);
void Common_Format(char *buffer, int bufferSize, const char *formatString...);
bool Common_BtnPress(Common_Button btn);
bool Common_BtnDown(Common_Button btn);
const char *Common_BtnStr(Common_Button btn);
const char *Common_MediaPath(const char *fileName);
const char *Common_WritePath(const char *fileName);
void Common_Mutex_Create(Common_Mutex *mutex);
void Common_Mutex_Destroy(Common_Mutex *mutex);
void Common_Mutex_Enter(Common_Mutex *mutex);
void Common_Mutex_Leave(Common_Mutex *mutex);

#endif
