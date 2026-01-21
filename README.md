# http-web-server

DRAFT 1:
Creating a http web server using raw sockets and C language.

This server can:
- Accept browser connections
- Read HTTP requests
- Serve an HTML file from disk
- Return '404 Not Found' for unknown pages
- Handle multiple clients as the same time

I used:
- C language
- TCP sockets
- HTTP protocols
- File I/O
- Fork( to handle multiple requests)


DRAFT 2:

## Server-side changes for Android connectivity

- Parsed HTTP request path to support API-style routing
- Added `GET /health` endpoint for Android client testing
- Returned JSON response with correct HTTP headers
- Ensured proper connection handling for Android HTTP requests
- Used plain HTTP for local emulator compatibility




Thank you...
