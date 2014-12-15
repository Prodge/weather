#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <curl/curl.h>
#include <getopt.h>

struct string {
    char *ptr;
    size_t len;
};

struct coords {
    char lat[4];
    char lon[4];
};

/*Initializes the string structure*/
void init_string(struct string *s) {
    s->len = 0;
    s->ptr = malloc(s->len+1);
    if (s->ptr == NULL) {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
}

/*writing function for copying the curl output to the given string*/
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
    size_t new_len = s->len + size*nmemb;
    s->ptr = realloc(s->ptr, new_len+1);
    if (s->ptr == NULL) {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(s->ptr+s->len, ptr, size*nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;
    return size*nmemb;
}

/*Returns xml as a string from the given url*/
struct string scrape(char *url, struct string s){
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if(curl) {
        init_string(&s);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK){
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
    return s;
}

/*Returns the value from the xml that is pointed to by the passed variables*/
char *getValue(char *class, char *type, int classLen, int typeLen, struct string s){
    int pos = 0;
    //searching for class
    for(int i=0; i<s.len; i++){
        if(s.ptr[i] == '<'){
            bool match = true;
            for(int x=0; x<classLen; x++){
                if(s.ptr[i+x+1] != class[x]){
                    match = false;
                    break;
                }
            }
            if(match){
                pos = i+classLen;
                break;
            }
        } 
    }
    if(pos == 0){
        fprintf(stderr, "Could not find given class in function getValue()\n");
        exit( EXIT_FAILURE );
    }
    //checking for type
    for(int i=pos; i<s.len; i++){
        if(s.ptr[i] == '>'){
            fprintf(stderr, "Could not find given type in function getValue()\n");
            exit( EXIT_FAILURE );
        }
        bool match = true;
        for(int x=0; x<typeLen; x++){
            if(s.ptr[i+x] != type[x]){
                match = false;
                break;
            }
        }
        if(match){
            pos = i + typeLen + 2; //set pos to the start of the value
            break;
        }
    }
    //extracting value
    int i = 0;
    char *value;
    value = (char *) malloc(1);
    while(s.ptr[pos+i] != '"'){
        value = (char *) realloc(value, i+1);
        value[i] = s.ptr[pos+i];
        i++;
    }
    value[i] = '\0';
    return value;
}

/*Returns a coords stucture from a string of the form "lat,lon" */
struct coords getCoords(char *s){
    struct coords coord;
    int seperator = 0;
    int lenS = strlen(s);
    /*figuring out where the ',' is for*/
    for(int i=0; i<lenS; i++){
        if(s[i] == ','){
            seperator = i;
            break;
        }
    }
    if(seperator == 0){
        fprintf(stderr, "Invalid Coordinates, Could not find ','\n");
        exit( EXIT_FAILURE );
    }
    /*initialize coords to 000*/
    for(int i=0; i<3; i++){
        coord.lat[i]='0';
        coord.lon[i]='0';
    }
    /*copying the coords into the struct*/
    int count = 2;
    for(int i=seperator-1; i>=0; i--){
        coord.lat[count] = s[i];
        count--;
    }
    count = 2;
    for(int i=lenS-1; i>seperator; i--){
        coord.lon[count] = s[i];
        count--;
    }
    /*Terminating strings with null bytes*/
    char *null = "\0";
    coord.lat[3] = null[0];
    coord.lon[3] = null[0];
    return coord;
}

/*Returns an openweathermap url for the given city*/
char *getUrlCity(char *url, char *city){
    char *pre = "http://api.openweathermap.org/data/2.5/weather?q=";
    char *post = "&mode=xml";
    int preLen = strlen(pre);
    int cityLen = strlen(city);
    int postLen = strlen(post);
    url = (char *) malloc(preLen + cityLen + postLen + 1);
    strcat(url, pre);
    strcat(url, city);
    strcat(url, post);
    //Appending a null byte at the end of the string
    char *null = "\0";
    url[strlen(url)] = null[0];
    return url;
}

/*Returns an openweathermap url with the given coordinates*/
char *getUrlCoords(char *url, struct coords coord){
    char *pre = "http://api.openweathermap.org/data/2.5/weather?lat=";
    char *mid = "&lon=";
    char *post = "&mode=xml";
    url = (char *) malloc(strlen(pre) + strlen(coord.lat) + strlen(mid) + strlen(coord.lon) + strlen(post));
    strcat(url, pre);
    strcat(url, coord.lat);
    strcat(url, mid);
    strcat(url, coord.lon);
    strcat(url, post);
    //Appending a null byte at the end of the string
    char *null = "\0";
    url[strlen(url)] = null[0];
    return url;
}

/*Allocates space for the given string, copys the given string to the new location and returns it*/
char *malStrCpy(char *s){
    char *dest;
    int sLen = strlen(s);
    dest = (char *) malloc(sLen + 1);
    for(int i=0; i<sLen; i++){
        dest[i] = s[i];
    }
    char *null = "\0";
    dest[sLen] = null[0];
    return dest;
}

int main(int argc, char **argv){
    char *city;
    char *url = ""; 
    int subscribe = 0; // set to 0 for no subscribe
    int localType = 0; // set to 0 for inadequate input, 1 for city, 2 for coords
    int c;
    struct coords coord;
    while((c = getopt(argc, argv, "c:C:s:")) != -1){
        switch(c){
            case 's':
                subscribe = atoi(optarg);
                printf("Subscribe: %i\n", subscribe);
                break;
            case 'c':
                city = malStrCpy(optarg);
                printf("City: %s\n", city);
                localType = 1;
                url = getUrlCity(url, city);
                break;
            case 'C':
                coord = getCoords(optarg);
                printf("Optarg: %s\n", optarg);
                printf("%s  :  %s\n", coord.lat, coord.lon);
                url = getUrlCoords(url, coord);
                localType = 2;
                break;
        }
    }

    if(localType == 0){ //Reciecieved no location information
        fprintf(stderr, "You did not enter enough information.. A city or coordinates are required.\n");
        exit( EXIT_FAILURE );
    }

    printf("URL: %s\n", url);
    struct string s = scrape(url, s);
    printf("Temperature: %s\n", getValue("temperature", "value", 11, 5, s));
    printf("Wind Speed: %s\nWind Direction: %s\n", getValue("speed", "name", 5, 4, s), getValue("direction", "code", 8, 4, s));
    printf("Weather: %s\n", getValue("weather", "value", 7, 5, s));
    free(s.ptr);
}
