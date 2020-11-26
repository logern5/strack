#ifndef STRACK_API
#define STRACK_API
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
struct destinations getdests(cJSON *root);
char* getfingerprint(cJSON *root);
char* getinfo (struct query *q);
#endif
