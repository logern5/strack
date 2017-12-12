#include <stdlib.h>
#ifndef UTILS
#define UTILS
struct coords{
	float lat;
	float lng;
};
struct coords interpolate(int x1, int x2, int y1, int y2,float percentalong){
	float x = x1+(x2-x1)*percentalong;
	float y = y1+(y2-y1)*percentalong;
	struct coords c;
	c.lat = x;
	c.lng = y;
	return c;
}
/*float percentalong(int src, int dest){
	int dist = dest-src;
	return 0.0F;
}*/
float mathrand(){
	srand((unsigned int)time(NULL));
	float scale = rand()/(float)RAND_MAX;
	return scale;
}
#endif
