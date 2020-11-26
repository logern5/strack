#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
/* For sleep() function */
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "lib/curl/curllib.h"
#include "lib/cjson/cJSON.h"
#include "lib/asciiworld/asciiworld.h"
#include "utils.h"
#include "api.h"

#define SLEEPTIME 5
#if defined(TIMEOFFSET) /* Compile-time time offset for testing and debugging purposes */
longoffset = TIMEOFFSET
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
		exit(-1);
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
	args[3] = "Title"; /* Title */
	worldmain(3,(char **)&args); /* Draw the location on the screen */
}

struct query init(){ /* Prepare a query to the API */
	struct query q;
	struct MemoryStruct a = http("https://santa-api.appspot.com/info?client=web");
	cJSON *root = cJSON_Parse(a.memory);
	q.fingerprint = (char *)malloc(255*sizeof(char));
	if(q.fingerprint == NULL){
		fprintf(stderr, "malloc() returned null\n");
		exit(-1);
	}
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

void showhelp(char *progname){
	printf("Usage:\n");
	printf("%s [option] [file]\n", progname);
	printf("Options:\n");
	printf("auto: Runs in automatic time offset mode, so the program will work on days other than December 24th.\n");
	printf("autojson [file]: Runs in automatic time offset mode, but reads location data from a JSON file.\n");
	printf("json [file]: Run without a time offset, but reads location data from a JSON file.\n");
	printf("\nWithout any arguments, %s will use data from the Web API (which doesn't work as of Nov 2020), and\n", progname);
	printf("the program will only run on December 24th.\n");
}

int main(int argc, char **argv){
	printf("argc=%d\n",argc);
	char *resp = NULL;
	if(argc==1){ /*No args to function, normal/online mode*/
		struct query q = init(); /* Create query to API */
		resp = getinfo(&q); /* Send query to the API, and get back route info */
	}
	else if ((strcmp(argv[1], "--help")==0)|| (strcmp(argv[1],"-h") == 0)){
		showhelp(argv[0]);
	}
	else if (strcmp(argv[1],"auto")==0){ /* Auto mode, sets a time offset. Offset is handled later. */
		struct query q = init(); /* Format a query */
		resp = getinfo(&q); /* Send query to API and get back a JSON string in resp*/
	}
	else if((strcmp(argv[1],"json")==0 || strcmp(argv[1],"autojson") == 0) && argc > 2){ /* Read information from JSON file instead of web API */
		FILE *file;
		file = fopen(argv[2],"r");
		if (file == NULL){
			fprintf(stderr,"Error opening JSON file %s\n", argv[2]);
			exit(-1);
		}
		fseek(file,0L,SEEK_END);
		int size = ftell(file);
		rewind(file);
		resp = (char *)malloc(size*sizeof(char));
		fread(resp,sizeof(char),size,(FILE *)file); /* Read JSON string from file into resp */
		fclose(file);
	}
	else{
		printf("we are here\n");
		showhelp(argv[0]); /* Unknown option, so show the help */
	}
	cJSON *respjson = cJSON_Parse(resp); /* Parse the JSON file of route info from API or file */
	struct destinations dests = getdests(respjson); /* Put the JSON data into a struct */
	cJSON_Delete(respjson);
	free(resp);
	long offset = 0L;
	if (argc>1){ /*Auto (time-adjust) mode, or reading from a JSON file mode */
		if((strcmp(argv[1],"autojson")==0) || (strcmp(argv[1],"auto")==0)){
			long now = time(NULL);
			long depart = dests.dest[0].departure;
			offset = now - depart;
		}
	}
	long tim = (long)time(NULL) - offset; /* Current absolute time, offsetted */
	if (tim < dests.dest[0].departure){
		fprintf(stderr,"It is not Christmas Eve yet!\n");
		fprintf(stderr,"Current POSIX time:%li\nDeparture POSIX time:%li\n", tim,dests.dest[0].departure);
		exit(-1);
	}
	for(int i=0;i<dests.length;i++){ /* Show current location on map */
		showworld(dests.dest[i].location.lat, dests.dest[i].location.lng);
		while (tim<dests.dest[i+1].arrival){ /* If in transit, not at a destination */
			long elapsedtime = tim-dests.dest[i].departure; /*elapsed time is current time - departure time */
			long totaltime = dests.dest[i+1].arrival-dests.dest[i].departure; /*total time between departure and arrival */
			float percentalong = (float)elapsedtime/(float)totaltime; /* How far are we along the route between the 2 points, assuming constant speed? */
			struct coords start; /* Last destination */
			start.lat = dests.dest[i].location.lat;
			start.lng = dests.dest[i].location.lng;
			struct coords end; /* Next destination */
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
		while (tim<dests.dest[i+1].departure){ /* If landed at a destination */
			showworld(dests.dest[i+1].location.lat, dests.dest[i+1].location.lng);
			int eta = (int)(dests.dest[i+1].departure-tim);
			if((i+2)>=dests.length){ /* Index+2 is more than the length of the destination, meaning we are at the end */
				printf("Current stop:%s, the end. Press the ENTER key to exit.\n",dests.dest[i+1].city);
				getchar();
				exit(0);
			}
			else{ /* Else, we are not at the end, continue along as normal, print a message about the current stop */
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
