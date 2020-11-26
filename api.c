#include "lib/curl/curllib.h"
#include "lib/cjson/cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct query{ /* Query to send to API */
	char *rand;
	char *client;
	char *language;
	char *fingerprint;
	char *routeOffset;
	char *streamOffset;
};
struct locationstruct{ /* Simple xy coordinate struct */
	float lat;
	float lng;
};
struct destination{ /* Information about each destination from the JSON api */
	char *id;
	long arrival;
	long departure;
	long population;
	long presentsDelivered;
	char *city;
	struct locationstruct location;
	/*Details (other facts about the location) could also go here, but we do not need it*/
};
struct destinations{ /* array of struct destination */
	struct destination *dest;
	int length;
};
struct destinations getdests(cJSON *root){ /* put JSON data about destinations into C struct */
	cJSON *jsondests = cJSON_GetObjectItemCaseSensitive(root,"destinations");
	int length = cJSON_GetArraySize(jsondests);
	struct destinations dests;
	dests.dest = calloc(length,sizeof(struct destination)); /* Create an array of destinations */
	dests.length = length;
	cJSON *jsondest = NULL;
	cJSON *jsonid = NULL;
	cJSON *jsonarrival = NULL;
	cJSON *jsondeparture = NULL;
	cJSON *jsonpopulation = NULL;
	cJSON *jsonpresentsDelivered = NULL;
	cJSON *jsoncity = NULL;
	cJSON *jsonlocation = NULL;
	cJSON *jsonlat = NULL;
	cJSON *jsonlng = NULL;
	for (int i=0;i<length;i++){ /* For each destination in the JSON data, put info into a struct destination */
		jsondest = cJSON_GetArrayItem(jsondests,i);
		jsonid = cJSON_GetObjectItemCaseSensitive(jsondest,"id");
		jsonarrival = cJSON_GetObjectItemCaseSensitive(jsondest,"arrival");
		jsondeparture = cJSON_GetObjectItemCaseSensitive(jsondest,"departure");
		jsonpopulation = cJSON_GetObjectItemCaseSensitive(jsondest,"population");
		jsonpresentsDelivered = cJSON_GetObjectItemCaseSensitive(jsondest,"presentsDelivered");
		jsoncity = cJSON_GetObjectItemCaseSensitive(jsondest,"city");
		jsonlocation = cJSON_GetObjectItemCaseSensitive(jsondest,"location");
		jsonlat = cJSON_GetObjectItemCaseSensitive(jsonlocation,"lat");
		jsonlng = cJSON_GetObjectItemCaseSensitive(jsonlocation,"lng");
		dests.dest[i].id = (char *)malloc(63*sizeof(char));
		strncpy(dests.dest[i].id, jsonid->valuestring,63);
		dests.dest[i].arrival = (long)(jsonarrival->valuedouble/1000); /* Timestamps are in ms */
		dests.dest[i].departure = (long)(jsondeparture->valuedouble/1000);
		dests.dest[i].population = (long)jsonpopulation->valuedouble;
		dests.dest[i].presentsDelivered = (long)jsonpresentsDelivered->valuedouble;
		dests.dest[i].city = (char *)malloc(63*sizeof(char));
		strncpy(dests.dest[i].city, jsoncity->valuestring,63);
		dests.dest[i].location.lat = (float)jsonlat->valuedouble;
		dests.dest[i].location.lng = (float)jsonlng->valuedouble;
	}
	return dests;
}
char* getfingerprint(cJSON *root){ /* Old API (pre Dec 2017) needed a fingerprint, not needed anymore */
	cJSON *fingerprint = cJSON_GetObjectItemCaseSensitive(root,"fingerprint");
	return fingerprint->valuestring;
}
char* getinfo (struct query *q){ /* get JSON route/destination data from the API */
	char *request = (char *)malloc(512*sizeof(char));
	snprintf(request,512,"https://santa-api.appspot.com/info?rand=%s&client=web&language=%s&fingerprint=%s&routeOffset=%s&streamOffset=%s", q->rand, q->client, q->fingerprint, q->routeOffset, q->streamOffset);
	struct MemoryStruct a = http(request);
	free(request);
	/* Free memory inside of the query */
	free(q->fingerprint);
	free(q->rand);
	cJSON *root = cJSON_Parse(a.memory);
	if(root == NULL){
		fprintf(stderr, "cJSON_Parse error\n");
		exit(-1);
	}
	cJSON *route = cJSON_GetObjectItemCaseSensitive(root,"route");
	char *url = (char *)malloc(256*sizeof(char));
	if(url == NULL){
		fprintf(stderr, "malloc() returned null\n");
		exit(-1);
	}
	strncpy(url,route->valuestring,256);
	cJSON_Delete(root);
	free(a.memory);
	struct MemoryStruct b = http(url);
	return b.memory;
}
