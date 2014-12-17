/*
 *      Weather by Prodge
 *      
 *      Release version 1.0
 *
 *      Github: https://github.com/Prodge/weather
 *      Website: http://prodge.net
 *
 *      See readme.md for information on how to use
 *      Or use 'man weather'
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <curl/curl.h>
#include <getopt.h>
#include <unistd.h>
#include <ctype.h>

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
char *getValue(char *class, char *type, struct string s){
    int classLen = strlen(class);
    int typeLen = strlen(type);
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

void output(struct string s, char *format, char *url){
    int thisChar = 0; 
    while(true){
        char character = ' ';
        character = format[thisChar];
        if(character == '%'){
            thisChar++;
            character = format[thisChar]; //getting the next character
            switch(character){  //move this to a new function
                case 'c': // City Name
                    printf("%s", getValue("city", "name", s));
                    break;
                case 'C': // Coordinates
                    printf("%s,%s", getValue("coord", "lat", s), getValue("coord", "lon", s));
                    break;
                case 'l': // Latitude
                    printf("%s", getValue("coord", "lat", s));
                    break;
                case 'L': // Longitude
                    printf("%s", getValue("coord", "lon", s));
                    break;
                case 'n': // New line character
                    printf("\n");
                    break;
                case 'u': // Print the URL
                    printf("%s", url);
                    break;
                case 'N': // null byte character
                    printf("\\0");
                    break;
                case 's': // Sunrise time
                    printf("%s", getValue("sun", "rise", s));
                    break;
                case 'S': // Sunset Time
                    printf("%s", getValue("sun", "set", s));
                    break;
                case 't': // Current Temperature in K
                    printf("%s", getValue("temperature", "value", s));  
                    break;
                case 'T': // Current Temperature in C 
                    printf("%.2f", atoi(getValue("temperature", "value", s)) - 273.15); 
                    break;
                case 'f': // Current Temperature in F
                    printf("%.2f", ((1.8 * (atoi(getValue("temperature", "value", s)) - 273.15)) + 32));
                    break;
                case 'h': // Current Humidity
                    printf("%s", getValue("humidity", "value", s));
                    break;
                case 'p': // Current Air Prussure
                    printf("%s", getValue("pressure", "value", s));
                    break;
                case 'w': // Wind Speed
                    printf("%s", getValue("speed", "value", s));
                    break;
                case 'W': // Wind Descriptor
                    printf("%s", getValue("speed", "name", s));
                    break;
                case 'd': // Wind direction abreviated
                    printf("%s", getValue("direction", "code", s));
                    break;
                case 'D': // Wind direction
                    printf("%s", getValue("direction", "name", s));
                    break;
                case 'a': // wind angle
                    printf("%s", getValue("direction", "value", s));
                    break;
                case 'r': // Raining
                    printf("%s", getValue("precipitation", "mode", s));
                    break;
                case 'k': // Cloud coverage %
                    printf("%s", getValue("clouds", "value", s));
                    break;
                case 'K': // cloud coverage descriptor
                    printf("%s", getValue("clouds", "name", s));
                    break;
                case 'i': // Weather descriptor
                    printf("%s", getValue("weather", "value", s));
                    break;
                case 'U': // Last updated
                    printf("%s", getValue("lastupdate", "value", s));
                    break;
                case '%': // percentage sign
                    printf("%%");
                    break;
            }   
        }else{ // Character is not specifying a variable, probably user formating
            printf("%c", character);
        }
        thisChar++;
        if(character == '\0'){
            printf("\n");
            break;
        }
    }
}

int main(int argc, char **argv){
    /*Setting up core variables*/
    char *city;
    char *url = ""; 
    int subscribe = 0;
    int localType = 0; // set to 0 for inadequate input, 1 for city, 2 for coords
    struct coords coord;
    char *format;
    int c;
    /*Reading user args*/
    while((c = getopt(argc, argv, "c:C:s:S:f:")) != -1){
        switch(c){
            case 's':   //Subscribe in seconds
                subscribe = atoi(optarg);
                printf("Subscribe: %i\n", subscribe);
                break;
            case 'c':   //City
                city = malStrCpy(optarg);
                for(int i=0; i < strlen(city); i++){ //converting to lowercase
                    city[i] = tolower(city[i]);
                }
                localType = 1;
                url = getUrlCity(url, city);
                break;
            case 'C':   //Coords
                coord = getCoords(optarg);
                url = getUrlCoords(url, coord);
                localType = 2;
                break;
            case 'f':   //Format String
                format = malStrCpy(optarg);
                printf("Format String: %s\n", format);
                break;
        }
    }
    if(localType == 0){ //Recieved no location information
        fprintf(stderr, "You did not enter enough information.. A city or coordinates are required.\n");
        exit( EXIT_FAILURE );
    }
    if(subscribe == 0){ //Subscribe was not set and will therefore only print once
        struct string s = scrape(url, s); //scraping the desired url into a string 's'
        output(s, format, url);
        free(s.ptr);
    }else{
        while(true){
        struct string s = scrape(url, s); //scraping the desired url into a string 's'
        output(s, format, url);
        free(s.ptr);
        sleep(subscribe);
        }
    }
}
