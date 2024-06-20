#include "pch.h"

Socket::Socket()
{
    // create this buffer once, then possibly reuse for multiple connections in Part 3
    this->buf = new char[1000];
    this->allocatedSize = 1000;
    this->curPos = 0;
    tamuCount = 0;
}

Socket::~Socket() {
    delete[] this->buf;
}

void Socket::Print() {
    printf("%s", this->buf);
}

bool Socket::Write(char* inputURL, bool checkRobot, std::set<std::string>* seenHosts, std::set<std::string>* seenIPs)
{
    if (this->allocatedSize > RESIZE_BUFFER_LENGTH) {
        // resize buffer with realloc() to initial size
        int newSize = INITIAL_BUF_SIZE;
        char* newBuf = (char*)realloc(this->buf, newSize);
        if (!newBuf) {
            printf("Failed to resize buffer\n");
            return false;
        }
        this->buf = newBuf;
        this->allocatedSize = newSize;
    }
    URLParser parser;
    if (!parser.Parse(inputURL)) {
        return false;
    }
    int port_num = parser.getPortNum();
    char* request = parser.getRequest();
    char* host = parser.getHost();
    if (seenHosts != nullptr) {
        printf("\tChecking host uniqueness... ");
        auto result = seenHosts->insert(host);
        if (result.second == false) {
            printf("failed\n");
            return false;
        }
        printf("passed\n");
    }



    // ***** DNS
    printf("\tDoing DNS... ");
    clock_t start = clock();
    clock_t end;
    double elapsed_time;
    struct hostent* remote;
    // structure for connecting to server
    struct sockaddr_in server;
    // string for checking seen IPs
    std::string ip_address;
    // first assume that the string is an IP address
    DWORD IP = inet_addr(host);
    if (IP == INADDR_NONE) {
        // if not a valid IP, then do a DNS lookup
        if ((remote = gethostbyname(host)) == NULL)
        {
            printf("Invalid string: neither FQDN, nor IP address\n");
            return false;
        }
        else {// take the first IP address and copy into sin_addr
            memcpy((char*)&(server.sin_addr), remote->h_addr, remote->h_length);

            end = clock();
            elapsed_time = (double)(end - start) / CLOCKS_PER_SEC * 1000;
            printf("done in %g ms, found %s\n", elapsed_time, inet_ntoa(server.sin_addr));
            ip_address = inet_ntoa(server.sin_addr);
        }
    }
    else {
        // if a valid IP, directly drop its binary version into sin_addr
        server.sin_addr.S_un.S_addr = IP;

        end = clock();
        elapsed_time = (double)(end - start) / CLOCKS_PER_SEC * 1000;
        printf("done in %g ms, found %s\n", elapsed_time, host);
        ip_address = host;
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(port_num);		// host-to-network flips the byte order
    if (seenIPs != nullptr) {
        printf("\tChecking IP uniqueness... ");
        auto result = seenIPs->insert(ip_address);
        if (result.second == false) {
            printf("failed\n");
            return false;
        }
        printf("passed\n");
    }



    // ***** Connect
    bool isCodeValid = true;
    if (checkRobot) {
        if (!Connect(server, host, "/robots.txt", "HEAD", "4XX", MAX_ROBOT_PAGE_SIZE, false, isCodeValid)) {
            return false;
        }
    }
    if (!isCodeValid) return true;

    if (!Connect(server, host, request, "GET", "2XX", MAX_PAGE_SIZE, true, isCodeValid)) {
        return false;
    }
    if (!isCodeValid) return true;

    // ***** page parsing
    char* response_break = strstr(this->buf, "\r\n\r\n");
    printf("      + Parsing page... ");
    start = clock();
    HTMLParserBase html_parser;
    int nlinks;
    html_parser.Parse(response_break + 4, static_cast<int>(strlen(response_break + 4)), inputURL, static_cast<int>(strlen(inputURL)), &nlinks);

    if (nlinks < 0) {
        printf("error during parsing\n");
        return false;
    }
    end = clock();
    elapsed_time = (double)(end - start) / CLOCKS_PER_SEC * 1000;
    printf("done in %g ms with %d links\n", elapsed_time, nlinks);
    *response_break = '\0';

    return true;
}


bool Socket::Connect(const sockaddr_in& server, const char* host, const char* request, const char* method, const char* valid_codes, size_t max_download_size, bool asterisk, bool& isCodeValid, bool printInfo, LONG volatile* stats) {

    // ***** Open a TCP socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    this->sock = sock;
    if (sock == INVALID_SOCKET)
    {
        if (printInfo) { printf("socket() generated error %d\n", WSAGetLastError()); }
        return false;
    }



    // ***** Connect socket to server
    char connect_symbol;
    const char* destination;
    if (asterisk) {
        connect_symbol = '*';
        destination = "page";
    }
    else {
        connect_symbol = ' ';
        destination = "robots";
    }
    if (printInfo) { printf("      %c Connecting on %s... ", connect_symbol, destination); }
    clock_t start = clock();
    // setup the port # and protocol type
    if (connect(sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in)) == SOCKET_ERROR)
    {
        if (printInfo) { printf("Connection error: %d\n", WSAGetLastError()); }
        closesocket(sock);
        return false;
    }
    clock_t end = clock();
    double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC * 1000;
    if (printInfo) { printf("done in %g ms\n", elapsed_time); }
    


    // ***** Send request conforming to correct protocol
    if (printInfo) { printf("\tLoading... "); }
    start = clock();
    // send HTTP requests here
    int requestLength = 76 + MAX_HOST_LEN + MAX_URL_LEN; // 76 chars in the request protocol besides host and url
    char sendBuf[76 + MAX_HOST_LEN + MAX_URL_LEN + 1];
    // place request into buf (e.g., sprintf)
    sprintf(sendBuf, "%s %s HTTP/1.0\r\nUser-agent: myTAMUcrawler/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", method, request, host);
    if (send(sock, sendBuf, requestLength, 0) == SOCKET_ERROR) {
        if (printInfo) { printf("Send error: %d\n", WSAGetLastError()); }
        closesocket(sock);
        return false;
    }
    // receive response into recvBuf
    if (!Read(max_download_size, printInfo)) {
        closesocket(sock);
        this->curPos = 0;
        return false;
    }
    end = clock();
    elapsed_time = (double)(end - start) / CLOCKS_PER_SEC * 1000;
    if (printInfo) { printf("done in %g ms with %d bytes\n", elapsed_time, this->curPos); }
    else { InterlockedAdd(stats + BYTES_DOWNLOADED, this->curPos); }


    // ***** close the socket to this server and reset curPos
    closesocket(sock);
    this->curPos = 0;


    // ***** verify header
    if (printInfo) { printf("\tVerifying header... "); }
    
    if (this->buf != strstr(this->buf, "HTTP/")) {
        if (printInfo) { printf("failed with non-HTTP header (does not begin with HTTP/)\n"); }        
        return false;
    }

    if (printInfo) { printf("status code %c%c%c\n", this->buf[9], this->buf[10], this->buf[11]); }
    else {
        switch (buf[9])
        {
        case '2':
            InterlockedAdd(stats + NUM_2XX, 1);
            break;
        case '3':
            InterlockedAdd(stats + NUM_3XX, 1);
            break;
        case '4':
            InterlockedAdd(stats + NUM_4XX, 1);
            break;
        case '5':
            InterlockedAdd(stats + NUM_5XX, 1);
            break;
        default:
            InterlockedAdd(stats + NUM_XXX, 1);
            break;
        }
    }

    if (this->buf[9] == valid_codes[0]) {
        isCodeValid = true;
    }
    else {
        isCodeValid = false;
    }
    
    return true;

}


bool Socket::Read(size_t max_download_size, bool printInfo)
{
    // initialize timeout to 10 seconds once
    struct timeval timeout;
    timeout.tv_sec = 10;  // 10 seconds
    timeout.tv_usec = 0;  // 0 microseconds

    while (true)
    {
        // set file descriptor
        fd_set fd;
        FD_ZERO(&fd);
        FD_SET(this->sock, &fd);

        // wait to see if socket has any data (see MSDN)
        int ret = select(0, &fd, NULL, NULL, &timeout);
        if (ret > 0)
        {
            // new data available; now read the next segment
            int bytes = recv(this->sock, this->buf + this->curPos, this->allocatedSize - this->curPos, 0);

            if (bytes == SOCKET_ERROR) {
                if (printInfo) { printf("recv failed with error: %d\n", WSAGetLastError()); }                
                break;
            }

            if (bytes == 0) {
                this->buf[this->curPos] = '\0'; // NULL-terminate buffer
                return true; // normal completion
            }

            this->curPos += bytes; // adjust where the next recv goes

            if (this->curPos > max_download_size) {
                if (printInfo) { printf("failed with exceeding max\n"); }
                break;
            }

            if ((this->allocatedSize - this->curPos) < 100) {
                // resize buffer with realloc()

                int newSize = this->allocatedSize * 2; // Double the buffer size
                char* newBuf = (char*)realloc(this->buf, newSize);
                if (!newBuf) {
                    if (printInfo) { printf("Failed to resize buffer\n"); }
                    break;
                }
                this->buf = newBuf;
                this->allocatedSize = newSize;
            }
        }
        else if (ret == 0) {
            if (printInfo) { printf("Timeout occurred\n"); }            
            break;
        }
        else {
            if (printInfo) { printf("select failed with error: %d\n", WSAGetLastError()); }
            break;
        }
    }
    return false;
}


bool Socket::Write(char* inputURL, std::set<std::string>* seenHosts, std::set<std::string>* seenIPs, URLParser &parser, HTMLParserBase &html_parser, HANDLE hostsMutex, HANDLE ipsMutex, LONG volatile* stats)
{
    if (this->allocatedSize > RESIZE_BUFFER_LENGTH) {
        // resize buffer with realloc() to initial size
        int newSize = INITIAL_BUF_SIZE;
        char* newBuf = (char*)realloc(this->buf, newSize);
        if (!newBuf) {
            return false;
        }
        this->buf = newBuf;
        this->allocatedSize = newSize;
    }
    if (!parser.Parse(inputURL, false)) {
        return false;
    }
    int port_num = parser.getPortNum();
    char* request = parser.getRequest();
    char* host = parser.getHost();
    if (seenHosts != nullptr) {
        WaitForSingleObject (hostsMutex, INFINITE);
        auto result = seenHosts->insert(host);
        ReleaseMutex(hostsMutex);
        if (result.second == false) {
            return false;
        }
        InterlockedAdd(stats + NUM_UNIQUE_HOSTS, 1);
    }



    // ***** DNS
    clock_t start = clock();
    clock_t end;
    double elapsed_time;
    struct hostent* remote;
    // structure for connecting to server
    struct sockaddr_in server;
    // string for checking seen IPs
    std::string ip_address;
    // first assume that the string is an IP address
    DWORD IP = inet_addr(host);
    if (IP == INADDR_NONE) {
        // if not a valid IP, then do a DNS lookup
        if ((remote = gethostbyname(host)) == NULL)
        {
            return false;
        }
        else {// take the first IP address and copy into sin_addr
            InterlockedAdd(stats + NUM_DNS_LOOKUPS, 1);
            
            memcpy((char*)&(server.sin_addr), remote->h_addr, remote->h_length);

            end = clock();
            elapsed_time = (double)(end - start) / CLOCKS_PER_SEC * 1000;
            ip_address = inet_ntoa(server.sin_addr);
        }
    }
    else {
        // if a valid IP, directly drop its binary version into sin_addr
        server.sin_addr.S_un.S_addr = IP;

        end = clock();
        elapsed_time = (double)(end - start) / CLOCKS_PER_SEC * 1000;
        ip_address = host;
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(port_num);		// host-to-network flips the byte order
    if (seenIPs != nullptr) {
        WaitForSingleObject (ipsMutex, INFINITE);
        auto result = seenIPs->insert(ip_address);
        ReleaseMutex(ipsMutex);
        if (result.second == false) {
            return false;
        }
        InterlockedAdd(stats + NUM_UNIQUE_IPS, 1);
    }



    // ***** Connect
    bool isCodeValid = true;
    if (!Connect(server, host, "/robots.txt", "HEAD", "4XX", MAX_ROBOT_PAGE_SIZE, false, isCodeValid, false, stats)) {
        return false;
    }
    if (!isCodeValid) return true;
    InterlockedAdd(stats + NUM_ROBOTS_PASSED, 1);

    if (!Connect(server, host, request, "GET", "2XX", MAX_PAGE_SIZE, true, isCodeValid, false, stats)) {
        return false;
    }
    if (!isCodeValid) return true;
    InterlockedAdd(stats + NUM_CRAWLED_URLS, 1);


    char* findTamu = strstr(host, "tamu.edu");
    char* expectedAddress = host + strlen(host) - 8;

    if ( !findTamu || (findTamu != expectedAddress) )
    {

    }
    else {
        InterlockedAdd(stats + TAMU_COUNT, 1);
    }
    

    // ***** page parsing
    char* response_break = strstr(this->buf, "\r\n\r\n");
    start = clock();
    int nlinks;
    html_parser.Parse(response_break + 4, static_cast<int>(strlen(response_break + 4)), inputURL, static_cast<int>(strlen(inputURL)), &nlinks);

    if (nlinks < 0) {
        return false;
    }
    end = clock();
    elapsed_time = (double)(end - start) / CLOCKS_PER_SEC * 1000;
    *response_break = '\0';
    InterlockedAdd(stats + TOTAL_LINKS, nlinks);

    return true;
}
