/*
 * =====================================================================================
 *
 *       Filename:  http_parse.c
 *
 *    Description:  parse http message
 *    				* from https://github.com/joyent/http-parser
 *    				*
 *
 *        Version:  1.0
 *        Created:  01/25/2013 04:19:02 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Billowkiller (bk), billowkiller@gmail.com
 *   Organization:
 *
 * =====================================================================================
 */

/* Based on src/http/ngx_http_parse.c from NGINX copyright Igor Sysoev
 *
 * Additional changes are licensed under the same terms as NGINX and
 * copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
 * Permission is hereby granted, FREE of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "http_parse.h"
#include <time.h>

static time_t start, end;
static int t = 0;
int _header_field_type(const char *at)
{
	if(!strncmp(at, "Cookie", 6))
		return COOKIE;
	if(!strncmp(at, "Host", 4))
		return HOST;
	if(!strncmp(at, "Content-Length", 14))
		return CONTENT_LENGTH;
	if(!strncmp(at, "Referer", 7))
		return REFERER;
	return 0;
}

void _init_c_info()
{
	bzero(&c_info, sizeof(c_info));
	c_info.user_id[0] = '\0';
	c_info.s_id[0] = '\0';
	c_info.r_id[0] = '\0';	
	c_info.comment[0] = '\0'; 
	c_info.p_type = 0;
	content_length = 0;
	con_len = 0;
}

int _page_type_(char *path)
{
	if(strstr(path,"like"))
		return LIKE;
	if(strstr(path, "notes"))
		return NOTE;
	if(strstr(path, "photo")||strstr(path, "Photo"))
		return PHOTO;
	if(strstr(path, "media_set"))
		return MEDIA_SET;
	if(strstr(path, "friends"))
		return FRIEND;
	if(strstr(path, "add_friend")||strstr(path,"ajax/eh.php"))
		return ADD_FRIEND;
	if(strstr(path, "edit"))
		return EDIT_NOTE;
	if(strstr(path, "comment"))
		return COMMENT;
	if(strstr(path, "updatestatus"))
		return STATUS;
	if(strstr(path,"media/saveunpublished"))
		return UPLOAD_PHOTO;
	if(strstr(path,"events"))
        {
                if(strstr(path,"join"))
                return JOIN_EVENT;
                else
		return EVENT;
        }
	if(strstr(path,"share"))
		return SHARE;
	if(strstr(path,"pages/fan_status"))
		return LIKE_PAGE;
	if(strstr(path,"search"))
		return SEARCH;
	if(strstr(path,"groups/membership/r2j"))
		return JOIN_GROUP;
	if(strstr(path,"groups/create_post"))
		return CREATE_GROUP;
	if(strstr(path,"mercury/send_messages"))
		return CHAT;
	if(strstr(path,"video/upload"))
		return UPLOAD_VIDEO;
	if(strstr(path,"plan")&&strstr(path,"create"))
		return CREATE_EVENT; 
	return 0;
}

void _print_c_info()
{
#ifdef DEBUG
	printf("user_id=%s, s_id=%s, p_type=%d, r_id=%s, comment=%s\n", c_info.user_id, c_info.s_id, c_info.p_type, c_info.r_id, c_info.comment);	
#endif
}

void _url_parse(char * url)
{
    #ifdef DEBUG
	printf("............url = %s.........\n", url);
	#endif
	char *pos;
	int len=0;
	char *path;

	//parse url
	parseURL(url, &storage);
	storage.path.start += 1;  //remove '/'
	if(storage.path.start == storage.path.end)
		return;
	
	path = readURLField(url, storage.path);
	//avoid referer url check
	if(c_info.p_type == 0)
		c_info.p_type = _page_type_(path);

	//modify me, stupid implemention of s_id
	int path_len;
	path_len=strlen(path);
	int i=-1;
	int r_id_len=0;
	char* temp;
	if(c_info.p_type==EVENT)
	{
		if(path[path_len-1]>='/'&&path[path_len-2]>='0'&&path[path_len-2]<='9')
		{
			i=path_len-3;
			r_id_len=1;
		}
	}
	else if(c_info.p_type==NOTE)
	{
		if(path[path_len-1]>='0'&&path[path_len-1]<='9')
		{
			i=path_len-2;
			r_id_len=1;
		}
	}
	while(path[i]>='0'&&path[i]<='9'&&i>=0)
	{
		i--;
		r_id_len++;
	}
	if(r_id_len>10&&r_id_len<52)
	{	
		temp=path+(i+1);
		strncpy(c_info.r_id,temp,r_id_len);
		c_info.r_id[r_id_len]='\0';
	}




	if(!strncmp(path+(strlen(path)-6),"_n.jpg",6))
	{
		printf("matched jpg\n");
		int resource_id_length;
		char* resource_id;
		char* position;
		position = path + strlen(path) - 7;
		while(*position!='_'&& *position!='/'){
			position--;
		}
		char* ptemp=position;
		position--;
		while(*position!='_'&& *position!='/'){
			 position--;
		}
		if(*position == '_'){
			position++;
			resource_id = position;
			resource_id_length = ptemp - position;
			printf("length=%d\n",resource_id_length);
			memcpy(c_info.r_id,resource_id,resource_id_length);
			c_info.r_id[resource_id_length]='\0';
			printf("%s\n",c_info.r_id);
		}
		
		FREE(path);
	}
	else if((pos = strchr(path, '/')) == strrchr(path, '/'))
	{
		if(!pos)
		{
			strcpy(c_info.s_id, path);
			FREE(path);
		}
		else
		{
			memcpy(c_info.s_id,path, pos-path);
			c_info.s_id[pos-path] = '\0';
			FREE(path);
		}
		if(strlen(c_info.s_id) > 4 && !strcmp(c_info.s_id+ strlen(c_info.s_id) - 4, ".php"))
		{
			bzero(c_info.s_id, sizeof(c_info.s_id));
			c_info.s_id[0] = '\0';
		}
	}else
		FREE(path);
//	regex_match("(\\w+\\.)+\\w+", path, &pos, &len);   //regex [\w+\.]+, not endwith php
//	if(len != 0 && !strstr(path, "php"))  //need modify
//	{
//		printf("pos = %s, len=%d\n", pos, len);
//	    strcpy(c_info.s_id, path);
//		printf("subject name:%s\n", c_info.s_id);
//	}

	if(storage.query.end != storage.query.start)
	{
		char * query = readURLField(url, storage.query);
		if(c_info.p_type == 0)
		{
			if(strstr(query, "sk=nf"))
				c_info.p_type = NEWSFEED;
			if(strstr(query, "sk=welcome"))
				c_info.p_type = WELCOME;
		}
		qs_scanvalue("__user", query, c_info.user_id, sizeof(c_info.user_id));
		qs_scanvalue("q", query, c_info.comment, sizeof(c_info.comment));
        char media_temp[100];
               char* photo_data;
		switch(c_info.p_type)
		{
                        case SHARE:
                                if(c_info.s_id[0]=='\0')
				qs_scanvalue("profile_id", query, c_info.s_id, sizeof(c_info.s_id));
                                if(c_info.r_id[0]=='\0')
				qs_scanvalue("p[0]", query, c_info.r_id, sizeof(c_info.r_id));
                                if(c_info.r_id[0]!='\0'&&!strcmp(c_info.r_id,c_info.s_id))
                                {
                                memset(c_info.r_id,0,sizeof(c_info.r_id));
				qs_scanvalue("p[1]", query, c_info.r_id, sizeof(c_info.r_id));
                                }
                                break; 
			case MEDIA_SET:
				qs_scanvalue("set", query, media_temp, sizeof(media_temp));
			    char* media_start=strchr(media_temp,'.');
				media_start++;
				char* media_end=strchr(media_start,'.');
				int media_len=media_end-media_start;
			    memcpy(c_info.r_id,media_start,media_len);
				c_info.r_id[media_len]='\0';
				break;
			case PHOTO:
				photo_data=(char*)malloc(300);
				photo_data[0]='\0';
                                printf("OK\n");
				qs_scanvalue("data", query, photo_data, 300);
				char* photo_rid=strstr(photo_data,"fbid");
				char* photo_set=strstr(photo_data,"set");

				if(photo_data[0]!='\0'&&photo_rid&&photo_set)
				{
			        printf("data:%s\n",photo_data);	
				printf("OK\n");
                             
				photo_rid+=5;
				if(*photo_rid==':')
                                {
				printf("OK\n");
                                    photo_rid+=2;
				printf("OK First\n");
                                    char* photo_rid_end= strchr(photo_rid,'"');
				    int photo_rid_len=photo_rid_end-photo_rid;
				    printf("rid_len is :%d\n",photo_rid_len);
                                    if(photo_rid_len>0)
                                    {                         
                                    memcpy(c_info.r_id,photo_rid,photo_rid_len);
				    c_info.r_id[photo_rid_len]='\0';
                                    }
                                }
                                char* photo_sid=strchr(photo_set,'.');
				photo_sid++;
				if(*photo_sid>='0'&&*photo_sid<='9')
                                {
                               
                                    char* photo_sid_end= strchr(photo_sid,'.');
				    int photo_sid_len=photo_sid_end-photo_sid;
                                    if(photo_sid_len>0)
                                    {                         
                                    memcpy(c_info.s_id,photo_sid,photo_sid_len);
				    c_info.r_id[photo_sid_len]='\0';
                                    }
                               }
                               }
                                free(photo_data);
                               if(c_info.r_id[0]=='\0')
				qs_scanvalue("fbid", query, c_info.r_id, sizeof(c_info.r_id));
                               
				break;
			case UPLOAD_PHOTO:
			    qs_scanvalue("target_id",query,c_info.s_id,sizeof(c_info.s_id));
		       break;
			case SEARCH:
			    qs_scanvalue("value",query,c_info.comment,sizeof(c_info.comment));
		}
		FREE(query);
	}
}

int on_url(http_parser* _, const char* at, size_t length) {
	(void)_;
	
	if((int)length > 500 || (int)length <2) 
        {
                printf("too long:%d\n",(int)length);
		return -1;
        }
	memcpy(http.url, at, (int)length);
	http.url[(int)length] = '\0';

	_url_parse(http.url);
	return 0;
}

int on_header_field(http_parser* _, const char* at, size_t length) {
	(void)_;
	http_field_type =  _header_field_type(at);
	return 0;
}

int on_header_value(http_parser* _, const char* at, size_t length) {
	(void)_;
	//char *url;
	switch(http_field_type)
	{
		case COOKIE: //unknow size of cookie, stay available
 			if(c_info.user_id[0]=='\0')
 			{
 				char *start = strstr(at, "c_user");
				if(start)
				{
					int len = strchr(start+7, ';')-start-7;
					memcpy(c_info.user_id, start+7, len);
					c_info.user_id[len] = '\0';
				}
					
			}
			//memcpy(http.cookie, at, (int)length);
			//http.cookie[(int)length] = '\0';
			break;
		case CONTENT_LENGTH:
			content_length = atoi(at);
			break;
		case REFERER:
			//fetch s_id
		   /*
			if(c_info.user_id[0] == '\0')
			{
				char *url = (char *)malloc((int)length+1);
				memcpy(url, at, (int)length);
				url[(int)length] = '\0';
				_url_parse(url);
				FREE(url);
			}
			*/
			break;
	}
					if((c_info.p_type==WELCOME||c_info.p_type==NEWSFEED)&&c_info.s_id[0] =='\0')
					{
						int len =strlen(c_info.user_id);
						memcpy(c_info.s_id,c_info.user_id , len);
						c_info.s_id[len]= '\0';
					}
//	printf( "%.*s\n", (int)length, at);
//
//	CURRENT_LINE->value = (char *)malloc(length+1);
//	strncpy(CURRENT_LINE->value, at, length);
//	CURRENT_LINE->value[length] = '\0';
	
//	if(!strcmp(CURRENT_LINE->field, "Content-Type"))
//	{
//		charset_parse(CURRENT_LINE->value, contype, charset);
//	}
//	if(!strcmp(CURRENT_LINE->field, "Content-Encoding"))
//	{
//		if(!strcmp(CURRENT_LINE->value, "gzip"))
//			isGzip = 1;
//	}
	return 0;
}

int on_body(http_parser* _, const char* at, size_t length) {
	(void)_;
printf("on_body is called\n");
	con_len = (int)length;
	char *title;
	char *chat_sid;
    
	//analysis
	if(c_info.user_id[0] == '\0')
	{
		qs_scanvalue("__user", at, c_info.user_id, sizeof(c_info.user_id));
		printf("user_id = %s\n", c_info.user_id);
	}
	switch(c_info.p_type)
	{
                case CREATE_EVENT:
			qs_scanvalue("title", at, c_info.comment, sizeof(c_info.comment));
                        int title_len=strlen(c_info.comment);
                        int detail_len=sizeof(c_info.comment)-title_len;
                        char* detail=(char*)malloc(detail_len);
			qs_scanvalue("details", at, detail, detail_len-1);
                        strcat(c_info.comment,detail);
                        free(detail);
                        break;
                case JOIN_EVENT:
			qs_scanvalue("eid", at, c_info.r_id, sizeof(c_info.r_id));
                        break;
		case COMMENT:
			qs_scanvalue("comment_text", at, c_info.comment, sizeof(c_info.comment));
			break;
		case ADD_FRIEND:
			qs_scanvalue("to_friend", at, c_info.s_id, sizeof(c_info.s_id));
			break;
		case NOTE:
			title = (char *)malloc(30);
			qs_scanvalue("title", at, title, 30);
			qs_scanvalue("note_content", at, c_info.comment, sizeof(c_info.comment));
			strcat(c_info.comment, title);
 			free(title);
			break;
		case STATUS:
			qs_scanvalue("xhpc_message_text", at, c_info.comment, sizeof(c_info.comment));
			break;
		case SHARE:
                        if(c_info.s_id[0]=='\0')
			qs_scanvalue("friendTarget", at, c_info.s_id, sizeof(c_info.s_id));
			qs_scanvalue("message_text", at, c_info.comment, sizeof(c_info.comment));
			break;
		case LIKE:
			qs_scanvalue("fbid", at, c_info.r_id, sizeof(c_info.r_id));	
			if(c_info.r_id[0]=='\0')
			qs_scanvalue("ft_ent_identifier", at, c_info.r_id, sizeof(c_info.r_id));
			break;
        case LIKE_PAGE:
			qs_scanvalue("fbpage_id", at, c_info.s_id, sizeof(c_info.s_id));
			break;
        case JOIN_GROUP:
			printf("join a group\n");
			qs_scanvalue("group_id", at, c_info.s_id, sizeof(c_info.s_id));
			break;
        case CREATE_GROUP:
			qs_scanvalue("name", at, c_info.comment, sizeof(c_info.comment));
			break;
        case CHAT:
			chat_sid=(char*)malloc(25);
			qs_scanvalue("message_batch[0][specific_to_list][0]", at, chat_sid, 25);
			memcpy(c_info.s_id, chat_sid+5,strlen(chat_sid)-5);
            c_info.s_id[strlen(chat_sid)-5]='\0';
			qs_scanvalue("message_batch[0][body]", at, c_info.comment, sizeof(c_info.comment));
                        free(chat_sid);
			break;
	}
	
//	if(!strncmp(contype, "text/html", CONTYPE_LENGTH))
//	{
//		printf("hello, world\n");
//		//fwrite(at, 1, length, dest);
//		if(length)
//		{
//			if(isGzip)
//			{
//				char * uncomp = (char*)calloc(length*8, sizeof(char));
//				decompress(at, uncomp, length);
//
//			#ifdef HTTP_PRINT
//				printf( "Body: %.*s\n", 8*length, uncomp);
//			#endif
//
//			}else{
//				
//			#ifdef HTTP_PRINT
//				printf( "Body: %.*s\n", (int)length, at);
//			#endif
//				;
//			}
//		}
//	}
	return 0;
}

//void charset_parse(char *contype_value, char *contype, char *str)
//{
//	char * chr= strchr(contype_value, '=');
//	if(NULL != chr)
//	{
//		strcpy(str, chr+1);
//		strcpy(contype, contype_value);
//		chr = strchr(contype, ';');
//		*chr = '\0';
//	}
//	else
//	{
//		strcpy(contype, contype_value);
//		str = "utf8";
//	}
//}

char* fileRead(char *filename, long* file_length)
{
	FILE* file = fopen(filename, "r");
	if (file == NULL) {
	  perror("fopen");
	}

	fseek(file, 0, SEEK_END);
	*file_length = ftell(file);
	if (*file_length == -1) {
	  perror("ftell");
	}
	fseek(file, 0, SEEK_SET);
	char* data = malloc(*file_length);
	if (fread(data, 1, *file_length, file) != (size_t)*file_length) {
	  fprintf(stderr, "couldn't read entire file\n");
	  FREE(data);
	}
	return data;
}


int processhttp(char* data, int http_length)
{
	if(!start)
		start = time(NULL);
	#ifdef DEBUG
	printf("t=%d\n",t++);
	#endif
	_init_c_info();
	http_parser_settings settings;
	size_t nparsed;
	memset(&settings, 0, sizeof(settings));
	settings.on_url = on_url;
	settings.on_header_field = on_header_field;
	settings.on_header_value = on_header_value;
	settings.on_body = on_body;

	http_parser parser;
	http_parser_init(&parser, HTTP_REQUEST);
	nparsed = http_parser_execute(&parser, &settings, data, (size_t)http_length);
	http.method = parser.method;

		end = time(NULL);
	#ifdef DEBUG
		printf("%fms\n", difftime(end, start)/t);
	#endif

	//test
	_print_c_info();

	if (nparsed != (size_t)http_length) 
	{
		#ifdef DEBUG
	    printf( "Error: %s (%s)\n",
	            http_errno_description(HTTP_PARSER_ERRNO(&parser)),
	            http_errno_name(HTTP_PARSER_ERRNO(&parser)));
		#endif
	}
	if(content_length !=  con_len && http.method == 3 && http_length < 4096)
	{
		memcpy(http.content, data, http_length);
		return FALSE;
	}

	return TRUE;
}

// int main()
// {
	
// 	long httplength = 0L;
// 	char *data;
// 	data = fileRead("b", &httplength);
// 	processhttp(data, httplength);
// }
