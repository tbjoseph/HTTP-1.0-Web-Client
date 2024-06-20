#pragma once

#ifndef CRAWLER_H
#define CRAWLER_H

class Crawler {
private:
    HANDLE	queueMutex;
    HANDLE	hostsMutex;
    HANDLE	ipsMutex;
	HANDLE	eventQuit;

    std::set<std::string>* seenHosts;
    std::set<std::string>* seenIPs;
    std::queue<char*>* queue;

    LONG volatile stats[20];
public:
    Crawler(int numThreads) {
        // create a mutex for accessing critical sections (including printf); initial state = not locked
		queueMutex = CreateMutex(NULL, 0, NULL);
		hostsMutex = CreateMutex(NULL, 0, NULL);
		ipsMutex = CreateMutex(NULL, 0, NULL);
		eventQuit = CreateEvent(NULL, true, false, NULL);

        seenHosts = new std::set<std::string>;
        seenIPs = new std::set<std::string>;
        queue = new std::queue<char*>;

        stats[NUM_THREADS] = numThreads;
        stats[NUM_EXTRACTED] = 0;
        stats[NUM_UNIQUE_HOSTS] = 0;
        stats[NUM_DNS_LOOKUPS] = 0;
        stats[NUM_UNIQUE_IPS] = 0;
        stats[NUM_ROBOTS_PASSED] = 0;
        stats[NUM_CRAWLED_URLS] = 0;
        stats[TOTAL_LINKS] = 0;
        stats[BYTES_DOWNLOADED] = 0;
        stats[NUM_2XX] = 0;
        stats[NUM_3XX] = 0;
        stats[NUM_4XX] = 0;
        stats[NUM_5XX] = 0;
        stats[NUM_XXX] = 0;
        stats[TAMU_COUNT] = 0;
    };

    ~Crawler() {
        delete seenHosts;
        delete seenIPs;
        delete queue;
    }

    void ReadFile (char* fileBuf)
    {
        // produce all items into the queue before the crawl begins
        const char delimeters[] = "\r\n"; // delimiters \r and \n
        char* token = strtok(fileBuf, delimeters);
        while (token != NULL) {
            // printf("URL: %s\n", token);
            queue->push(token);
            token = strtok(NULL, delimeters); // get next token
        }
        stats[QUEUE_SIZE] = static_cast<int>(queue->size());
    }


    void Run () // crawling thread, N instances running
    {
        Socket httpSock;
        URLParser parser;
        HTMLParserBase html_parser;
        
        while (true)
        {
            WaitForSingleObject (queueMutex, INFINITE);					// lock mutex
            if (queue->size() == 0) // finished crawling
            {
                ReleaseMutex(queueMutex);						        // release mutex
                InterlockedAdd(stats + NUM_THREADS, -1); // atomic write
                break;
            }
            char* url = queue->front (); queue->pop();
            // printf("%s\n", url);
            ReleaseMutex(queueMutex);
            InterlockedAdd(stats + QUEUE_SIZE, -1);
            InterlockedAdd(stats + NUM_EXTRACTED, 1);

            // crawl x
            httpSock.Write(url, seenHosts, seenIPs, parser, html_parser, hostsMutex, ipsMutex, stats);
        }
    } 
 
	void StatsRun(void)
    {
        // wait for both threadA threads to quit
        clock_t start = clock();
        clock_t end;
        int elapsed_time;
        int urls;
        int totalUrls = 0;
        int bytes;
        int totalBytes = 0;
        while (WaitForSingleObject (eventQuit, 2000) == WAIT_TIMEOUT)
        {
            end = clock();
            elapsed_time = static_cast<int>(std::round( (double)(end - start) / CLOCKS_PER_SEC ));
            // no need to sync prints as only StatsRun is printing
            // atomic reads w/ InterlockedAdd
            bytes = InterlockedExchange(stats + BYTES_DOWNLOADED, 0);
            totalBytes += bytes;
            urls = InterlockedExchange(stats + NUM_CRAWLED_URLS, 0);
            totalUrls += urls;

            printf("[%3d] %4d Q %6d E %7d H %6d D %6d I %5d R %5d C %5d L %4dK\n", 
            elapsed_time,
            InterlockedAdd(stats + NUM_THREADS, 0), // atomic read
            InterlockedAdd(stats + QUEUE_SIZE, 0),
            InterlockedAdd(stats + NUM_EXTRACTED, 0),
            InterlockedAdd(stats + NUM_UNIQUE_HOSTS, 0),
            InterlockedAdd(stats + NUM_DNS_LOOKUPS, 0),
            InterlockedAdd(stats + NUM_UNIQUE_IPS, 0),
            InterlockedAdd(stats + NUM_ROBOTS_PASSED, 0),
            totalUrls,
            InterlockedAdd(stats + TOTAL_LINKS, 0) / 1000
            );

            printf("     *** crawling %.1f pps @ %.1f Mbps\n", urls / 2.0, bytes / 1.0e6 / 2.0);             
        }

        end = clock();
        elapsed_time = static_cast<int>(std::round( (double)(end - start) / CLOCKS_PER_SEC ));

        // Shouldn't need atomic operations now, but included just in case
        printf ("\nExtracted %d URLs @ %d/s\n", 
        InterlockedAdd(stats + NUM_EXTRACTED, 0), 
        InterlockedAdd(stats + NUM_EXTRACTED, 0) / elapsed_time
        );
        
        printf ("Looked up %d DNS names @ %d/s\n", 
        InterlockedAdd(stats + NUM_DNS_LOOKUPS, 0),
        InterlockedAdd(stats + NUM_DNS_LOOKUPS, 0) / elapsed_time
        );

        printf ("Attempted %d site robots @ %d/s\n", 
        InterlockedAdd(stats + NUM_ROBOTS_PASSED, 0),
        InterlockedAdd(stats + NUM_ROBOTS_PASSED, 0) / elapsed_time
        );

        bytes = InterlockedExchange(stats + BYTES_DOWNLOADED, 0);
        totalBytes += bytes;
        urls = InterlockedExchange(stats + NUM_CRAWLED_URLS, 0);
        totalUrls += urls;
        printf ("Crawled %d pages @ %d/s (%.2f MB)\n", totalUrls, totalUrls / elapsed_time, totalBytes / 1.0e6);

        printf ("Parsed %d links @ %d/s\n", 
        InterlockedAdd(stats + TOTAL_LINKS, 0),
        InterlockedAdd(stats + TOTAL_LINKS, 0) / (int)elapsed_time
        );

        printf("HTTP codes: 2xx = %d, 3xx = %d, 4xx = %d, 5xx = %d, other = %d\n",
        InterlockedAdd(stats + NUM_2XX, 0),
        InterlockedAdd(stats + NUM_3XX, 0),
        InterlockedAdd(stats + NUM_4XX, 0),
        InterlockedAdd(stats + NUM_5XX, 0),
        InterlockedAdd(stats + NUM_XXX, 0)
        );

        // printf ("tamu count: %d\n", 
        // InterlockedAdd(stats + TAMU_COUNT, 0)
        // );
    };
    
    void quitStatsRun() { SetEvent (eventQuit); };
};

#endif //CRAWLER_H
