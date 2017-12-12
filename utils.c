#include <stdlib.h>
#ifndef UTILS
#define UTILS
struct coords{
	float lat;
	float lng;
};
struct coords interpolate(int x1, int x2, int y1, int y2,int percentalong){
	int x = x1+(x2-x1)*percentalong;
	int y = y1+(y2-y1)*percentalong;
	struct coords c;
	c.lat = x;
	c.lng = y;
	return c;
}
float mathrand(){
	srand((unsigned int)time(NULL));
	float scale = rand()/(float)RAND_MAX;
	return scale;
}
#endif
