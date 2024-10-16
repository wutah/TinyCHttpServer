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


