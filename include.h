//#define DEBUG   //define debug print
#define NDEBUG   //shutdown assert

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define FREE(X) if((X)) {free((X)); (X)=NULL;}
#define PR(X, Y) printf(#X " = " Y "\n", X ) //i=1; PR(i, "%d")
#define PS(X) printf(#X " = %s\n", X ) //PR(str)
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif

#ifndef TCPTYPE
#define TCPTYPE
	#define FIRSTSHARK -1
	#define SECONDSHARK -2
	#define THIRDSHARK -3
	#define ACK -4
	#define GET 1
	#define POST 3
#endif

#ifndef HEADCAL
#define HEADCAL
	#define TCPHL(X) ((X)->doff * 4)
	#define IPHL(X) ((X)->ihl * 4)
	#define IPL(X) (ntohs((X)->tot_len))
#endif

#ifndef SENDTYPE
#define SENDTYPE
	#define SEND_DIRECT 0
	#define SEND_UP 1
#endif

#ifndef WEBTYPE
#define WEBTYPE
	#define FRIEND 1
	#define STATUS 2
	#define NOTE 3
	#define COMMENT 4
	#define PHOTO 5
	#define MEDIA_SET 6
	#define ADD_FRIEND 7
	#define EDIT_NOTE 8
    #define UPLOAD_PHOTO 9
    #define EVENT 10
    #define NEWSFEED 11
    #define WELCOME 12
    #define SHARE 13
    #define LIKE 14
    #define LIKE_PAGE 15
    #define SEARCH 16
    #define JOIN_GROUP 17
    #define CREATE_GROUP 18
    #define CHAT 19
    #define UPLOAD_VIDEO 20
    #define JOIN_EVENT 21
    #define CREATE_EVENT 22
#endif

#ifndef STRUCT
#define STRUCT

struct HTTP{
	unsigned char method;
	int head_length;
	char url[300];
	char cookie[300];
	char content[4096];
};

struct connection_info{
	char user_id[20];
	char s_id[20]; //visitor view subject's page
	int p_type; /* page type */
	char r_id[52]; /* resource id */
	char comment[100]; /* post content */
};

struct cnn_news_info{
	char time[10];        //publish time of a news 
	char category[20];    //category of a news
};
#endif
