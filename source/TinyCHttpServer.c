#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>
#include <windows.h>
#include <winsock.h>
#include <winsock2.h>
#include <winuser.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")  // Winsock Library

int main(int argc, char* argv[]) {
 int port;
 port = atoi(argv[1]);

 if (port < 1 || port > 65535) {
  port = 55555;
 }

 // Initialize WSA variables
 WSADATA wsaData;
 int wsaerr;
 WORD wVersionRequested = MAKEWORD(2, 2);
 wsaerr = WSAStartup(wVersionRequested, &wsaData);

 // Check for initialization success
 if (wsaerr != 0) {
  printf("The Winsock dll not found!\n");
  return 0;
 } else {
  printf("The Winsock dll found\n");
 }

 SOCKET serverSocket;
 serverSocket = INVALID_SOCKET;  // Initializing as a inivalid socket
 serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

 if (serverSocket == INVALID_SOCKET) {
  printf("Error at socket\n");
  WSACleanup();
  return 0;
 } else {
  printf("Socket is ok\n");
 }

 // Bind the socket to an IP address and port number
 SOCKADDR_IN service;
 service.sin_family = AF_INET;
 service.sin_addr.s_addr = inet_addr("0.0.0.0");  // Desired IP address
 service.sin_port = htons(port);                  // Port number

 int iResult = 0;
 iResult = bind(serverSocket, (SOCKADDR*)&service, sizeof(service));

 if (iResult == SOCKET_ERROR) {
  printf("Bind failed with error %u\n", WSAGetLastError());
  closesocket(serverSocket);
  WSACleanup();
  return 1;
 } else {
  printf("Bind returned success\n");
 }

 if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
  printf("Listen function failed with error: %d\n", WSAGetLastError());
 }
 printf("Listening on http://127.0.0.1:%d\n", port);

 // Create a SOCKET for accepting incoming requests.
 SOCKET AcceptSocket;
 unsigned long long counter = 0;
 while (1) {
  printf("Waiting for client to connect ...\n");

  AcceptSocket = accept(serverSocket, NULL, NULL);

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  if (AcceptSocket == INVALID_SOCKET) {
   printf("%d-%02d-%02d %02d:%02d:%02d - Accept failed with error: %ld\n\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, WSAGetLastError());
   continue;
  } else {
   printf("%d-%02d-%02d %02d:%02d:%02d - Client connected!\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  }

  printf("Conn. counter=%llu\n", counter++);

  char req[65536];
  char request[65536];

  DWORD timeout = 1500;
  setsockopt(AcceptSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
  // timeout = 1000;
  setsockopt(AcceptSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

  // setsockopt(AcceptSocket, SOL_SOCKET, SO_KEEPALIVE,(char*)&timeout, sizeof(timeout));

  int bytes_recvd = recv(AcceptSocket, req, 65535, 0);

  if (bytes_recvd > 65534 || bytes_recvd < 1) {
   printf("Reading error! Socket Closed.\n\n");
   closesocket(AcceptSocket);
   continue;
  }

  char uri[65536] = "./serverroot";  // Root web directory

  printf("Recieved req=%d bytes.\n", bytes_recvd);

  int p = 0;
  while (req[p] != '/' && p < bytes_recvd) {
   p++;
  }
  int p2 = 12;
  while (req[p] != ' ' && req[p] != '?' && req[p] != '*' && req[p] != ':' && p < bytes_recvd) {
   if (req[p] == '%' && req[p + 1] == '2' && req[p + 2] == '0') {
    p = p + 2;
    uri[p2++] = ' ';
   } else {
    uri[p2++] = tolower(req[p]);
   }

   p++;
  }

  bool IsDirRequested = false;
  p = strlen(uri);
  if (uri[p - 1] == '/') {
   IsDirRequested = true;
   sprintf(uri + p, "%s", "index.html");
  }
  printf("Requested file: %s\n", uri);

  char text[65536];
  FILE* fp;
  fp = fopen(uri, "rb");
  char* fname = strrchr(uri, '/');
  fname++;
  if (fp == NULL) {
   fclose(fp);
   printf("FILE NOT FOUND!\n");

   WIN32_FIND_DATA data;
   char buf[65536];

   if (IsDirRequested) {
    uri[p] = '*';
    uri[p + 1] = '\0';
    HANDLE hFind = FindFirstFile(uri, &data);
    uri[p] = '\0';
    p = 0;
    if (hFind != INVALID_HANDLE_VALUE) {
     // memset(text, 0, sizeof(text));memset(buf, 0, sizeof(buf));memset(request, 0, sizeof(request));

     printf("Creating listing of %s ...\n", uri);
     printf("Send: HTTP/1.1 200 OK ...\n");

     sprintf(request, "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache\r\nServer: TinyCHttpServer\r\nAccept-ranges: bytes\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n");
     send(AcceptSocket, request, strlen(request), 0);

     sprintf(request, "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link href=\"data:image/x-icon;base64,AAABAAEAEBAAAAEAIABoBAAAFgAAACgAAAAQAAAAIAAAAAEAIAAAAAAAAAQAABILAAASCwAAAAAAAAAAAAD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYAAAD//wAA//8AAP//AAD//wAA///0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAAAA//8AAP//AAD//wAA//8AAP//9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgAAAP//AAD//wAA//8AAP//AAD///SkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYAAAD//wAA//8AAP//AAD//wAA///0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAAAA//8AAP//AAD//wAA//8AAP//9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgAAAP//AAD//wAA//8AAP//AAD///SkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYAAAD//wAA//8AAP//AAD//wAA///0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAAAA//8AAP//AAD//wAA//8AAP//9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgAAAP//AAD//wAA//8AAP//AAD///SkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYAAAD//wAA//8AAP//AAD//wAA///0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgAAAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD///SkxgD0pMYAAAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA///0pMYA9KTGAAAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//9KTGAPSkxgAAAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD///SkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA//8AAPwfAAD8HwAA/B8AAPwfAAD8HwAA/B8AAPwfAAD8HwAA/B8AAPwfAACAAQAAgAEAAIABAACAAQAA//8AAA==\" rel=\"icon\" type=\"image/x-icon\"><style>html {line-break: anywhere;word-wrap: break-word;height: 100%%;padding: 0;margin: 0;}body {text-align: center;max-width: 920px;margin: auto;border-color: #bf4f00;border-style: solid;}b {color:#bf4f00;}</style><title>TinyCHttpServer: Index of %s</title></head><body><br>date time now: %d-%02d-%02d %02d:%02d:%02d\n<br>and random number: %d<p><h1><b>Index of %s</b></h1><p><a href=\"..\">..</a>\n", uri + 12, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, rand(), uri + 12);

     sprintf(buf, "\r\n%x\r\n%s\r\n", strlen(request), request);
     send(AcceptSocket, buf, strlen(buf), 0);

     do {
      if (strcmp(data.cFileName, ".") != 0 && strcmp(data.cFileName, "..") != 0) {  // memset(text, 0, sizeof(text));memset(buf, 0, sizeof(buf));
       sprintf(text, "<p><a href=\"%s\">%s</a> - %d\n", data.cFileName, data.cFileName, data.nFileSizeLow);
       sprintf(buf, "%x\r\n%s\r\n", strlen(text), text);
       send(AcceptSocket, buf, strlen(buf), 0);
      }
     } while (FindNextFile(hFind, &data));
     FindClose(hFind);

     sprintf(text, "<br><br><a href=\"/\">Back to root</a><p><b>TinyCHttpServer</b></body></html>");
     sprintf(request, "%x\r\n%s\r\n", strlen(text), text);
     send(AcceptSocket, request, strlen(request), 0);

     send(AcceptSocket, "0\r\n\r\n", 5, 0);
     closesocket(AcceptSocket);
     printf("Socket Closed.\n\n");
     continue;
    }

   } else {
    uri[p] = '/';
    uri[p + 1] = '*';
    uri[p + 2] = '\0';
    HANDLE hFind = FindFirstFile(uri, &data);
    uri[p] = '/';
    uri[p + 1] = '\0';
    p = 0;
    if (hFind != INVALID_HANDLE_VALUE) {
     // do{p++;if(p>2){break;}}while(FindNextFile(hFind, &data));
     FindClose(hFind);

     printf("DIR FOUND!\nSend: HTTP/1.1 301 Moved Permanently ...\n");
     // sprintf(request,"<html><head><title>301 Moved Permanently</title></head><body><center><h1>301 Moved Permanently</h1></center><hr><center>TinyCHttpServer</center></body></html>");

     sprintf(buf, "HTTP/1.1 301 Moved Permanently\r\nContent-Type: text/html; charset=UTF-8\r\nLocation: %s\r\nCache-Control: no-cache\r\nServer: TinyCHttpServer\r\nAccept-ranges: bytes\r\nContent-Length: 158\r\nConnection: close\r\n\r\n<html><head><title>301 Moved Permanently</title></head><body><center><h1>301 Moved Permanently</h1></center><hr><center>TinyCHttpServer</center></body></html>", uri + 12);
     send(AcceptSocket, buf, strlen(buf), 0);
     closesocket(AcceptSocket);
     printf("Socket Closed.\n\n");
     continue;
    }
   }

   sprintf(text, "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link href=\"data:image/x-icon;base64,AAABAAEAEBAAAAEAIABoBAAAFgAAACgAAAAQAAAAIAAAAAEAIAAAAAAAAAQAABILAAASCwAAAAAAAAAAAAD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYAAAD//wAA//8AAP//AAD//wAA///0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAAAA//8AAP//AAD//wAA//8AAP//9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgAAAP//AAD//wAA//8AAP//AAD///SkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYAAAD//wAA//8AAP//AAD//wAA///0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAAAA//8AAP//AAD//wAA//8AAP//9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgAAAP//AAD//wAA//8AAP//AAD///SkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYAAAD//wAA//8AAP//AAD//wAA///0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAAAA//8AAP//AAD//wAA//8AAP//9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgAAAP//AAD//wAA//8AAP//AAD///SkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYAAAD//wAA//8AAP//AAD//wAA///0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgAAAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD///SkxgD0pMYAAAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA///0pMYA9KTGAAAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//9KTGAPSkxgAAAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD///SkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA9KTGAPSkxgD0pMYA//8AAPwfAAD8HwAA/B8AAPwfAAD8HwAA/B8AAPwfAAD8HwAA/B8AAPwfAACAAQAAgAEAAIABAACAAQAA//8AAA==\" rel=\"icon\" type=\"image/x-icon\"><style>html {line-break: anywhere;word-wrap: break-word;height: 100%%;padding: 0;margin: 0;}body {text-align: center;max-width: 920px;margin: auto;border-color: #bf4f00;border-style: solid;}b {color:#bf4f00;}</style><title>TinyCHttpServer: 404 Not Found</title></head><body><br>date time now: %d-%02d-%02d %02d:%02d:%02d\n<br>and random number: %d<p><h1><b>404 Page not found</b></h1><br><a href=\"/\">Back to root</a><p><b>TinyCHttpServer</b></body></html>\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, rand());

   sprintf(request, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html; charset=UTF-8\r\nCache-Control: no-cache\r\nAccept-ranges: bytes\r\nServer: TinyCHttpServer\r\nContent-Length: %d\r\nConnection: close\r\n\r\n%s", strlen(text), text);
   printf("Send: HTTP/1.1 404 Not Found ...\n");
   send(AcceptSocket, request, strlen(request), 0);

  } else {
   char* ext = strrchr(fname, '.');
   char mimetype[32] = "application/octet-stream";  // Default mimetype
   if (ext != NULL) {
    ext++;

    if (strcmp(ext, "txt") == 0) {
     strcpy(mimetype, "text/plain; charset=UTF-8");
    } else {
     if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0 || strcmp(ext, "xhtml") == 0) {
      strcpy(mimetype, "text/html; charset=UTF-8");
     } else {
      if (strcmp(ext, "jpeg") == 0 || strcmp(ext, "jpg") == 0 || strcmp(ext, "jfif") == 0) {
       strcpy(mimetype, "image/jpg");
      } else {
       if (strcmp(ext, "css") == 0) {
        strcpy(mimetype, "text/css");
       } else {
        if (strcmp(ext, "js") == 0) {
         strcpy(mimetype, "application/javascript");
        } else {
         if (strcmp(ext, "json") == 0) {
          strcpy(mimetype, "application/json");
         } else {
          if (strcmp(ext, "gif") == 0) {
           strcpy(mimetype, "image/gif");
          } else {
           if (strcmp(ext, "png") == 0) {
            strcpy(mimetype, "image/png");
           } else {
            if (strcmp(ext, "ico") == 0) {
             strcpy(mimetype, "image/x-icon");
            } else {
             if (strcmp(ext, "webm") == 0) {
              strcpy(mimetype, "video/webm");
             } else {
              if (strcmp(ext, "mp4") == 0 || strcmp(ext, "mp4v") == 0 || strcmp(ext, "mpg4") == 0) {
               strcpy(mimetype, "video/mp4");
              } else {
               if (strcmp(ext, "mp3") == 0 || strcmp(ext, "m2a") == 0 || strcmp(ext, "m3a") == 0 || strcmp(ext, "mp2") == 0 || strcmp(ext, "mp2a") == 0 || strcmp(ext, "mpga") == 0) {
                strcpy(mimetype, "audio/mpeg");
               } else {
                if (strcmp(ext, "svg") == 0 || strcmp(ext, "svgz") == 0) {
                 strcpy(mimetype, "image/svg+xml");
                } else {
                 if (strcmp(ext, "wbmp") == 0) {
                  strcpy(mimetype, "image/vnd.wap.wbmp");
                 } else {
                  if (strcmp(ext, "webp") == 0) {
                   strcpy(mimetype, "image/webp");
                  }
                 }
                }
               }
              }
             }
            }
           }
          }
         }
        }
       }
      }
     }
    }
   }

   fseek(fp, 0, SEEK_END);
   long long int sz = ftell(fp);
   rewind(fp);

   bool IsRangeFound = false;
   char range[1024];
   range[0] = '\0';
   while (p < bytes_recvd) {
    p++;

    while (req[p] != '\n' && req[p + 1] != '\n' && p < bytes_recvd) {
     range[p2] = req[p++];
     p2++;
     range[p2] = '\0';
    }

    if (range[0] == 'R' && range[1] == 'a' && range[2] == 'n' && range[3] == 'g' && range[4] == 'e') {
     p = bytes_recvd;
     IsRangeFound = true;
     break;
    }
    p2 = 0;
   }

   if (IsRangeFound) {
    strcpy(range, (range + 12));  // printf("range: %s\n",range);
    printf("RANGE request received: ");
    long long int ranges[255][2];
    memset(ranges, 0, sizeof(ranges));
    range[0] = ',';
    p2 = strlen(range);
    p = 0;
    range[p2] = ',';
    char rangebuf[64];
    rangebuf[0] = '\0';
    int p3 = 0, rangecnt = 0;
    while (p < p2) {
     p++;
     rangebuf[p3] = range[p];
     p3++;
     rangebuf[p3] = '\0';
     if (range[p] == ',') {
      rangebuf[strlen(rangebuf) - 1] = '\0';
      sscanf(rangebuf, "%lld-%lld", &ranges[rangecnt][1], &ranges[rangecnt][2]);
      p3 = 0;
      rangecnt++;
     }
    }

    for (p = 0; p < rangecnt; p++) {
     long long int temp1;

     if (ranges[p][1] == 0 && ranges[p][2] == 0) {
      ranges[p][2] = sz - 1;
     } else {
      if (ranges[p][2] <= 0) {
       ranges[p][2] = (sz + ranges[p][2]) - 1;
      }
      if (ranges[p][1] <= 0) {
       ranges[p][1] = (sz + ranges[p][1]);
      }
     }

     if (ranges[p][1] > sz) {
      ranges[p][1] = sz;
     }
     if (ranges[p][2] > sz) {
      ranges[p][2] = sz;
     }
     // if(ranges[p][1]<0){ranges[p][1]=0;}
     // if(ranges[p][2]<0){ranges[p][2]=0;}
     if (ranges[p][2] < ranges[p][1]) {
      temp1 = ranges[p][1];
      ranges[p][1] = ranges[p][2];
      ranges[p][2] = temp1;
     }
    }

    for (int i = 0; i < rangecnt - 1; i++) {
     for (int j = 0; j < rangecnt - i - 1; j++) {
      // Swap if the element found is greater than the
      // next element
      if (ranges[j][1] > ranges[j + 1][1]) {
       long long int temp1 = ranges[j][1];
       long long int temp2 = ranges[j][2];
       ranges[j][1] = ranges[j + 1][1];
       ranges[j][2] = ranges[j + 1][2];
       ranges[j + 1][1] = temp1;
       ranges[j + 1][2] = temp2;
      }
     }
    }

    int index = 0;  // Stores index of last element
    // in output array (modified arr[])
    int n = rangecnt;
    // Traverse all input Intervals
    for (int i = 1; i < n; i++) {
     // If this is not first Interval and overlaps
     // with the previous one
     if (ranges[index][2] >= ranges[i][1]) {
      // Merge previous and current Intervals
      if (ranges[index][2] <= ranges[i][2]) {
       ranges[index][2] = ranges[i][2];
      }
      rangecnt--;
     } else {
      index++;
      ranges[index][1] = ranges[i][1];
      ranges[index][2] = ranges[i][2];
     }
    }

    // for(p=0;p<rangecnt; p++){printf("start=%d, end=%d\n",ranges[p][1],ranges[p][2]);}

    // printf("rangecnt=%d\n",rangecnt);

    if (rangecnt > 1) {
     // Multiple ranges
     printf("Multiple ranges ...\n");

     char boundary[16];
     sprintf(boundary, "%x%x", 32 * rand() + 128, 4 * rand() + 1024);
     request[0] = '\0';

     sprintf(request, "HTTP/1.1 206 Partial Content\r\nContent-Type: multipart/byteranges; boundary=%s\r\nCache-Control: no-cache\r\nServer: TinyCHttpServer\r\nAccept-ranges: bytes\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n", boundary);
     printf("Send: HTTP/1.1 206 Partial Content ...\n");

     send(AcceptSocket, request, strlen(request), 0);

     for (int i = 0; i < rangecnt; i++) {
      long int to_read = 0;
      to_read = ranges[i][2] - ranges[i][1] + 1;
      memset(request, 0, sizeof(request));
      memset(text, 0, sizeof(text));

      sprintf(request, "\r\n--%s\r\nContent-type: %s\r\nContent-range: bytes %lld-%lld/%lld\r\n\r\n", boundary, mimetype, ranges[i][1], ranges[i][2], sz);
      sprintf(text, "\r\n%x\r\n%s", strlen(request) + to_read, request);
      send(AcceptSocket, text, strlen(text), 0);

      fseek(fp, ranges[i][1], SEEK_SET);

      // sz=to_read;

      p = (to_read / 65536);
      printf("Filesize=%lld bytes\n", to_read);
      for (p2 = 0; p2 < p; p2++) {
       printf("Sending file: %d/%d ...\r", p2 + 1, p);
       fread(request, 65536, 1, fp);
       if (send(AcceptSocket, request, 65536, 0) == -1) {
        break;
       }
      }

      if (p > 0) {
       if (p2 >= p) {
        printf("\nFile sent successfully!\n");
       } else {
        printf("\nFile transfer interrupted.\n");
       }
      } else {
       printf("File sent successfully!\n");
      }
      memset(request, 0, sizeof(request));
      memset(text, 0, sizeof(text));
      p = to_read - (p * 65536);
      fread(request, p, 1, fp);
      send(AcceptSocket, request, p, 0);
     }

     memset(request, 0, sizeof(request));
     memset(text, 0, sizeof(text));
     sprintf(request, "\r\n--%s--\r\n", boundary);
     sprintf(text, "\r\n%x\r\n%s\r\n", strlen(request), request);
     send(AcceptSocket, text, strlen(text), 0);

     send(AcceptSocket, "0\r\n\r\n", 9, 0);

     fclose(fp);
     closesocket(AcceptSocket);
     printf("Socket Closed.\n\n");
     continue;

    } else {
     // One range
     printf("One range ...\n");

     long int to_read = 0;
     to_read = ranges[0][2] - ranges[0][1] + 1;

     sprintf(request, "HTTP/1.1 206 Partial Content\r\nContent-Type: %s\r\nCache-Control: no-cache\r\nServer: TinyCHttpServer\r\nAccept-ranges: bytes\r\nContent-Length: %lld\r\nContent-Range: bytes %lld-%lld/%lld\r\nConnection: close\r\n\r\n", mimetype, to_read, ranges[0][1], ranges[0][2], sz);
     printf("Send: HTTP/1.1 206 Partial Content ...\n");
     send(AcceptSocket, request, strlen(request), 0);

     fseek(fp, ranges[0][1], SEEK_SET);
     sz = to_read;

     p = (sz / 65536);
     printf("Filesize=%lld bytes\n", sz);
     for (p2 = 0; p2 < p; p2++) {
      printf("Sending file: %d/%d ...\r", p2 + 1, p);
      fread(request, 65536, 1, fp);
      if (send(AcceptSocket, request, 65536, 0) == -1) {
       break;
      }
     }

     if (p > 0) {
      if (p2 >= p) {
       printf("\nFile sent successfully!\n");
      } else {
       printf("\nFile transfer interrupted.\n");
      }
     } else {
      printf("File sent successfully!\n");
     }

     p = sz - (p * 65536);
     fread(request, p, 1, fp);
     send(AcceptSocket, request, p, 0);

     fclose(fp);
     closesocket(AcceptSocket);
     printf("Socket Closed.\n\n");
     continue;
    }

   } else {
    sprintf(request, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nCache-Control: no-cache\r\nServer: TinyCHttpServer\r\nAccept-ranges: bytes\r\nContent-Length: %lld\r\nConnection: close\r\n\r\n", mimetype, sz);
    printf("Send: HTTP/1.1 200 OK ...\n");
    send(AcceptSocket, request, strlen(request), 0);

    p = (sz / 65536);
    printf("Filesize=%lld bytes\n", sz);
    for (p2 = 0; p2 < p; p2++) {
     printf("Sending file: %d/%d ...\r", p2 + 1, p);
     fread(request, 65536, 1, fp);
     if (send(AcceptSocket, request, 65536, 0) == -1) {
      break;
     }
    }

    if (p > 0) {
     if (p2 >= p) {
      printf("\nFile sent successfully!\n");
     } else {
      printf("\nFile transfer interrupted.\n");
     }
    } else {
     printf("File sent successfully!\n");
    }

    p = sz - (p * 65536);
    fread(request, p, 1, fp);
    send(AcceptSocket, request, p, 0);

    fclose(fp);
   }
  }

  closesocket(AcceptSocket);
  printf("Socket Closed.\n\n");
 }

 return 0;
}