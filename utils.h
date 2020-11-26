#ifndef STRACK_UTILS
#define STRACK_UTILS
struct coords{
	float lat;
	float lng;
};

struct coords slerp(struct coords start, struct coords end, float percentalong);
struct coords pslerp(struct coords start, struct coords end, float percentalong);
float mathrand();
#endif
