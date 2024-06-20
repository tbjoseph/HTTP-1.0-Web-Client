#pragma once
#ifndef SOCKET_H
#define SOCKET_H
#include "URLParser.h"
#include <iostream>
#include <set>
#include <string>

class Socket {
private:
    SOCKET sock; // socket handle
    char* buf; // current buffer
    int allocatedSize; // bytes allocated for buf
    int curPos; // current position in buffer
    int tamuCount;
    // extra stuff as needed
    bool Read(size_t max_download_size, bool printInfo = true);
    bool Connect(const sockaddr_in& server, const char* host, const char* request, const char* method, const char* valid_codes, size_t max_download_size, bool asterisk, bool& isCodeValid, bool printInfo = true, LONG volatile* stats = nullptr);

public:
    Socket();
    ~Socket();
    void Print();
    bool Write(char* inputURL, bool checkRobot, std::set<std::string>* seenHosts = nullptr, std::set<std::string>* seenIPs = nullptr);
    bool Write(char* inputURL, std::set<std::string>* seenHosts, std::set<std::string>* seenIPs, URLParser &parser, HTMLParserBase &html_parser, HANDLE hostsMutex, HANDLE ipsMutex, LONG volatile* stats);
};

#endif //SOCKET_H