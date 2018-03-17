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
long offset = TIMEOFFSET;
#elif !defined(AUTO_JSON_OFFSET)
long offset = 0;
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
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
	strncpy(q.fingerprint,"0",255); /*New API doesn't need fingerprint*/
	cJSON_Delete(root);
	q.rand = (char *)malloc(20*sizeof(char));
	snprintf(q.rand,20,"%f",mathrand());
	q.client = "Web";
	q.language = "en";
	q.routeOffset = "0";
	q.streamOffset = "0";
	return q;
}
int main(int argc, char **argv){
	char *resp = NULL;
	if(argc==1){
		struct query q = init();
		resp = getinfo(&q);
	}
	else if (strcmp(argv[1],"auto")==0){
		struct query q = init();
		resp = getinfo(&q);
	}
	else if(strcmp(argv[1],"json")==0){
		FILE *file;
		file = fopen("example.json","r");
		if (file == NULL){
			fprintf(stderr,"Error opening JSON file example.json\n");
			exit(-1);
		}
		fseek(file,0L,SEEK_END);
		int size = ftell(file);
		rewind(file);
		resp = (char *)malloc(size*sizeof(char));
		fread(resp,sizeof(char),size,(FILE *)file);
		fclose(file);
	}
	cJSON *respjson = cJSON_Parse(resp);
	struct destinations dests = getdests(respjson);
	cJSON_Delete(respjson);
	free(resp);
	long offset = 0L;
	if (argc>1){
		if((strcmp(argv[1],"json")==0)||(strcmp(argv[1],"auto")==0)){
			long now = time(NULL);
			long depart = dests.dest[0].departure;
			offset = now-depart;
		}
	}
	long tim = (long)time(NULL)-offset;
	if (tim < dests.dest[0].departure){
		fprintf(stderr,"It is not Christmas Eve yet!\n");
		fprintf(stderr,"Current POSIX time:%li\nDeparture POSIX time:%li\n", tim,dests.dest[0].departure);
		exit(-1);
	}
	for(int i=0;i<dests.length;i++){
		showworld(dests.dest[i].location.lat, dests.dest[i].location.lng);
		while (tim<dests.dest[i+1].arrival){
			long elapsedtime = tim-dests.dest[i].departure;
			long totaltime = dests.dest[i+1].arrival-dests.dest[i].departure;
			float percentalong = (float)elapsedtime/(float)totaltime;
			struct coords start;
			start.lat = dests.dest[i].location.lat;
			start.lng = dests.dest[i].location.lng;
			struct coords end;
			end.lat = dests.dest[i+1].location.lat;
			end.lng = dests.dest[i+1].location.lng;
			struct coords crd = slerp(start,end,percentalong);
			showworld(crd.lat,crd.lng);
			//struct tm *utc = gmtime((time_t *)&tim); /*Intentionally commented out*/
			int eta = (int)(dests.dest[i+1].arrival-tim);
			#if defined(_WIN32) || defined(__CYGWIN__)
			printf("\n");
			#endif
			printf("Last stop:%s,Next stop:%s,Arriving in:%02d:%02d\n",dests.dest[i].city,dests.dest[i+1].city,eta/60,eta%60);
			//printf("UTC Time:%d-%d-%d %2d:%02d:%02d\n",(utc->tm_year)+1900, (utc->tm_mon)+1, utc->tm_mday, (utc->tm_hour)%24, utc->tm_min, utc->tm_sec); /*Intentionally commented out*/
			sleep(SLEEPTIME);
			clr();
			tim = (long)time(NULL)-offset;
		}
		while (tim<dests.dest[i+1].departure){ /*we have arrived/landed*/
                        showworld(dests.dest[i+1].location.lat, dests.dest[i+1].location.lng);
			//struct tm *utc = gmtime((time_t *)&tim); /*Intentionally commented out*/
			int eta = (int)(dests.dest[i+1].departure-tim);
			if((i+2)>=dests.length){
				printf("Current stop:%s, the end. Press the ENTER key to exit.\n",dests.dest[i+1].city);
				getchar();
				exit(0);
			}
			else{
				#if defined(_WIN32) || defined (__CYGWIN__)
				printf("\n");
				#endif
				printf("Current stop:%s,Departing in:%02d:%02d\n",dests.dest[i+1].city,eta/60,eta%60);
			}
			sleep(SLEEPTIME);
			clr();
			tim = (long)time(NULL)-offset;
		}
		clr();
	}
	return 0;
}
