#pragma once
int fmodKeyboardCB(unsigned char Key, int x, int y);
int FMOD_ShutDown(void);
int FMOD_Init(void);
int FMOD_Play(int keynote);
inline float distance(float x, float y, float z, float x1, float y1, float z1)
{
	float temp = (x - x1)*(x - x1) + (y - y1)*(y - y1) + (z - z1)*(z - z1);
	return sqrt(temp);
}

enum KEY_NOTE {
	KeyNote_C, KeyNote_D, KeyNote_E, KeyNote_F, KeyNote_G, KeyNote_A, KeyNote_B,
	KeyNote_C_u, KeyNote_D_u, KeyNote_E_u, KeyNote_F_u, KeyNote_G_u, KeyNote_A_u, KeyNote_B_u,
	KeyNote_C_u2, KeyNote_D_u2, KeyNote_E_u2, KeyNote_F_u2, KeyNote_G_u2, KeyNote_A_u2, KeyNote_B_u2
	/*,
	KeyNote_C_d, KeyNote_D_d, KeyNote_E_d, KeyNote_F_d, KeyNote_G_d, KeyNote_A_d, KeyNote_B_d,
	KeyNote_G_d2, KeyNote_A_d2, KeyNote_B_d2, KeyNote_C_u3*/
};
