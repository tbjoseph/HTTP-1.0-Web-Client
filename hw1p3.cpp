// hw1p3.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"


// this function is where threadA starts
UINT workerThread(LPVOID pParam)
{
	Crawler* p = ((Crawler*)pParam);
	p->Run();
	return 0;
}

UINT dataThread(LPVOID pParam)
{
	Crawler* p = ((Crawler*)pParam);
	p->StatsRun();
	return 0;
}

int main(int argc, char** argv)
{
    if (argc == 2)
    {
        WSADATA wsaData;
        WORD wVersionRequested = MAKEWORD(2, 2);
        if (WSAStartup(wVersionRequested, &wsaData) != 0)
        {
            printf("WSAStartup error %d\n", WSAGetLastError());
            WSACleanup();
            return -1;
        }

        printf("URL: %s\n", argv[1]);
        Socket httpSock;
        if (!httpSock.Write(argv[1], false)) {
            WSACleanup();
            return -1;
        }
        httpSock.Print();

        WSACleanup();
        return 0;
    }
    else if ((argc == 3) && (atoi(argv[1]) == 1)) {

        // open file
        HANDLE hFile = CreateFile(argv[2], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        // process errors
        if (hFile == INVALID_HANDLE_VALUE)
        {
            printf("CreateFile failed with %d\n", GetLastError());
            return 0;
        }
        // get file size
        LARGE_INTEGER li;
        BOOL bRet = GetFileSizeEx(hFile, &li);
        // process errors
        if (bRet == 0)
        {
            printf("GetFileSizeEx error %d\n", GetLastError());
            return 0;
        }
        // read file into a buffer
        int fileSize = (DWORD)li.QuadPart;			// assumes file size is below 2GB; otherwise, an __int64 is needed
        DWORD bytesRead;
        // allocate buffer
        char* fileBuf = new char[fileSize + 1];
        // read into the buffer
        bRet = ReadFile(hFile, fileBuf, fileSize, &bytesRead, NULL);
        // process errors
        if (bRet == 0 || bytesRead != fileSize)
        {
            printf("ReadFile failed with %d\n", GetLastError());
            return 0;
        }
        fileBuf[fileSize] = '\0';
        CloseHandle(hFile);

        WSADATA wsaData;
        WORD wVersionRequested = MAKEWORD(2, 2);
        if (WSAStartup(wVersionRequested, &wsaData) != 0)
        {
            printf("WSAStartup error %d\n", WSAGetLastError());
            WSACleanup();
            return -1;
        }

        printf("Opened %s with size %d\n", argv[2], fileSize);

        Socket httpSock;
        std::set<std::string>* seenHosts = new std::set<std::string>;
        std::set<std::string>* seenIPs = new std::set<std::string>;

        const char delimeters[] = "\r\n"; // delimiters \r and \n
        char* token = strtok(fileBuf, delimeters);

        while (token != NULL) {
            printf("URL: %s\n", token);
            httpSock.Write(token, true, seenHosts, seenIPs);
            token = strtok(NULL, delimeters); // get next token
        }

        delete seenHosts;
        delete seenIPs;
        delete[] fileBuf;

        WSACleanup();
        return 0;
    }
    else if ((argc == 3) && (atoi(argv[1]) > 1)) {

        // open file
        HANDLE hFile = CreateFile(argv[2], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        // process errors
        if (hFile == INVALID_HANDLE_VALUE)
        {
            printf("CreateFile failed with %d\n", GetLastError());
            return 0;
        }
        // get file size
        LARGE_INTEGER li;
        BOOL bRet = GetFileSizeEx(hFile, &li);
        // process errors
        if (bRet == 0)
        {
            printf("GetFileSizeEx error %d\n", GetLastError());
            return 0;
        }
        // read file into a buffer
        int fileSize = (DWORD)li.QuadPart;			// assumes file size is below 2GB; otherwise, an __int64 is needed
        DWORD bytesRead;
        // allocate buffer
        char* fileBuf = new char[fileSize + 1];
        // read into the buffer
        bRet = ReadFile(hFile, fileBuf, fileSize, &bytesRead, NULL);
        // process errors
        if (bRet == 0 || bytesRead != fileSize)
        {
            printf("ReadFile failed with %d\n", GetLastError());
            return 0;
        }
        fileBuf[fileSize] = '\0';
        CloseHandle(hFile);

        WSADATA wsaData;
        WORD wVersionRequested = MAKEWORD(2, 2);
        if (WSAStartup(wVersionRequested, &wsaData) != 0)
        {
            printf("WSAStartup error %d\n", WSAGetLastError());
            WSACleanup();
            return -1;
        }

        printf("Opened %s with size %d\n", argv[2], fileSize);
        





        // thread handles are stored here; they can be used to check status of threads, or kill them
        int numThreads = atoi(argv[1]);
        HANDLE *handles = new HANDLE [numThreads];
        Crawler p(numThreads);

        p.ReadFile(fileBuf);
        HANDLE handle = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)dataThread, &p, 0, NULL);

        for (int i = 0; i < numThreads; i++)
        {
            handles [i] = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)workerThread, &p, 0, NULL);
        }

        // make sure this thread hangs here until the other three quit; otherwise, the program will terminate prematurely
        for (int i = 0; i < numThreads; i++)
        {
            WaitForSingleObject (handles [i], INFINITE);
            CloseHandle (handles [i]);
        }
        p.quitStatsRun();
        WaitForSingleObject (handle, INFINITE);
        CloseHandle (handles);








        delete[] fileBuf;
        delete[] handles;

        WSACleanup();
        return 0;
    }
    else if ((argc == 3) && (atoi(argv[1]) == -1)) {
    // open file
        HANDLE hFile = CreateFile(argv[2], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        // process errors
        if (hFile == INVALID_HANDLE_VALUE)
        {
            printf("CreateFile failed with %d\n", GetLastError());
            return 0;
        }
        // get file size
        LARGE_INTEGER li;
        BOOL bRet = GetFileSizeEx(hFile, &li);
        // process errors
        if (bRet == 0)
        {
            printf("GetFileSizeEx error %d\n", GetLastError());
            return 0;
        }
        // read file into a buffer
        int fileSize = (DWORD)li.QuadPart;			// assumes file size is below 2GB; otherwise, an __int64 is needed
        DWORD bytesRead;
        // allocate buffer
        char* fileBuf = new char[fileSize + 1];
        // read into the buffer
        bRet = ReadFile(hFile, fileBuf, fileSize, &bytesRead, NULL);
        // process errors
        if (bRet == 0 || bytesRead != fileSize)
        {
            printf("ReadFile failed with %d\n", GetLastError());
            return 0;
        }
        fileBuf[fileSize] = '\0';
        CloseHandle(hFile);

        WSADATA wsaData;
        WORD wVersionRequested = MAKEWORD(2, 2);
        if (WSAStartup(wVersionRequested, &wsaData) != 0)
        {
            printf("WSAStartup error %d\n", WSAGetLastError());
            WSACleanup();
            return -1;
        }

        printf("Opened %s with size %d\n", argv[2], fileSize);

        Socket httpSock;
        std::set<std::string>* seenHosts = new std::set<std::string>;
        std::set<std::string>* seenIPs = new std::set<std::string>;

        const char delimeters[] = "\r\n"; // delimiters \r and \n
        char* token = strtok(fileBuf, delimeters);

        while (token != NULL) {
            printf("URL: %s\n", token);
            httpSock.Write(token, true, seenHosts, seenIPs);
            token = strtok(NULL, delimeters); // get next token
        }

        delete seenHosts;
        delete seenIPs;
        delete[] fileBuf;

        WSACleanup();
        return 0;
        
    }


    // while (token != NULL) {
    //         // printf("URL: %s\n", token);
    //         if (!parser.Parse(token, false)) {
    //             token = strtok(NULL, delimeters); // get next token
    //             continue;
    //         }
    //         host = parser.getHost();
    //         findTamu = strstr(parser.getHost(), "tamu.edu");
    //         expectedAddress = host + strlen(host) - 8;
    //         if ( !findTamu || (findTamu != expectedAddress) )
    //         {
    //             token = strtok(NULL, delimeters); // get next token
    //             continue;
    //         }
            
    //         printf("%s\n", host);
    //         tamuCount++;
    //         token = strtok(NULL, delimeters); // get next token
    //     }

    printf("usage: ./a.out scheme://host[:port][/path][?query][#fragment]\n");
    printf("or\n");
    printf("usage: ./a.out [threads>=0] [filename]\n");

    return -1;

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
