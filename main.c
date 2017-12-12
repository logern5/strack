#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lib/curl/curl.h"
#include "lib/cjson/cJSONlib.h"
#include "lib/asciiworld/asciiworld.c"
#include "utils.c"
#include "api.c"
#define SLEEPTIME 5
#if defined(TIMEOFFSET)
int offset = TIMEOFFSET;
#elif !defined(AUTO_JSON_OFFSET)
int offset = 0;
#endif
#if defined(_WIN32) && !defined(_CYGWIN_)
#define clr() system("cls"); /*Clear screen on Windows using a cmd command*/
#else
#define clr() printf("\e[1;1H\e[2J"); /*POSIX clear screen, should work on Cygwin too*/
#endif
void showworld(float lat, float lng){
	FILE *f;
	f = fopen("pointfile.txt","w");
	if (f == NULL){
		fprintf(stderr,"Error opening file pointfile.txt\n");
	}
	rewind(f);
	char buf[63];
	snprintf(buf, 64, "points\n%f\t%f\n.",lat,lng);
	fwrite(buf, strlen(buf), 1, f);
	fclose(f);
	char *args[4];
	args[0] = "asciiworld";
	args[1] = "ne_110m_land.shp";
	args[2] = "pointfile.txt";
	args[3] = ""; /*Title*/
	worldmain(3,(char **)&args);
}
struct query init(){
	struct query q;
	struct MemoryStruct a = http("https://santa-api.appspot.com/info?client=web");
	cJSON *root = cJSON_Parse(a.memory);
	q.fingerprint = (char *)malloc(255*sizeof(char));
	strncpy(q.fingerprint,getfingerprint(root),255);
	cJSON_Delete(root);
	q.rand = (char *)malloc(20*sizeof(char));
	snprintf(q.rand,20,"%f",mathrand());
	q.client = "Web";
	q.language = "en";
	q.routeOffset = "0";
	q.streamOffset = "0";
	return q;
}
int main(void){
	#ifndef JFILE /*get responses from HTTP*/
	struct query q = init();
	char *resp = getinfo(&q);
	#endif
	#ifdef JFILE /*get response from a local JSON file*/
	FILE *file;
	file = fopen(JFILE,"r");
	if (file == NULL){
		fprintf(stderr,"Error opening JSON file\n");
		exit(-1);
	}
	fseek(file,0L,SEEK_END);
	int size = ftell(file);
	rewind(file);
	char *resp = (char *)malloc(size*sizeof(char));
	fread(resp,sizeof(char),size,(FILE *)file);
	fclose(file);
	#endif
	cJSON *respjson = cJSON_Parse(resp);
	struct destinations dests = getdests(respjson);
	cJSON_Delete(respjson);
	free(resp);
	#if defined(AUTO_JSON_OFFSET)
	long now = time(NULL);
	long depart = dests.dest[0].departure;
	int offset = (int)(now-depart);
	#endif
	int tim = (int)time(NULL)-offset;
	if (tim < (int)dests.dest[0].departure){
		fprintf(stderr,"It is not Christmas Eve yet!\n");
		fprintf(stderr,"Current POSIX time:%d\nDeparture POSIX time:%li\n", tim,dests.dest[0].departure);
		exit(-1);
	}
	char *status = "In transit";
	for(int i=0;i<dests.length;i++){
		status = "In transit";
		showworld(dests.dest[i].location.lat, dests.dest[i].location.lng);
		while (tim<dests.dest[i+1].arrival){
			showworld(dests.dest[i].location.lat, dests.dest[i].location.lng);
			struct tm *utc = gmtime((time_t *)&tim);
			/*TODO: Find percentalong=elapsedTime/totalTime(aka nextarriv-thisdepart)*/
			/*TODO: Interpolate position from percentalong*/
			printf("Last location:%s, Status:%s, UTC Time:%d-%d-%d %2d:%02d:%02d\n",dests.dest[i].city, status, (utc->tm_year)+1900, (utc->tm_mon)+1, utc->tm_mday, (utc->tm_hour)%24, utc->tm_min, utc->tm_sec);
			clr();
			tim = (int)time(NULL)-offset;
			sleep(SLEEPTIME);
		}
		status = "Landed";
		while (tim<dests.dest[i+1].departure){ /*we have arrived/landed*/
                        showworld(dests.dest[i+1].location.lat, dests.dest[i+1].location.lng);
			struct tm *utc = gmtime((time_t *)&tim);
			printf("Current location:%s, Status:%s, UTC Time:%d-%d-%d %2d:%02d:%02d\n",dests.dest[i+1].city, status,(utc->tm_year)+1900, (utc->tm_mon)+1, utc->tm_mday, (utc->tm_hour)%24, utc->tm_min, utc->tm_sec);
			clr();
                        tim = (int)time(NULL)-offset;
			sleep(SLEEPTIME);
		}
		clr();
	}
	free(status);
	return 0;
}
