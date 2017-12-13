#include <stdlib.h>
#include <math.h>
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
struct coords slerp(struct coords start, struct coords end, float percentalong){
	/*Convert to radians*/
	start.lat *= 0.0175;
	start.lng *= 0.0175;
	end.lat *= 0.0175;
	end.lng *= 0.0175;
	int d = 1; /*Placeholder distance that should work*/
	//int d = gcdist(start,end);
	float a = sin((1-percentalong)*d)/sin(d);
	float b = sin(percentalong*d)/sin(d);
	float x = a*cos(start.lat)*cos(start.lng)+b*cos(end.lat)*cos(end.lng);
	float y = a*cos(start.lat)*sin(start.lng)+b*cos(end.lat)*sin(end.lng);
	float z = a*sin(start.lat)+b*sin(end.lat);
	struct coords c;
	c.lat = (atan2(z,sqrt(x*x+y*y)))/0.0175;
	c.lng = (atan2(y,x))/0.0175;
	//printf("Lat: %f, Lng: %f",c.lat,c.lng);
	//getchar();
	return c;
}
struct coords pslerp(struct coords start, struct coords end, float percentalong){
	/*Convert to radians*/
	start.lat *= 0.0175;
	start.lng *= 0.0175;
	end.lat *= 0.0175;
	end.lng *= 0.0175;
	//printf("Rads: %f,%f,%f,%f",start.lat,start.lng,end.lat,end.lng);
	float x = start.lat+((end.lng-start.lng)*cos((start.lng+end.lng)/2)*percentalong);
	float y = start.lng+(end.lat-start.lat)*percentalong;
	struct coords c;
	/*Convert back to degrees*/
	c.lat = x/0.0175;
	c.lng = y/0.0175;
	//printf("Lat:%f,Lng:%f",x,y);
	//getchar();
	return c;
}
float mathrand(){
	srand((unsigned int)time(NULL));
	float scale = rand()/(float)RAND_MAX;
	return scale;
}
#endif
