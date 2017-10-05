/*
 * threaded.c -- A simple multi-threaded HTTPD application.
 */

#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include<arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <jpeglib.h>
#include <pwd.h>

#include "tinycamd.h"
#include "httpd.h"

extern char setup_html[];
extern int setup_html_size;
extern char tinycamd_js[];
extern int tinycamd_js_size;
extern char jquery_min_js[];
extern int jquery_min_js_size;
extern char tinycamd_css[];
extern int tinycamd_css_size;


#define BOUNDARY "myboundary"

static const char *boundary = "--" BOUNDARY "\r\n";
static const char *crlf = "\r\n";

static const int udp_frame_size = 1450;
static const int fps_multiplier = 100;

typedef struct 
{
    int socket;
    struct sockaddr_in addr;
    unsigned char* buffer;
    size_t buffer_size;
} udp_params;


static int jpeg_compress( const struct chunk *c, unsigned char *jpegBuffer, int jpegLeft )
{
    int jpegSize = 0;

    struct jpeg_compress_struct cinfo = { .dest = 0};
    struct jpeg_destination_mgr dmgr;
    struct jpeg_error_mgr err;
    
    void init_destination(j_compress_ptr cinfo) 
    {
        struct jpeg_destination_mgr *d = cinfo->dest;
        d->next_output_byte = jpegBuffer;
        d->free_in_buffer = jpegLeft;
    }
    
    int empty_output_buffer(j_compress_ptr cinfo) 
    {
        //struct jpeg_destination_mgr *d = cinfo->dest;
        log_f("eob\n");
        return  TRUE;
    }
    
    void term_destination(j_compress_ptr cinfo) 
    {
        struct jpeg_destination_mgr *d = cinfo->dest;
        log_f("termdest\n");
        jpegSize = d->next_output_byte - jpegBuffer;
    }
    
    dmgr.init_destination = init_destination;
    dmgr.empty_output_buffer = empty_output_buffer;
    dmgr.term_destination = term_destination;

    cinfo.err = jpeg_std_error(&err);
    jpeg_create_compress(&cinfo);
    cinfo.image_width = video_width;
    cinfo.image_height = video_height;
    
    if ( mono) 
    {
        cinfo.input_components = 1;
        cinfo.in_color_space = JCS_GRAYSCALE;
    } 
    else 
    {
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_YCbCr;
    }
    
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    cinfo.dest = &dmgr;

    jpeg_start_compress( &cinfo, TRUE);
    {
        const unsigned char *b = c[0].data;
        int row = 0;
        int col = 0;
        JSAMPLE pix[video_width*3];
        JSAMPROW rows[] = { pix};
        JSAMPARRAY scanlines = rows;

        for ( row = 0; row < video_height; row++) 
        {
            JSAMPLE *p = pix;
            for ( col = 0; col < video_width; col+=2) 
            {
                *p++ = b[0];
                if (!mono) 
                {
                    *p++ = b[1];
                    *p++ = b[3];
                }
                
                *p++ = b[2];
                
                if (!mono) 
                {
                    *p++ = b[1];
                    *p++ = b[3];
                }
                b += 4;
            }
            jpeg_write_scanlines( &cinfo, scanlines, 1);
        }
    }
    jpeg_finish_compress( &cinfo);
    
    return jpegSize;
}


static void do_status_request( HTTPD_Request req)
{
    static const char *status = "<html><head><title>Status</title></head>"
                                "<body>status</body>"
                                "</html>";

    HTTPD_Add_Header( req, "Content-type: text/html");
    HTTPD_Send_Body(req, status,strlen(status));
}

static void put_image_stub( struct chunk *c, void *arg)
{
}

static void put_image( struct chunk *c, void *arg)
{
    HTTPD_Request req = (HTTPD_Request)arg;
    
    int i,s=0;

    char buf[1024];

    switch(camera_method) 
    {
    case CAMERA_METHOD_MJPEG:
    case CAMERA_METHOD_JPEG:
        {
            for ( i = 0; c[i].data != 0; i++) 
                s += c[i].length;

            snprintf(buf, sizeof(buf)-1,
                    "Content-type: image/jpeg\r\n"
                    "Content-length: %d\r\n"
                    "\r\n", s);

            HTTPD_Send_Buffer(req, buf, strlen(buf));

            for ( i = 0; c[i].data != 0; i++) 
                HTTPD_Send_Buffer(req, c[i].data, c[i].length);
            

        }
        break;
        
    case CAMERA_METHOD_YUYV:
        {
            unsigned char *jpegBuffer;
            unsigned int jpegLeft = 1024*1024;

            jpegBuffer = malloc(jpegLeft);
            if (!jpegBuffer) 
                fatal_f("Failed to allocate JPEG encoding buffer.\n");

            int s = jpeg_compress(c, jpegBuffer, jpegLeft);
            
            snprintf(buf, sizeof(buf)-1,
                    "Content-type: image/jpeg\r\n"
                    "Content-length: %d\r\n"
                    "\r\n", s);

            HTTPD_Send_Buffer(req, buf, strlen(buf));
            HTTPD_Send_Buffer(req, jpegBuffer, s);
            free(jpegBuffer);
        }
        break;
    }
}


static void put_image_udp( struct chunk *c, void *arg)
{
    HTTPD_Request req = (HTTPD_Request)arg;
    
    udp_params* params = HTTPD_GetData(req);
    
    if (params == 0)
        fatal_f("UDP params is null.\n");
    
    int i, s = 0;

    char buf[256];

    switch(camera_method) 
    {
    case CAMERA_METHOD_MJPEG:
    case CAMERA_METHOD_JPEG:
        {
            for ( i = 0; c[i].data != 0; i++) 
                s += c[i].length;

            unsigned char *b;

            if (params->buffer_size < s)
                params->buffer = params->buffer == 0 ? malloc(s) : realloc(params->buffer, s);
            
            if (params->buffer == 0)
                fatal_f("Cannot allocate frame buffer.\n");
            
            for (i = 0, b = params->buffer; c[i].data != 0; i++) 
            {
                memcpy( b, c[i].data, c[i].length);
                b += c[i].length;
            }
            
            b = params->buffer;
            
            while (s > 0)
            {
                int ss = s > udp_frame_size ? udp_frame_size : s;
                
                if (sendto(params->socket, b, ss, 0 , &params->addr, sizeof(params->addr)) == -1)
                    log_f("Cannot send udp packet, %s", strerror(errno));
                    
                b += ss;
                s -= ss;
            }
        }
        break;
        
    case CAMERA_METHOD_YUYV:
        {
            unsigned char *jpegBuffer;
            int jpegLeft = 1024*1024;

            jpegBuffer = malloc(jpegLeft);
            if (!jpegBuffer) 
                fatal_f("Failed to allocate JPEG encoding buffer.\n");

            int s = jpeg_compress(c, jpegBuffer, jpegLeft);
            
            unsigned char* b = jpegBuffer;
            
            while (s > 0)
            {
                int ss = s > udp_frame_size ? udp_frame_size : s;
                
                if (sendto(params->socket, b, ss, 0 , &params->addr, sizeof(params->addr)) == -1)
                    log_f("Cannot send udp packet, %s", strerror(errno));
                    
                b += ss;
                s -= ss;
            }
            
            free(jpegBuffer);
        }
        break;
    }
}

static void put_single_image(const struct chunk *c, void *arg)
{
    HTTPD_Request req = (HTTPD_Request)arg;
    int i,s=0;

    HTTPD_Add_Header(req, "Cache-Control: no-cache");
    HTTPD_Add_Header(req, "Pragma: no-cache");
    HTTPD_Add_Header(req, "Expires: Thu, 01 Dec 1994 16:00:00 GMT");
    HTTPD_Add_Header(req, "Content-type: image/jpeg");

    switch(camera_method) 
    {
    case CAMERA_METHOD_MJPEG:
    case CAMERA_METHOD_JPEG:
        {
            unsigned char *buffer, *b;
            for (i = 0; c[i].data != 0; i++) 
                s += c[i].length;
            
            buffer = malloc(s);
            if (buffer == 0)
                fatal_f("Cannot allocate frame buffer.\n");
            
            for (i = 0, b = buffer; c[i].data != 0; i++) 
            {
                memcpy( b, c[i].data, c[i].length);
                b += c[i].length;
            }
            
            HTTPD_Send_Body( req, buffer, s);
            free(buffer);
        }
        break;
        
    case CAMERA_METHOD_YUYV:
        {
            unsigned char *jpegBuffer;
            unsigned int jpegLeft = 1024*1024;

            jpegBuffer = malloc(jpegLeft);
            if ( !jpegBuffer) 
                fatal_f("Failed to allocate JPEG encoding buffer.\n");

            int s = jpeg_compress(c, jpegBuffer, jpegLeft);
            
            HTTPD_Send_Body( req, jpegBuffer, s);
            free(jpegBuffer);
        }
        break;
    }

    log_f("image size = %d\n",s);
}

static void stream_image_udp( HTTPD_Request req)
{
    HTTPD_Add_Header( req, "Cache-Control: no-cache");
    HTTPD_Add_Header( req, "Pragma: no-cache");
    HTTPD_Add_Header( req, "Expires: Thu, 01 Dec 1994 16:00:00 GMT");
    HTTPD_Add_Header( req, "Content-Type: text/plain");
    
    HTTPD_Send_Buffer(req, crlf, strlen(crlf));

    char buf[256];
    unsigned int frames_sent = 0;
    
    int fps_div = device_fps * fps_multiplier / fps;
    int i = fps_div;
    
    while (1)
    {
        HTTPD_ResetWatchdog(req);
        
        if (i >= fps_div)
        {
            with_next_frame( &put_image_udp, req);
            ++frames_sent;
            i -= fps_div;
            
            if ((frames_sent % fps) == 0)
            {
                snprintf(buf, sizeof(buf)-1,"%u frames sent\r\n", frames_sent);
                if (!HTTPD_Send_Buffer( req, buf, strlen(buf)))
                    break;
            }
        }
        else
            with_next_frame( &put_image_stub, req);
        
        i += fps_multiplier;
    }
    
    udp_params* params = HTTPD_GetData(req);
    
    if (params == 0)
        fatal_f("UDP params is null.\n");
    
    if (params->buffer != 0)
        free(params->buffer);
}


static void stream_image( HTTPD_Request req)
{
    HTTPD_Add_Header( req, "Cache-Control: no-cache");
    HTTPD_Add_Header( req, "Pragma: no-cache");
    HTTPD_Add_Header( req, "Expires: Thu, 01 Dec 1994 16:00:00 GMT");
    HTTPD_Add_Header( req, "Content-Type: multipart/x-mixed-replace; boundary=" BOUNDARY);

    HTTPD_Send_Buffer(req, crlf, strlen(crlf));

    int fps_div = device_fps * fps_multiplier / fps;
    int i = fps_div;
    
    while (HTTPD_Send_Buffer( req, boundary, strlen(boundary)))
    {
        HTTPD_ResetWatchdog(req);
        
        if (i >= fps_div)
        {
            with_next_frame( &put_image, req);
            i -= fps_div;
        }
        else
        {   
            with_next_frame( &put_image_stub, req);
            i += fps_multiplier;
        }
    }
}

static void do_video_call( HTTPD_Request req, video_action action, int cid, int val)
{
    char buf[8192];

    with_device( action, buf, sizeof(buf), cid, val);
    HTTPD_Add_Header( req, "Cache-Control: no-cache");
    HTTPD_Add_Header( req, "Pragma: no-cache");
    HTTPD_Add_Header( req, "Expires: Thu, 01 Dec 1994 16:00:00 GMT");
    HTTPD_Add_Header( req, "Content-Type: text/xml");
    HTTPD_Send_Body( req, buf, strlen(buf));
}

static int demand_authorization(HTTPD_Request req)
{
    HTTPD_Send_Status( req, 401, "Authorization Required");
    HTTPD_Add_Header( req, "WWW-Authenticate: Basic realm=\"tinycamd\"");
    HTTPD_Send_Body(req, "login required", 14);
    return 0;
}

static int check_password( HTTPD_Request req, int setup) 
{

    if ( setup ) 
    {
        if ( !setup_password) 
            return 1;
    } 
    else 
    {
        if ( !setup_password && !password) 
            return 1;
    }

    {
        const char *auth = HTTPD_Get_Authorization(req);

        if ( !auth) return demand_authorization(req);

        // a setup_password will do anything, if we have it.
        if ( setup_password && strcmp( auth, setup_password) == 0) 
            return 1;
        if ( setup) 
            return demand_authorization(req);

        if ( password && strcmp( auth, password)==0) 
            return 1;
    }
    
    return demand_authorization(req);
}

static void handle_requests(HTTPD_Request req, const char *method, const char *rawUrl)
{
    int cid, val;
    const char *url = rawUrl;
    
    int ip[4] = {0, 0, 0, 0}, port;

    if ( strncmp( rawUrl, url_prefix, strlen(url_prefix))) 
    {
        url = "***BADURL-NOPREFIX***";
    } 
    else 
    {
        url = rawUrl + strlen(url_prefix);
    }

    log_f("Request: %s %s => %s\n", method, rawUrl, url);
    if ( strcmp(url,"/status")==0) 
    {
        do_status_request(req);
    } 
    else if ( strcmp(url,"/setup") == 0) 
    {
        if ( check_password(req, 1)) 
        {
            HTTPD_Add_Header( req, "Content-type: text/html");
            HTTPD_Send_Body(req, setup_html,setup_html_size);
        }
    } 
    else if ( strcmp(url,"/tinycamd.js") == 0) 
    {
        HTTPD_Add_Header( req, "Content-type: text/javascript; charset=utf8");
        HTTPD_Send_Body(req, tinycamd_js,tinycamd_js_size);
    } 
    else if ( strcmp(url,"/jquery.min.js") == 0) 
    {
        HTTPD_Add_Header( req, "Content-type: text/javascript; charset=utf8");
        HTTPD_Send_Body(req, jquery_min_js, jquery_min_js_size);
    } 
    else if ( strcmp(url,"/tinycamd.css") == 0) 
    {
        HTTPD_Add_Header( req, "Content-type: text/css");
        HTTPD_Send_Body(req, tinycamd_css,tinycamd_css_size);
    } 
    else if ( strcmp(url,"/stream")==0) 
    {
        if ( check_password(req, 0)) 
            stream_image(req);
    } 
    else if ( sscanf(url,"/udp?host=%d.%d.%d.%d&port=%d",&ip[0], &ip[1], &ip[2], &ip[3], &port) == 5 ||
              sscanf(url,"/udp?port=%d&host=%d.%d.%d.%d", &port, &ip[0], &ip[1], &ip[2], &ip[3]) == 5 ||
              sscanf(url,"/udp?port=%d", &port) == 1)
    {
        if ( check_password(req, 0))
        {
            udp_params* params = HTTPD_AllocData(req, sizeof(udp_params));
            
            if (params == 0) 
                fatal_f("Failed to allocate UDP params.\n");
            
            params->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (params->socket == -1)
                fatal_f("Failed to create UDP socket.\n");
            
            memset(&params->addr, 0, sizeof(params->addr));
            params->addr.sin_family = AF_INET;
            params->addr.sin_port = htons(port);

            if (ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0)
            {
                struct sockaddr_in remoteAddr = HTTPD_GetRemoteAddr(req);
                params->addr.sin_addr = remoteAddr.sin_addr;
            }
            else
            {
                char saddr[64];
                snprintf(saddr, sizeof(saddr)-1, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

                if (inet_aton(saddr, &params->addr.sin_addr) == 0) 
                    fatal_f("inet_aton() failed\n");
            }

            log_f("udp addr: %s:%d\n", inet_ntoa(params->addr.sin_addr), port);
            
            params->buffer = 0;
            params->buffer_size = 0;

            stream_image_udp(req);
            close(params->socket);
        }
            
    }
    else if ( strcmp(url,"/controls")==0) 
    {
        do_video_call( req, list_controls,0,0);
    } 
    else if ( sscanf(url,"/set?%d=%d",&cid,&val)==2 ) 
    {
        if ( check_password(req,1)) 
            do_video_call( req, set_control,cid,val);
    } 
    else if ( strcmp(url,"/") == 0 || strcmp( url, "/image") == 0 || strncmp( url, "/image?", 11) == 0) 
    {
        if ( check_password(req, 0)) 
            with_current_frame( &put_single_image, req);
    } 
    else 
    {
        HTTPD_Send_Status( req, 404, "Not Found");
        HTTPD_Send_Body( req, "404 - Not found", 15);
    }
}

static void *sleeper(void *arg)
{
    while(1) 
        sleep(1000);
    return 0;
}

int main(int argc, char **argv)
{
    pthread_t captureThread;
    pthread_t httpdThread;

    do_options(argc, argv);

    if ( daemon_mode) {
        if ( daemon(0,0) == -1) {
            fatal_f("Failed to become a daemon: %s\n", strerror(errno));
        }
    }

    if ( pid_file) {
        FILE *pf = fopen( pid_file,"w");
        if ( !pf) fatal_f("Failed to open pid file %s: %s\n", pid_file, strerror(errno));
        fprintf(pf, "%d\n", getpid());
        if ( fclose(pf)==EOF) fatal_f("Failed to close pid file %s: %s\n", pid_file, strerror(errno));
    }

    open_device();

    if ( probe_only) {
        probe_device();
        return 0;
    }

    init_device();
    start_capturing();

    pthread_create( &captureThread, NULL, main_loop, NULL);

    /*
    ** I am so sorry. But glibc dynamically loads libgcc_s.so.1 to handle pthread_cancel, so
    ** I need to get that in before chrooting.
    */
    {
        pthread_t crappyHack;
        void *whatever;

        if ( pthread_create( &crappyHack, 0, sleeper, 0)) fatal_f("Failed to start test thread.\n");
        if ( pthread_cancel( crappyHack)) fatal_f("Failed to cancel test thread.\n");
        if ( pthread_join( crappyHack, &whatever)) fatal_f("Failed to join test thread.\n");
    }

    /*
    ** Slink into our ghetto and lower our privileges in preparation for handling queries.
    */
    {
        uid_t uid = getuid();

        /* lookup uid before we chroot... or it is gone. */
        if ( setuid_to) {
            struct passwd *pw = getpwnam(setuid_to);
            if ( !pw) fatal_f("Failed to lookup user `%s' for setuid: %s\n", setuid_to, strerror(errno));
            uid = pw->pw_uid;
        }

        if ( chroot_to) {
            if ( chroot( chroot_to)) fatal_f("Failed to chroot to `%s': %s\n", chroot_to, strerror(errno));
        }

        if ( setuid_to) {
            if ( setreuid(uid, uid)) fatal_f("Failed to setuid to `%s': %s\n", setuid_to, strerror(errno));
        }
    }

    httpdThread = HTTPD_Start( bind_name, handle_requests);

    for(;;) sleep(100);

    close_device();

    return 0;
}
