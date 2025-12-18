# TinyCHttpServer
Tiny Http Server in C for serving static files. 

Features
--------

1. Basic MIME mapping
2. Directory listing
3. Low resource usage
4. Support only GET requests
5. Support Accept-Ranges: bytes

Compile
-------
Compile with [tcc](http://www.tinycc.org/):

`tcc -c source/TinyCHttpServer.c`

`tcc -o TinyCHttpServer.exe TinyCHttpServer.o -lws2_32`

Run
----
`TinyCHttpServer.exe [port_number]`



----
----
----
This C code implements a very basic HTTP server on Windows using Winsock. While functional for simple file serving, it contains multiple serious security vulnerabilities and reliability issues. Below is a comprehensive list of the most critical problems:1. Directory Traversal / Path Traversal Vulnerability (Critical)Issue: The code constructs the file path by directly concatenating "./serverroot" with user-controlled input from the HTTP request URI, but performs no proper path normalization or restriction.
Exploitation: An attacker can send requests like GET /../../windows/system32/drivers/etc/hosts HTTP/1.1 to read arbitrary files on the system (limited only by the process's permissions).
Why it works: The parsing stops at space, ?, *, or :, but allows .. sequences.
Only minimal %20 decoding is done — no handling of %2e (.) or other encoded traversal sequences.
No realpath()-like normalization or check that the final path stays within ./serverroot.

Impact: Arbitrary file disclosure (including sensitive system/configuration files).

2. Buffer Overflow Vulnerabilities (Critical)Multiple fixed-size buffers are used without sufficient bounds checking:char uri[65536], req[65536], request[65536], text[65536], buf[65536] — all vulnerable to overflow if assumptions are broken.
Specific cases:URI parsing loop: uri[p2++] = tolower(req[p]) — no check that p2 < 65536. If the path component is >64KB (unlikely in practice but possible), overflow occurs.
sprintf(uri + p, "index.html") when directory requested — if p is near buffer end, overflow.
Many sprintf calls into request, buf, text assume the formatted output fits (e.g., directory listings with many/long filenames can overflow).

Impact: Potential code execution if attacker controls input size/content.

3. Incomplete/No URL DecodingOnly decodes %20 → space.
Does not decode %2F (/), %2E (.), %5C (), %00, etc.
This partially mitigates traversal (can't easily inject / via encoding), but is inconsistent and allows some bypasses (e.g., double encoding or unusual chars).

4. Insecure Directory ListingWhen a directory is requested (ends with /), it automatically serves index.html.
If no index, it generates and sends a full directory listing with filenames and sizes.
Risk: Information disclosure — exposes entire directory structure and file sizes.

5. No HTTP Request ValidationAssumes the request starts with GET /something HTTP/.
Skips to first / with a hardcoded p2 = 12 — fragile and wrong for non-standard requests.
Does not validate HTTP method (accepts anything), version, or headers properly.
No handling of malformed requests → potential crashes or undefined behavior.

6. Range Request Handling VulnerabilitiesExtremely complex custom parsing of Range header with many bugs:Uses fixed buffers without bounds checking.
Incorrect parsing logic (e.g., prepends/appends commas, manual sscanf).
Buffer overflows possible with long/malformed Range headers.
No validation of range bounds → negative or huge values could cause crashes.
Merging overlapping ranges has off-by-one/logic errors.

Supports multipart ranges but with flawed boundary generation and formatting.

7. Denial of Service (DoS) RisksSingle-threaded accept loop → one slow client (slow loris) blocks all others.
No limit on concurrent connections.
Timeout set via SO_RCVTIMEO/SO_SNDTIMEO, but only after accept — initial recv can still hang.
Large file transfers use fixed 64KB buffer — inefficient and no backpressure handling.

8. Lack of Access Control and AuthenticationServes any file under ./serverroot (and beyond, due to traversal) to anyone.
No authentication, IP filtering, or access controls.

9. Information DisclosureServer banner: Server: TinyCHttpServer
Error pages and directory listings include current date/time and rand() output.
Exposes internal paths in responses.

10. Resource Leak and Error Handling Issuesfopen(..., "rb") → if fails, fclose(fp) is called with fp == NULL → undefined behavior (should check before closing).
No cleanup on early errors in some paths.
Uses rand() without srand() → predictable values (though seeded implicitly by time in some environments).

11. Other Minor IssuesBinds to 0.0.0.0 → listens on all interfaces (may be unintended).
Uses atoi(argv[1]) without bounds checking → potential crash if no/invalid arg.
Hardcoded root ./serverroot — no chroot or sandboxing.
No logging to file — only console output.

Summary of SeverityVulnerability
Severity
Exploitability
Directory Traversal
Critical
High
Buffer Overflows
Critical
Medium-High
Directory Listing
High
High
DoS (slowloris, resource)
Medium
High
Info Disclosure
Medium
High

RecommendationDo not use this code in production or on any exposed network.
It is suitable only as a learning example in a controlled, isolated environment.To fix it properly would require a near-complete rewrite with:Proper path normalization (e.g., using GetFullPathName + prefix check)
Full URL decoding
Bounds-checked string handling (or snprintf)
Request validation
Threading or async I/O
Removal of auto-directory listing

If you need a tiny HTTP server, consider using established libraries like mongoose, civetweb, or even Python's http.server for simple cases.

