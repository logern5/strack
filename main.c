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
	fwrite(buf, strlen(buf), 1, f); /* Write to a temp file to pass to the world drawer */
	fclose(f);
	char *args[4];
	args[0] = "asciiworld";
	args[1] = "ne_110m_land.shp";
	args[2] = "pointfile.txt";
	args[3] = ""; /* Title */
	worldmain(3,(char **)&args); /* Draw the location on the screen */
}
struct query init(){ /* Prepare a query to the API */
	struct query q;
	struct MemoryStruct a = http("https://santa-api.appspot.com/info?client=web");
	cJSON *root = cJSON_Parse(a.memory);
	q.fingerprint = (char *)malloc(255*sizeof(char));
	strncpy(q.fingerprint,"0",255); /*New API doesn't need fingerprint*/
	cJSON_Delete(root);
	q.rand = (char *)malloc(20*sizeof(char)); /* The API needs a random value as a nonce */
	snprintf(q.rand,20,"%f",mathrand());
	q.client = "Web";
	q.language = "en";
	q.routeOffset = "0";
	q.streamOffset = "0";
	return q;
}
int main(int argc, char **argv){
	char *resp = NULL;
	if(argc==1){ /*No args to function, normal mode*/
		struct query q = init(); /* Create query to API */
		resp = getinfo(&q); /* Send query to the API, and get back route info */
	}
	else if (strcmp(argv[1],"auto")==0){
		struct query q = init();
		resp = getinfo(&q);
	}
	else if(strcmp(argv[1],"json")==0){ /* Read information from JSON file instead of API */
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
	cJSON *respjson = cJSON_Parse(resp); /* Parse the JSON file of route info from API or file */
	struct destinations dests = getdests(respjson); /* Parse the JSON structure of route info */
	cJSON_Delete(respjson);
	free(resp);
	long offset = 0L;
	if (argc>1){ /*Auto (time-adjust) mode, or reading from a JSON file mode */
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
	for(int i=0;i<dests.length;i++){ /* Show current location on map */
		showworld(dests.dest[i].location.lat, dests.dest[i].location.lng);
		while (tim<dests.dest[i+1].arrival){ /* If in transit */
			long elapsedtime = tim-dests.dest[i].departure; /*elapsed time is current time - departure time */
			long totaltime = dests.dest[i+1].arrival-dests.dest[i].departure; /*total tiem between departure and arrival */
			float percentalong = (float)elapsedtime/(float)totaltime;
			struct coords start;
			start.lat = dests.dest[i].location.lat;
			start.lng = dests.dest[i].location.lng;
			struct coords end;
			end.lat = dests.dest[i+1].location.lat;
			end.lng = dests.dest[i+1].location.lng;
			struct coords crd = slerp(start,end,percentalong); /* Find location based on percent along route */
			showworld(crd.lat,crd.lng); /*Show location along route*/
			int eta = (int)(dests.dest[i+1].arrival-tim);
			#if defined(_WIN32) || defined(__CYGWIN__)
			printf("\n");
			#endif
			printf("Last stop:%s,Next stop:%s,Arriving in:%02d:%02d\n", /* Print info about stops and time to screen */
			  dests.dest[i].city,
			  dests.dest[i+1].city,
			  eta/60,
			  eta%60
			);
			sleep(SLEEPTIME); /* Wait to update location */
			clr(); /* Clear screen */
			tim = (long)time(NULL)-offset; /* Update current time */
		}
		while (tim<dests.dest[i+1].departure){ /* If landed */
			showworld(dests.dest[i+1].location.lat, dests.dest[i+1].location.lng);
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
			sleep(SLEEPTIME); /*Wait to update location */
			clr(); /* Clear screen */
			tim = (long)time(NULL)-offset; /* Update current time */
		}
		clr();
	}
	return 0;
}
