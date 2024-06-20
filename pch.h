// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#define _CRT_SECURE_NO_WARNINGS
#define MAX_PAGE_SIZE 2097152
#define MAX_ROBOT_PAGE_SIZE 16384
#define RESIZE_BUFFER_LENGTH 32768
#define INITIAL_BUF_SIZE 1000

enum handleEnum { NUM_THREADS, QUEUE_SIZE, NUM_EXTRACTED, NUM_UNIQUE_HOSTS, NUM_DNS_LOOKUPS, NUM_UNIQUE_IPS, NUM_ROBOTS_PASSED, NUM_CRAWLED_URLS, TOTAL_LINKS, BYTES_DOWNLOADED, NUM_2XX, NUM_3XX, NUM_4XX, NUM_5XX, NUM_XXX, TAMU_COUNT };

#include <windows.h>
#include "HTMLParserBase.h"
#include "URLParser.h"
#include "Socket.h"
#include <iostream>
#include <string>
#include <fstream>
#include <iostream>
#include <set>
#include <queue>
#include "Crawler.h"

#pragma comment(lib, "wsock32.lib")

#endif //PCH_H
