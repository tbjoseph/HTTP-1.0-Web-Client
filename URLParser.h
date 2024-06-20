#pragma once

#ifndef URLPARSER_H
#define URLPARSER_H

#include <iostream>

class URLParser {
private:
    char request[MAX_REQUEST_LEN + 1]; // + 1 for null terminator
    int port_num;
    char host[MAX_HOST_LEN + 1]; // + 1 for null terminator

public:

    char* getRequest() { return this->request; };

    int getPortNum() { return this->port_num; };

    char* getHost() { return this->host; };

    bool Parse(char* inputURL, bool printInfo = true) {
        if (printInfo) { printf("\tParsing URL... "); }

        // check URL length
        if ((strlen(inputURL)) > MAX_URL_LEN) {
            if (printInfo) { printf("failed with invalid URL length\n"); }
            return false;
        }
        // copy original url
        char fullURL[MAX_URL_LEN + 1]; // + 1 for null terminator
        sprintf(fullURL, "%s", inputURL);


        // check for presence of scheme and ensure scheme is in the front
        char* checkScheme = strstr(fullURL, "http://");
        if (!checkScheme || (checkScheme != fullURL)) {
            if (printInfo) { printf("failed with invalid scheme\n"); }
            return false;
        }
        // chop off http://
        char* url = fullURL + 7;
        // check for and remove fragment
        char* fragment = strchr(url, '#');
        if (fragment) *fragment = '\0';


        // check for path + query for the http request
        // initialize request to empty in case there is no query or path
        char* request = (char*)"";
        // extract request based on whether '?' or '/' is seen first
        char url_indexer;
        size_t i;
        for (i = 0; url[i] != '\0'; i++) {
            url_indexer = url[i];
            if (url_indexer == '?')
            {
                request = url + i;
                break;
            }
            else if (url_indexer == '/')
            {
                request = url + i + 1; // + 1 to remove '/', since it needs to be added in the case '?' is found first
                break;
            }
        }
        // check request length. +1 for the '/' that must be added to the front
        if ((strlen(request) + 1) > MAX_REQUEST_LEN) {
            if (printInfo) { printf("failed with invalid request length\n"); }
            return false;
        }
        sprintf(this->request, "/%s", request);
        url[i] = '\0'; // null terminator for port


        // Learned about strtoul from https://stackoverflow.com/questions/6093414/convert-char-array-to-single-int
        char* port = strchr(url, ':');
        if (port == NULL) this->port_num = 80;
        else {
            *port = '\0'; // null terminator for host
            port += 1;
            char* endptr;
            unsigned long num;
            num = strtoul(port, &endptr, 10); // convert char* to ulong
            if ((*endptr != '\0') || (num == 0) || (num > UINT16_MAX)) {
                if (printInfo) { printf("failed with invalid port number\n"); }
                return false;
            }
            this->port_num = (int)num;
        }


        // remaining url is the host
        char* host = url;
        // check host length
        if (strlen(host) > MAX_HOST_LEN)
        {
            if (printInfo) { printf("failed with invalid host length\n"); } 
            return false;
        }
        sprintf(this->host, "%s", host);

        if (printInfo) { printf("host %s, port %d\n", this->host, this->port_num); } 

        return true;
    };

};

#endif //URLPARSER_H
