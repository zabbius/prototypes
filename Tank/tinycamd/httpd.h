#ifndef HTTPD_IS_IN
#define HTTPD_IS_IN

#include <pthread.h>
#include <netinet/in.h>

typedef struct http_request *HTTPD_Request;

pthread_t HTTPD_Start(const char *bindName, void (*func)(HTTPD_Request req, const char *method, const char *url) );

void HTTPD_Send_Status( HTTPD_Request req, int status, const char *text);      // optional, will be sent as 200 if you try to skip it
void HTTPD_Add_Header( HTTPD_Request req, const char *h);  // optional
void HTTPD_Send_Body( HTTPD_Request req, const void *data, int length);
int  HTTPD_Send_Buffer( HTTPD_Request req, const void *buf, int len);
void HTTPD_ResetWatchdog( HTTPD_Request req);

void* HTTPD_AllocData( HTTPD_Request req, size_t size );
void* HTTPD_GetData(HTTPD_Request req);
void HTTPD_DeleteData(HTTPD_Request req);

struct sockaddr_in HTTPD_GetRemoteAddr(HTTPD_Request req);

const char *HTTPD_Get_Authorization( HTTPD_Request req);  // NULL if none given

#endif
