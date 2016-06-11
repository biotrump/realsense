#pragma once
/*
@author thomas@life100.cc
*/
#ifdef  __cplusplus
extern  "C" {
#endif 

	int oglModelLoad(char path[]);
	void modelDisplay(void);
	int modelZooming(int zoomIn, float step);
	int modelKeyboardCB(unsigned char Key, int x, int y);

#define HARP_ROT_Y	(319)		//1760
#define HARP_SCALE	(2.0f)

#ifdef  __cplusplus
}
#endif 
