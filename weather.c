#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <curl/curl.h>

struct string {
    char *ptr;
    size_t len;
};


void init_string(struct string *s) {
    s->len = 0;
    s->ptr = malloc(s->len+1);
    if (s->ptr == NULL) {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
}

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

struct string scrape(char *url, struct string s){
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if(curl) {
        init_string(&s);
        //curl_easy_setopt(curl, CURLOPT_URL, "http://api.openweathermap.org/data/2.5/weather?q=perth&mode=xml");
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

//Returns the value from the xml that is pointed to by the passed variables
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

char *getUrlCity(char *url, char *city){
    char *pre = "http://api.openweathermap.org/data/2.5/weather?q=";
    char *post = "&mode=xml";
    int preLen = strlen(pre);
    int cityLen = strlen(city);
    int postLen = strlen(post);
    url = (char *) malloc(preLen + cityLen + postLen + 1);
    for(int i=0; i<preLen; i++){
        url[i] = pre[i];
    }
    for(int i=0; i<cityLen; i++){
        url[preLen + i] = city[i];
    }
    for(int i=0; i<postLen; i++){
        url[preLen + cityLen + i] = post[i];
    }
    //Appending a null byte at the end of the string
    char *null = "\0";
    url[preLen + cityLen + postLen] = null[0];
    return url;
}

int main(int argc, char **argv){
    char *url = ""; 
    char *city = "perth";
    url = getUrlCity(url, city);
    printf("URL: %s\n", url);

    struct string s = scrape(url, s);
    //printf("Output:\n%s\n", s.ptr);
    printf("Temperature: %s\n", getValue("temperature", "value", 11, 5, s));
    printf("Wind Speed: %s\nWind Direction: %s\n", getValue("speed", "name", 5, 4, s), getValue("direction", "code", 8, 4, s));
    printf("Weather: %s\n", getValue("weather", "value", 7, 5, s));
    free(s.ptr);
}

/*
    -c cityname
    -C lon,lat


   */
