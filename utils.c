#include <stdlib.h>
#ifndef UTILS
#define UTILS
struct coords{
	float lat;
	float lng;
};
struct coords interpolate(struct coords start, struct coords end, float percentalong){
	//float x = x1+(x2-x1)*percentalong; /*x1+((x2-x1)*percentalong)*/
	//float y = y1+(y2-y1)*percentalong;
	float x = start.lat+(end.lat-start.lat)*percentalong;
	float y = start.lng+(end.lng-start.lng)*percentalong;
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
