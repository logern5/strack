#include "lib/curl/curl.h"
#include "lib/cjson/cJSONlib.h"
#ifndef API
#define API
struct query{
	char *rand;
	char *client;
	char *language;
	char *fingerprint;
	char *routeOffset;
	char *streamOffset;
};
struct locationstruct{
	float lat;
	float lng;
};
struct destination{
	char *id;
	long arrival;
	long departure;
	long population;
	long presentsDelivered;
	char *city;
	struct locationstruct location;
	/*Details could also go here, but we do not need it*/
};
struct destinations{
	struct destination *dest;
	int length;
};
struct destinations getdests(cJSON *root){
	cJSON *jsondests = cJSON_GetObjectItemCaseSensitive(root,"destinations");
	int length = cJSON_GetArraySize(jsondests);
	struct destinations dests;
	dests.dest = calloc(length,sizeof(struct destination));
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
	for (int i=0;i<length;i++){
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
		dests.dest[i].arrival = (long)(jsonarrival->valuedouble/1000); //Timestamps are in ms
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
char* getfingerprint(cJSON *root){
	cJSON *fingerprint = cJSON_GetObjectItemCaseSensitive(root,"fingerprint");
	return fingerprint->valuestring;
}
char* getinfo (struct query *q){
	char *request = (char *)malloc(512*sizeof(char));
	snprintf(request,512,"https://santa-api.appspot.com/info?rand=%s&client=web&language=%s&fingerprint=%s&routeOffset=%s&streamOffset=%s", q->rand, q->client, q->fingerprint, q->routeOffset, q->streamOffset);
	struct MemoryStruct a = http(request);
	free(request);
	return a.memory;
}
#endif
