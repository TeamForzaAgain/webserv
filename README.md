# Webserv (42 School)

A custom HTTP/1.1 web server built from scratch in C++98 as part of the 42 curriculum.

This project recreates core web server behaviors without external frameworks: request parsing, routing, virtual hosts, static file serving, CGI execution, uploads, and non-blocking I/O with poll.

## Why This Project Matters

Webserv demonstrates low-level systems programming skills applied to real networking problems:

- Event-driven server architecture with non-blocking sockets
- HTTP protocol handling (methods, headers, body, status codes, errors)
- Configuration-driven behavior (Nginx-style server/location blocks)
- Process management for CGI and signal-safe server shutdown
- Robustness checks (syntax + logic validation of configuration)

## Key Features

- HTTP methods: GET, POST, DELETE
- Virtual hosting via Host header on shared listen sockets
- Non-blocking multiplexing with poll
- Per-location routing with longest-prefix match
- Static file serving with MIME type detection
- Autoindex directory listing
- Custom error pages (301, 403, 404, 405, 415, 500)
- Redirects via return directive (example: 301)
- Multipart/form-data uploads
- File deletion endpoint
- CGI execution for .py and .sh scripts (fork + execve)
- CGI watchdog timeout process to prevent hanging scripts
- Cookie-based session tracking
- Request body support:
  - Content-Length
  - Transfer-Encoding: chunked
- Keep-Alive / Connection: close handling
- Graceful shutdown on SIGINT, SIGTERM, SIGHUP

## Architecture Overview

The project is organized by responsibility:

- parsing/
  - lexical/syntax checks (tokens, braces, semicolons)
  - semantic checks (ports, methods, booleans, body size, return)
  - conversion from config text to runtime server structures
- serverFiles/
  - server behavior, routing, response generation
  - error handling, file reading, autoindex, content types
- socketFiles/
  - listening sockets, client sockets, non-blocking mode
- HTTP_Methods/
  - GET/POST/DELETE implementations
- bonus/
  - CGI execution and session manager

Execution flow:

1. Load and validate configuration file
2. Build ServerConfig objects
3. Create listening sockets and virtual servers
4. Enter poll loop
5. Accept clients, parse requests, generate responses
6. Handle keep-alive/timeout/close and repeat

## Tech Stack

- Language: C++98
- Build: Makefile + g++
- System APIs: POSIX sockets, poll, fcntl, fork/execve, waitpid, signals
- Target environment: Unix/Linux (or WSL on Windows)

## Build and Run

```bash
make
./webserv configurationFiles/webserv.conf
```

Default sample config includes multiple servers and ports, for example:

- localhost:8080
- localhost:7070

## Quick Test Commands

```bash
# Basic GET
curl -i http://localhost:8080/

# Redirect example
curl -i http://localhost:7070/

# Session cookie example
curl -i http://localhost:8080/session/

# Multipart upload
curl -i -X POST -F "file=@README.md" http://localhost:8080/upload/

# Delete uploaded file (adjust filename/path)
curl -i -X DELETE http://localhost:8080/delete/uploaded_file.bin
```

## Configuration Highlights

The provided config supports directives such as:

- server, location
- listen, server_name, root
- methods, index, auto_index
- upload, alias, cgi
- error_page, max_body_size
- return
