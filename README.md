## Project Overview
This project is designed to enhance understanding and practical application of text-based application-layer protocols, multithreading, system APIs, and Windows sockets. It consists of a series of tasks that extend a basic web client into a multi-threaded web crawler capable of handling complex URL formats, managing network communication, and parsing web content.

## Features
* URL Handling: The program can process various URL formats, including handling different schemes, ports, paths, queries, and fragments.
* Web Client Implementation: Using Visual C++, a web client is developed to crawl URLs and fetch page data while displaying server and page statistics.
* Error Handling: Robust error handling is implemented to manage different network errors and server responses, providing clear feedback on issues like non-HTTP responses or connection timeouts.
* Dynamic Buffer Management: The client uses a dynamic buffer that expands as needed to accommodate page data of arbitrary length.
* Multi-threading: The software scales to handle multiple URLs simultaneously using multithreading, improving the efficiency and speed of data processing.
* Politeness and Efficiency: The crawler respects robots.txt standards and includes functionality to avoid overloading servers, managing unique IP hits and DNS lookups efficiently.
* Detailed Statistics: Outputs detailed runtime statistics such as DNS lookups, unique IP checks, and HTTP status codes, which help in evaluating the crawler's performance.
* Performance Metrics: Includes timed breakdowns of each operation, from DNS resolution to page loading and parsing, aiding in performance optimization.

## Technical Specifications
* Languages: Implemented in Visual C++.
* Environment: Developed to run on Windows using Windows sockets.
* Dependencies: Requires libraries for handling HTTP requests and parsing HTML content.

## Usage
The program is executed via the command line, where users can input specific URLs or a file containing multiple URLs. It supports different operational modes, including single URL processing and batch URL crawling with specified thread counts.
