#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <locale.h>
#include <math.h>
#include <limits.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#define strsize(args...) snprintf(NULL, 0, args) + sizeof('\0')
#define PORT 8000
#define MAX_BUFFER_SIZE 4096
#define MAX_PATH_LEN 240
#define MAX_IPs 8
#define MAX_REQUEST 8
#define MAX_COUNT_HISTORY 20
#define MAX_USERS 100
#define MAX_HTML_COUNTER 10
#define MAX_LENGTH 50
#define MAX_LENGTH_PWD 200
#define MAX_ROOMS 8
#define MAX_USERS_IN_ROOM 8

typedef struct{
    //Start string
	char Method[MAX_LENGTH];
	char URL[MAX_LENGTH];
	char Protocol[MAX_LENGTH];
	//Heading
	char Host[MAX_LENGTH];
	char Date [MAX_LENGTH];
	char Last_Modified[MAX_LENGTH];
	char Content_Type[MAX_LENGTH];
	char Content_Language [MAX_LENGTH];
	char Content_Length [MAX_LENGTH];
	char Connection[MAX_LENGTH];
	char Cach_Control[MAX_LENGTH];
	char Upgrade_Insecure_Requests[MAX_LENGTH];
	char Origin[MAX_LENGTH];
	char User_Agent[200];
	char Accept[200];
	char Referer[MAX_LENGTH];
	char Accept_Encoding[MAX_LENGTH];
	char Accept_Language[MAX_LENGTH];
}HTTP_Context;

typedef struct{
    char typ;
	char user2[MAX_LENGTH];
    char room[MAX_LENGTH];
	size_t money;
}LastOperation;

typedef struct{
	char login[MAX_LENGTH];
	char password[MAX_LENGTH];
	char name[MAX_LENGTH];
	char IP[16];
	LastOperation lastoperation;
}User;

typedef struct{
	char name[MAX_LENGTH];
	char login[MAX_LENGTH];
	size_t balance;
}Name;

typedef struct{
	char IP[16];
	char login[MAX_LENGTH];
}IP;

typedef struct{
	char name_url_html[100];
}HTML_ADDR;

typedef struct{
	size_t users_unread;
	size_t money;
	char recipient[MAX_LENGTH];
	char uninterviewed[MAX_LENGTH*MAX_USERS_IN_ROOM-1];
}Request;

typedef struct{
	size_t counter_users;
	size_t counter_request;
	char name_room[MAX_LENGTH];
	Name user[MAX_USERS_IN_ROOM];
	char users_names[MAX_USERS_IN_ROOM*MAX_LENGTH];
	Request request[MAX_REQUEST];
}Room;

typedef struct{
	char function[100];
	int result;
	bool a;
}Error;

HTML_ADDR	htmls[MAX_HTML_COUNTER];
User		users[MAX_USERS];
Room		rooms[MAX_ROOMS];
IP			IPs[MAX_USERS];
Name		names[MAX_USERS];
char        now_pwd_dirrectory[MAX_LENGTH_PWD];


size_t num_users = 0;
size_t num_rooms = 0;
size_t num_IPs = 0;
struct stat st = {0};

int print_rooms(void){
    puts("\nRooms:");
    Room room;
    for(size_t i=0;i<num_rooms;i++){
        room = rooms[i];
        printf("(%lu)Name %s:\n\tUsers_names: %s\n\tCounter_users: %lu\n",
                i,room.name_room, room.users_names, room.counter_users);
        for(size_t j=0;j<room.counter_users;j++){
            printf("\t\t(%lu)User %s: \n\t\t\tName =%s\n\t\t\tBalanse = %lu\n",
                    j, room.user[j].login, room.user[j].name,
                    room.user[j].balance);
        }
    }
    return 0;
}

char * str_f_money(size_t money){
    char s_number[MAX_LENGTH];
    snprintf(s_number, sizeof(s_number), "%lu", money);
    size_t length = strlen(s_number);
    if (length == 0) {
        return NULL;
    }
    char *result = malloc(length + length / 3 + 1);
    if (!result) {
        return NULL;
    }
    char *ptr = result;
    for (size_t i = 0; i < length; ++i) {
        *ptr++ = s_number[i];
        if ((length - i - 1) % 3 == 0 && i != length - 1) {
            *ptr++ = ' ';
        }
    }
    *ptr = '\0';
    return result;

}

int print_users(void){
    puts("\nUsers:");
    User u;
    for(size_t i=0;i<num_users;i++){
        u = users[i];
        printf("User %lu:\n\tLogin: %s\n\tName: %s\n\tPassword: %s\n\tIP: %s\n",
                i, u.login, u.name, u.password, u.IP);
    }
    puts("\nName:");
    Name n;
    for(size_t i=0;i<num_users;i++){
        n = names[i];
        printf("User %lu:\n\tName: %s\n\tLogin: %s\n",
                i, n.name, n.login);
    }
    puts("\nIPs:");
    IP ip;
    for(size_t i=0;i<num_IPs;i++){
        ip = IPs[i];
        printf("User %lu:\n\tIP: %s\n\tLogin: %s\n",
                i, ip.IP, ip.login);
    }
    return 0;
}

int insert_string(char * file_string, char * search_string, char * inserting_string, char * result, size_t result_size){
	size_t len_search = strlen(search_string);
	size_t len_insert = strlen(inserting_string);
	size_t len_file   = strlen(file_string);
    if(len_file+len_insert<len_search){
        return -1;
    }
    size_t len_result = len_file + len_insert + 1 - len_search;
	if(result_size<len_result){
		return -1;
	}
	if(len_search==0){
		strcpy(result, file_string);
		return 0;
	}
    char cpy_result[len_result];
	char * c_file = strstr(file_string, search_string);
	if(c_file != NULL){
		if(len_insert!=0){
			if(c_file-file_string!=0){
				if(*(c_file+len_search)!='\0'){
					if(strncpy(cpy_result, file_string, c_file-file_string)==NULL){
						printf("Error strcpy");
						return -3;
					}
					cpy_result[c_file-file_string]='\0';
					if(strcat(cpy_result, inserting_string)==NULL){
						printf("Error strcat");
						return -4;
					}
					if(strcat(cpy_result, c_file+len_search)==NULL){
						printf("Error strcat");
						return -5;
					}
                    strcpy(result, cpy_result);
					return 0;
				}else{
					if(strncpy(cpy_result, file_string, c_file-file_string)==NULL){
						printf("Error strncpy");
						return -3;
					}
					cpy_result[c_file-file_string]='\0';
					if(strcat(cpy_result, inserting_string)==NULL){
						printf("Error strcat");
						return -4;
					}
                    strcpy(result, cpy_result);
					return 0;
				}
			}else{
				if(strcpy(cpy_result, inserting_string)==NULL){
					printf("Error strcpy");
					return -4;
				}
				if(strcat(cpy_result, c_file+len_search)==NULL){
					printf("Error strcat");
					return -5;
				}
                strcpy(result, cpy_result);
				return 0;
			}
		}else{
		    if(c_file-file_string!=0){
				if(*(c_file+strlen(search_string))!='\0'){
					if(strncpy(cpy_result, file_string, c_file-file_string)==NULL){
						printf("Error strcpy");
						return -3;
					}
					cpy_result[c_file-file_string]='\0';
					if(strcat(cpy_result, c_file+len_search)==NULL){
						printf("Error strcat");
						return -5;
					}
                    strcpy(result, cpy_result);
					return 0;
				}else{
					if(strncpy(cpy_result, file_string, c_file-file_string)==NULL){
						printf("Error strncpy");
						return -3;
					}
					cpy_result[c_file-file_string]='\0';
                    strcpy(result, cpy_result);
					return 0;
				}
			}else{
				if(strcat(cpy_result, c_file+len_search)==NULL){
					printf("Error strcat");
					return -5;
				}
                strcpy(result, cpy_result);
				return 0;
			}
		}
	}else{
		strcpy(result, file_string);
		return -6;
	}
}

size_t correct_size(char * path_file){
	FILE *html_file;
	size_t file_size;
	html_file = fopen(path_file, "rb");
	if (html_file == NULL) {
		perror("fopen");
		return 0;
	}
	// Ïðîâåðÿåì, ÷òî ôàéë ñóùåñòâóåò è äîñòóïåí äëÿ ÷òåíèÿ
	if (access(path_file, R_OK) != 0) {
		fclose(html_file);
		return 0;
	}
	fseek(html_file, 0, SEEK_END);
	file_size = ftell(html_file);
	rewind(html_file);
	return  file_size+1;
}

int send_html(char *path_file, char * result){
	FILE *html_file;
	size_t file_size;
	html_file = fopen(path_file, "rb");
	if (html_file == NULL) {
		perror("fopen");
		return -1;
	}
	// Ïðîâåðÿåì, ÷òî ôàéë ñóùåñòâóåò è äîñòóïåí äëÿ ÷òåíèÿ
	if (access(path_file, R_OK) != 0) {
		fclose(html_file);
		return -2;
	}

	fseek(html_file, 0, SEEK_END);
	file_size = ftell(html_file);
	rewind(html_file);
	if (fread(result, sizeof(char), file_size, html_file) != file_size) {
		fclose(html_file);
		return -3;
	}
	// Äîáàâëÿåì íóëü-òåðìèíàòîð â êîíåö ñòðîêè
	result[file_size] = '\0';

	fclose(html_file);
	return 0;
}

int s_user_login_users(char * login_user, size_t * id_user){
    if(num_users==0){
        *id_user = 0;
        return 1;
    }
    if(strcmp(users[0].login, login_user)>0){
        *id_user = 0;
        return 1;
    }
	size_t low=0;
	size_t high=num_users-1;
	size_t mid=low;
	int cmp=0;
	while (low <= high){
		mid = low + (high - low) / 2;
		cmp = strcmp(users[mid].login, login_user);
		if (cmp == 0){
            *id_user = mid;
			return 0;
		}else if (cmp < 0){
			low = mid + 1;
		}else{
			high = mid - 1;
		}
	}
    *id_user = low;
	return 1;
}

int s_user_IP_users(char * user_IP, size_t *id_user){
    /*s_user_login_users
     */
    if(num_IPs==0){
        *id_user = 0;
        return 1;
    }
    int passed =0;
	size_t low=0;
	size_t mid=low;
	size_t high=num_IPs - 1;
	int cmp=0;
    if(strcmp(IPs[0].IP, user_IP)>0){
        *id_user = 0;
        return 1;    
    }
    while(low<=high){
        mid = low + (high - low) / 2;
        cmp = strcmp(IPs[mid].IP, user_IP);
        if(cmp == 0){
            low = high+1;
        }else if(cmp<0){
            low = mid + 1;
        }else{
            high = mid - 1;
        }
    }
    if(cmp!=0){
        *id_user = low;
        return 1; //index IP in IPs
    }
    passed = s_user_login_users(IPs[mid].login, &low);
    if(passed!=0){
        *id_user = mid;
        return -1;
    }
    if(strcmp(IPs[mid].IP, users[low].IP)==0){
        *id_user = low;
        return 0;
    }
    *id_user = mid;
    return 2;
}

int s_user_name_users(char * name_user, size_t *id_user){
    int passed=0;
    if(num_users==0){
        *id_user = 0;
        return 1;
    }
	size_t low = 0;
	size_t mid = low;
	size_t high = num_users - 1;
	int cmp=0;
    if(strcmp(names[0].name, name_user)>0){
        *id_user = 0;
        return 1;
    }
	while(low<=high){
		mid = low + (high - low) / 2;
		cmp = strcmp(names[mid].name, name_user);
		if(cmp==0){
			low = high + 1;
		}else if(cmp<0){
			low = mid + 1;
		}else{
            high = mid - 1;
		}
	}
	if(cmp!=0){
		*id_user = low;     //index Name in names
        return 1;
    }
	passed = s_user_login_users(names[mid].login, &high);
    if(passed!=0){
        *id_user = mid;        //index User in users
        return -1;
    }
    if(strcmp(names[mid].name, users[high].name)==0){
        *id_user = high;
        return 0;
    }else{
        *id_user = mid;
        return 2;
    }
}

int s_room_name_rooms(char *name_room, size_t *id_room){
    if(num_rooms==0){
        *id_room = 0;
        return 1;
    }
	size_t low=0;
	size_t high=num_rooms-1;
	size_t mid=low;
	int cmp=0;
    if(strcmp(rooms[0].name_room, name_room)>0){
        *id_room = 0;
        return 1;
    }
	while (low <= high){
		mid = low + (high - low) / 2;
		cmp = strcmp(rooms[mid].name_room, name_room);
		if (cmp == 0){
            *id_room = mid;
			return 0;
		}else if (cmp < 0){
			low = mid + 1;
		}else{
			high = mid - 1;
		}
	}
    *id_room = low;
    return 1;
}

int s_user_name_room(char *name_user, char *name_room, size_t *id_user){
    if(num_rooms==0){
        return -1;
    }
    int passed = 0;
    size_t id_room=0;
    passed = s_room_name_rooms(name_room,&id_room);
    if(passed!=0){
        return -2;
    }
    if(rooms[id_room].counter_users==0){
        *id_user = 0;
        return 1;
    }

    if(strcmp(rooms[id_room].user[0].name, name_user)>0){
        *id_user = 0;
        return 1;
    }
    size_t low=0;
    size_t high=rooms[id_room].counter_users-1;
    size_t mid=0;
    int cmp=0;
    while(low<=high){
        mid = low +(high-low)/2;
        cmp = strcmp(rooms[id_room].user[mid].name, name_user);
        if(cmp == 0){
            *id_user=mid;
            return 0;
        }else if(cmp<0){
            low = mid+1;
        }else{
            high = mid-1;
        }
    }
    *id_user = low;
    return 1; //index user in room
}    

int s_request_uRequest_room(char *name_request, char *name_room, size_t *id_request){
   int passed = 0;
   size_t id_user=0;
   size_t id_room=0;
   passed = s_user_name_room(name_request, name_room, &id_user);
   if(passed!=0){
       return -1;
   }
   passed = s_room_name_rooms(name_room, &id_room);
   if(passed!=0){
       return -2;
   }
   if(rooms[id_room].counter_request==0){
       *id_request = 0;
       return 1;
   }
   if(strcmp(rooms[id_room].request[0].recipient, name_request)>0){
       *id_request = 0;
       return 1;
   }
   size_t low=0;
   size_t high=rooms[id_room].counter_request-1;
   size_t mid=low;
   int cmp=0;
   while(low<=high){
       mid=low+(high-low)/2;
       cmp = strcmp(rooms[id_room].request[mid].recipient, name_request);
       if(cmp == 0){
           *id_request=mid;
           return 0;
      }else if(cmp<0){
          low = mid+1;
      }else{
          high = mid -1;
      }
   }
   *id_request = low;
   return 1;
}

int create_room(char * buffer_name){
    int passed = 0;
    size_t id_room;
    passed = s_room_name_rooms(buffer_name, &id_room);
    if(passed==0){
        return -1;
    }
	if ((num_rooms+1)<=MAX_ROOMS){
		if(strlen(buffer_name)<MAX_LENGTH){
            char link_history[4096];
            strcpy(link_history, now_pwd_dirrectory);
            strcat(link_history,"/rooms/");
            strcat(link_history,buffer_name);
            FILE *file = fopen(link_history, "w");
            if(file == NULL){
                perror("Error opening file");
                return -4;
            }
            fputs("\0",file);
            fclose(file);
            for(size_t i = num_rooms;i>id_room;i--){
                rooms[i] = rooms[i-1];
            }
            strcpy(rooms[id_room].name_room, buffer_name);
            rooms[id_room].users_names[0]='\0';
            rooms[id_room].counter_users=0;
            rooms[id_room].counter_request=0;
            for(size_t i = 0;i<MAX_REQUEST;i++){
                rooms[id_room].request[i].money = 0;
                rooms[id_room].request[i].recipient[0]='\0';
                rooms[id_room].request[i].uninterviewed[0]='\0';
                rooms[id_room].request[i].users_unread = 0;
            }
            num_rooms+=1;
            return 0;
		}else{
			return -3;
		}
	}else{
		return -2;
	}
}

int insert_base(char *login, char *name, char *password, char *IP){
    int passed = 0;
	size_t k[3];
    if(num_users + 1 > MAX_USERS){
        return -1;
    }
	if(num_users == 0){
		strcpy(users[0].login, login);
		strcpy(users[0].name, name);
		strcpy(users[0].password, password);
		strcpy(users[0].IP, IP);


		strcpy(names[0].name, name);
		strcpy(names[0].login, login);

		strcpy(IPs[0].login, login);
		strcpy(IPs[0].IP, IP);
        num_IPs +=1;
		num_users += 1;
	}else{
        passed = s_user_login_users(login, &k[0]);
        passed = s_user_name_users(name, &k[1]);
        passed = s_user_IP_users(IP, &k[2]);
        if(passed > 0){
            for(size_t i= num_IPs; i>k[2] ;i--){ 
                IPs[i] = IPs[i-1];
            }
            strcpy(IPs[k[2]].login, login);
            strcpy(IPs[k[2]].IP, IP);
            num_IPs +=1;
        }else{
            strcpy(IPs[k[2]].login, login);
        }
            
        for(size_t i= num_users; i>k[0]; i--){
            users[i] = users[i-1];
        }
        for(size_t i= num_users; i>k[1]; i--){
            names[i] = names[i-1];
        }
        strcpy(users[k[0]].login, login);
        strcpy(users[k[0]].name, name);
        strcpy(users[k[0]].password, password);
        strcpy(users[k[0]].IP, IP);


        strcpy(names[k[1]].name, name);
        strcpy(names[k[1]].login, login);
        num_users += 1;

	}
	return 0;
}

int insert_head(char * buffer_all, char * search_field, char * buffer_out){
	char buffer[strlen(buffer_all)];
	char * id = strstr(buffer_all, search_field);
	if (id!=NULL){
		id += strlen(search_field);
		int i = 0;
		while((*id!='\n')&&(*id!= '\0')){
			buffer[i]=*id;
			i+=1;
			id+=1;
		}
		buffer[i]='\0';
		if(strcpy(buffer_out,buffer)!=NULL){
			return 0;
		}else{
			return -2;
		}
	}else{
		return -1;
	}
}

int insert_field(char * buffer_all, char * search_field, char * buffer_out){
	char buffer[strlen(buffer_all)];
	char * id = strstr(buffer_all, search_field);
	if (id!=NULL){
		id += strlen(search_field);
		int i = 0;
		while((*id!='&')&&(*id!= '\0')){
			buffer[i]=*id;
			i+=1;
			id+=1;
		}
		buffer[i]='\0';
		if(strcpy(buffer_out,buffer)!=NULL){
			return 0;
		}else{
			return -2;
		}
	}else{
		return -1;
	}
}

void html_way_create(void){
	char path[256];
    strcpy(path, now_pwd_dirrectory);
    strcat(path, "/htmls/");
    for(unsigned short int i = 0;i < MAX_HTML_COUNTER; i++){
		strcat(htmls[i].name_url_html, path);
	}
	strcat(htmls[0].name_url_html, "login.html\0");
	strcat(htmls[1].name_url_html, "registration.html\0");
	strcat(htmls[2].name_url_html, "rooms.html\0");
	strcat(htmls[3].name_url_html, "create_room.html\0");
	strcat(htmls[4].name_url_html, "game.html\0");
	strcat(htmls[5].name_url_html, "confirmation.html\0");
	strcat(htmls[6].name_url_html, "confirmation_bank.html\0");
	strcat(htmls[7].name_url_html, "history.html\0");
	strcat(htmls[8].name_url_html, "profile.html\0");
}

int insert_account(char * login, char * name, char * password, char * IP){
	//insert_base
    int passed=0;
	size_t index =0;
    passed = s_user_name_users(name, &index);
	if(passed!=0){
        passed =s_user_login_users(login, &index);
        if(passed!=0){
			insert_base(login, name, password, IP);
            users[index].lastoperation.money = 0;
            strcpy(users[index].lastoperation.room, "FUN");
            strcpy(users[index].lastoperation.user2, "Jocker");
            return 0;
		}else{
			return -2;
		}
	}else{
		return -1;
	}
}

int create_http_struct(char *buffer, HTTP_Context * indicator_http){
	//insert_head
	size_t i_for_buffer = 0;
	size_t k;
	while(buffer[i_for_buffer] != ' '){
		indicator_http->Method[i_for_buffer]=buffer[i_for_buffer];
		i_for_buffer++;
	}
	indicator_http->Method[i_for_buffer]='\0';
	i_for_buffer++;
	for(size_t i = 0; buffer[i_for_buffer]!= ' '; i++){
		indicator_http->URL[i]=buffer[i_for_buffer];
		i_for_buffer++;
		k=i;
	}
	indicator_http->URL[k+1] = '\0';

	i_for_buffer ++;
	for(size_t i = 0; (buffer[i_for_buffer]!= '\n')&&(buffer[i_for_buffer]!='\r');i++){
		indicator_http->Protocol[i]=buffer[i_for_buffer];
		i_for_buffer++;
		k=i;
	}
	indicator_http->Protocol[k+1] = '\0';

	if(insert_head(buffer,"Host: ",indicator_http->Host)<0){
		indicator_http->Host[0]='\0';
	}
	if(insert_head(buffer,"Date: ",indicator_http->Date)<0){
		indicator_http->Date[0]='\0';
	}
	if(insert_head(buffer,"Last-Modified: ",indicator_http->Last_Modified)<0){
		indicator_http->Last_Modified[0]='\0';
	}
	if(insert_head(buffer,"Content-Type: ",indicator_http->Content_Type)<0){
		indicator_http->Content_Type[0]='\0';
	}
	if(insert_head(buffer,"Content-Language: ",indicator_http->Content_Language)<0){
		indicator_http->Content_Language[0]='\0';
	}
	if(insert_head(buffer,"Content-Length: ",indicator_http->Content_Length)<0){
		indicator_http->Content_Length[0]='\0';
	}
	if(insert_head(buffer,"Connection: ",indicator_http->Connection)<0){
		indicator_http->Connection[0]='\0';
	}
	if(insert_head(buffer,"Cache-Control: ",indicator_http->Cach_Control)<0){
		indicator_http->Cach_Control[0]='\0';
	}
	if(insert_head(buffer,"Upgrade-Insecure-Requests: ",indicator_http->Upgrade_Insecure_Requests)<0){
		indicator_http->Upgrade_Insecure_Requests[0]='\0';
	}
	if(insert_head(buffer,"Origin: ",indicator_http->Origin)<0){
		indicator_http->Origin[0]='\0';
	}
	if(insert_head(buffer,"User-Agent: ",indicator_http->User_Agent)<0){
		indicator_http->User_Agent[0]='\0';
	}
	if(insert_head(buffer,"Accept: ",indicator_http->Accept)<0){
		indicator_http->Accept[0]='\0';
	}
	if(insert_head(buffer,"Referer: ",indicator_http->Referer)<0){
		indicator_http->Referer[0]='\0';
	}
	if(insert_head(buffer,"Accept-Encoding: ",indicator_http->Accept_Encoding)<0){
		indicator_http->Accept_Encoding[0]='\0';
	}
	if(insert_head(buffer,"Accept-Language: ",indicator_http->Accept_Language)<0){
		indicator_http->Accept_Language[0]='\0';
	}

	return 0;
}

char * error_send(void){
	char *response;
	response = malloc(strsize("HTTP/1.1 404 OK\r\n"
				"Content-Type:text/html;charset=utf-8\n\n%s",
				"<html><body><h1>Not found</h1></body></html>"));
	if(response == NULL){
		return NULL;
	}
	sprintf(response, "HTTP/1.1 404 OK\r\n"
				"Content-Type:text/html;charset=utf-8\n\n%s",
				"<html><body><h1>Not found</h1></body></html>");
	return response;
}

char * error_handler(char * buffer, char * explanation, int error, Error * err, int *request_return){
	char path[256];
    char *request;
    request = error_send();
    getcwd(path,256);
    strcat(path,"/error_buffer.txt");

	FILE *html_file = fopen(path,"a");
	if(html_file!=NULL){
		fprintf(html_file,"%s\n\nExplanation = %s\nError number = %d\n\n\n\n",
                buffer, explanation, error);
		fclose(html_file);
		err->a=true;
		strcpy(err->function,explanation);
		err->result=error;
        *request_return = 0;
		return request;
	}else{
        *request_return=-1;
		return request;
	}
}

int give_money(char *name_user0, char *name_user1, char *room, size_t money){
    int passed = 0;
    size_t id_room;
    char * check= str_f_money(money);
    passed = s_room_name_rooms(room, &id_room);
    if(passed !=0){
        return -5;
    }
    if(strcmp(name_user0, "bank\0")==0){//id bank --> name_user1

        size_t id_user1_r, id_user1;
        passed = s_user_name_users(name_user1, &id_user1);
        if(passed !=0){
            return -2;
        }
        passed = s_user_name_room(name_user1, room, &id_user1_r);
        if(passed !=0){
            return -4;
        }
        if(money+rooms[id_room].user[id_user1_r].balance<money){
            return 2;
        }
        //information user in rooms
        rooms[id_room].user[id_user1_r].balance+=money;

        //information lastoperation in users
        users[id_user1].lastoperation.money=money;
        strcpy(users[id_user1].lastoperation.room, rooms[id_room].name_room);
        strcpy(users[id_user1].lastoperation.user2, "bank");
        users[id_user1].lastoperation.typ = 'G';//get
    }else if(strcmp(name_user1, "bank\0")==0){//id name_user1 --> bank
        size_t id_user0_r, id_user0;
        passed = s_user_name_users(name_user0, &id_user0);
        if(passed !=0){
            return -1;
        }
        passed = s_user_name_room(name_user0, room, &id_user0_r);
        if(passed !=0){
            return -3;
        }
        if(money>rooms[id_room].user[id_user0_r].balance){
            return 1;
        }

        //information user in rooms
        rooms[id_room].user[id_user0_r].balance-=money;

        //information lastoperation in users
        users[id_user0].lastoperation.money=money;
        strcpy(users[id_user0].lastoperation.room, rooms[id_room].name_room);
        strcpy(users[id_user0].lastoperation.user2, "bank");
        users[id_user0].lastoperation.typ = 'P';//post
    }else{
        size_t id_user0_r, id_user0;
        size_t id_user1_r, id_user1;
        passed = s_user_name_users(name_user0, &id_user0);
        if(passed !=0){
            return -1;
        }
        passed = s_user_name_users(name_user1, &id_user1);
        if(passed !=0){
            return -2;
        }
        passed = s_user_name_room(name_user0, room, &id_user0_r);
        if(passed !=0){
            return -3;
        }
        passed = s_user_name_room(name_user1, room, &id_user1_r);
        if(passed !=0){
            return -4;
        }
        if(money>rooms[id_room].user[id_user0_r].balance){
            return 1;
        }
        if(money+rooms[id_room].user[id_user1_r].balance<money){
            return 2;
        }
        
        //information user in rooms
        rooms[id_room].user[id_user0_r].balance-=money;
        rooms[id_room].user[id_user1_r].balance+=money;

        //information lastoperation in users
        users[id_user0].lastoperation.money=money;
        strcpy(users[id_user0].lastoperation.room, rooms[id_room].name_room);
        strcpy(users[id_user0].lastoperation.user2, name_user1);
        users[id_user0].lastoperation.typ = 'P';//post

        users[id_user1].lastoperation.money=money;
        strcpy(users[id_user1].lastoperation.room, rooms[id_room].name_room);
        strcpy(users[id_user1].lastoperation.user2, name_user0);
        users[id_user1].lastoperation.typ = 'G';//get
    }

    char s_money[strsize("%lu", money)];
    char s_money_result[sizeof(s_money)+((sizeof(s_money)-1)/3)];
    s_money_result[0]='\0';
    sprintf(s_money, "%lu", money);
    size_t size_s_money = strlen(s_money);
    for(size_t i = size_s_money-1;i>3;i-=3){
        sprintf(s_money_result,"%c%c%c %s",
                s_money[i],s_money[i-1],s_money[i-2], s_money_result);
    }
    char pwd_request[MAX_LENGTH_PWD];
    sprintf(pwd_request,"%s/rooms/%s",
            now_pwd_dirrectory,rooms[id_room].name_room);
    FILE *html_file = fopen(pwd_request,"a");
    if(html_file==NULL){
        return -6;
    }
    fprintf(html_file,"%s <-- %s %s\n\n", name_user1, name_user0, check);
    fclose(html_file);
    free(check);
    return 0;
}

int decision_processing(char * user_request, char * name_room, char * user_answer){
    int passed = 0;
    size_t id_room;
    size_t id_answer;
    passed = s_room_name_rooms(name_room, &id_room);
    char *c;
    if(passed !=0){
        return -1;
    }
    passed = s_request_uRequest_room(user_request, name_room, &id_answer);
    if(passed !=0){
        return -2;
    }
    c = strstr(rooms[id_room].request[id_answer].uninterviewed, user_answer);
    if(c==NULL){
        return -3;
    }
    passed = insert_string(rooms[id_room].request[id_answer].uninterviewed,
            user_answer, "", rooms[id_room].request[id_answer].uninterviewed,
            sizeof(rooms[id_room].request[id_answer].uninterviewed));
    if(passed!=0){
        return -4;
    }
    if(rooms[id_room].request[id_answer].users_unread==1){
        size_t id_user_request;
        passed = s_user_name_room(user_request, rooms[id_room].name_room,
                &id_user_request);
        if(passed != 0){
            return -5;
        }
	passed = give_money("bank", 
			user_request,
			name_room,
			rooms[id_room].request[id_answer].money);
        rooms[id_room].request[id_answer].users_unread=0;
        rooms[id_room].request[id_answer].recipient[0]='\0';
        rooms[id_room].request[id_answer].uninterviewed[0]='\0';
        rooms[id_room].request[id_answer].money = 0;
        for(size_t i=id_answer;i<rooms[id_room].counter_request-1;i++){
            rooms[id_room].request[i]=rooms[id_room].request[i+1];
        }
        rooms[id_room].counter_request-=1;
        return 0;
    }
    rooms[id_room].request[id_answer].users_unread-=1;
    return 1;
} 

int create_processing(char * name_user, char * name_room, size_t money){
    //insert_string
    size_t id_room;
    size_t id_user;
    size_t id_u_r;
    int passed=0;
    passed=s_room_name_rooms(name_room, &id_room);
    if(passed!=0){
        return -1;
    }
    passed=s_user_name_users(name_user, &id_user);
    if(passed!=0){
        return -2;
    }

	Room *r = &rooms[id_room];

    if(r->counter_request>=MAX_REQUEST){
        return -3;
    }
    passed = s_user_name_room(users[id_user].name,
            rooms[id_room].name_room, &id_u_r);
    if(passed!=0){
        return -4;
    }
    if(r->counter_users==1){
	passed = give_money("bank", name_user, name_room, money);
        return passed;
    }
    size_t id_request;
    passed = s_request_uRequest_room(users[id_user].name,
            rooms[id_room].name_room, &id_request);
    for(size_t i = r->counter_request; i>id_request; i--){
        r->request[i] = r->request[i-1];
    }
	r->counter_request+=1;
	r->request[id_request].money = money;
	strcpy(r->request[id_request].recipient, name_user);

	char buffer[MAX_LENGTH*MAX_USERS_IN_ROOM]="";
	for(unsigned char i=0;i<r->counter_users;i++){
		strcat(buffer, r->user[i].name);
	}
	insert_string(buffer, name_user, "", buffer, sizeof(buffer));
	strcpy(r->request[id_request].uninterviewed, buffer);
	r->request[id_request].users_unread= r->counter_users-1;
	return 0;
}

char * view_processing(char * name_user,char * name_room, int *request_return){
    char *response;
	int passed = 0;
    size_t id_room=0;
    passed = s_room_name_rooms(name_room, &id_room);
    if(passed!=0){
        *request_return = -1;
        return NULL;
    }
    if(rooms[id_room].counter_request==0){
        *request_return = 1;
        response = malloc(1);
        *response = '\0';
        return response;
    }
    char process[4*MAX_LENGTH+125];
    char process_result[sizeof(process)*(rooms[id_room].counter_request)];
    process_result[0]='\0';
	for(int i=0;i<rooms[id_room].counter_request;i++){
		if(strstr(rooms[id_room].request[i].uninterviewed, name_user)!=NULL){
			sprintf(process,"<input type=\"checkbox\" id=\"%s\" "
					"name=\"%s\" value=\"%lu\">"
					"<label for=\"%s\"> %s %lu</label><br>\n",
					rooms[id_room].request[i].recipient,
					rooms[id_room].request[i].recipient,
					rooms[id_room].request[i].money,
				   	rooms[id_room].request[i].recipient,
				   	rooms[id_room].request[i].recipient,
				   	rooms[id_room].request[i].money);
			strcat(process_result, process);
		}
	}
    response = malloc(strlen(process_result)+1);
    if(response == NULL){
        *request_return=-2;
        return NULL;
    }
    strcpy(response, process_result);
	return response;
}

int connect_room(char *name, char *name_room){
    if(strlen(name)==0){
        return -3;
    }
    size_t id_room=0;
	size_t id_user=0;
    int passed =0;
    passed = s_room_name_rooms(name_room, &id_room);
    if(passed!=0){
        return -1;
    }
    passed = s_user_name_users(name, &id_user);
	if(passed!=0){
		return -2;
	}

    size_t id_user_in_room=0;
    passed = s_user_name_room(name, name_room, &id_user_in_room);
	if(passed>0){
        if(rooms[id_room].counter_users!=0){
            for(size_t i = rooms[id_room].counter_users; i>id_user_in_room; i--){
                rooms[id_room].user[i] = rooms[id_room].user[i-1];
            }
        }
		strcpy(rooms[id_room].user[id_user_in_room].name, name);
		strcpy(rooms[id_room].user[id_user_in_room].login, users[id_user].login);
		strcat(rooms[id_room].users_names, name);
        rooms[id_room].user[id_user_in_room].balance=15000000;
		rooms[id_room].counter_users += 1;
		return 0;
	}else if(passed==0){
		return 1;
	}else{
        return -3;
    }
}

char * send_history(size_t id_room, int * request_return){
    char *request;
    int passed=0;
    char url_room[MAX_LENGTH_PWD];
    if((num_rooms==0)||(id_room>=num_rooms)){
        *request_return = -1;
        request = malloc(1);
        if(request==NULL){
            return NULL;
        }
        *request = '\0';
        return request;
    }
    sprintf(url_room, "./rooms/%s", rooms[id_room].name_room);

    FILE *file = fopen(url_room, "r");
    if(file==NULL){
        *request_return = -4;
        request = malloc(1);
        if(request==NULL){
            *request_return = -1000;
            return NULL;
        }
        *request = '\0';
        return request;
    }
    size_t count = 0;
    char s[MAX_LENGTH*2 +45];       //2 names and money
    *s='\0';
    char lines[MAX_COUNT_HISTORY][sizeof(s)];
    *lines[MAX_COUNT_HISTORY-1] = '\0';

    while(fgets(s, sizeof(s), file)!= NULL){
        strncpy(lines[count], s, sizeof(s));
        count = (count+1) % MAX_COUNT_HISTORY;
    }
    fclose(file);
    request = malloc(sizeof(lines));
    *request = '\0';

    if(strlen(lines[MAX_COUNT_HISTORY-1])==0){
        if(count==0){
            return request;
        }
        for(size_t i =0; i < count; i++){
            strcat(request , lines[i]);
            strcat(request, "<br>\n");
        }
    }else{
        for(size_t i =0; i < MAX_COUNT_HISTORY; ++i){
            strcat(request , lines[(count+i)%MAX_COUNT_HISTORY]);
            strcat(request, "<br>\n");
        }
    }
    printf("history send = %s\n\n", request);

    return request;
}

char * handle_html(char *buffer,char *html_doc,int *request_return, size_t *information_for_html){
    int passed = 0;
	size_t passeduns=0;//return function(unsigned)
    Error err;
    char *response;
    char explanation[100];
    if(strcmp(html_doc,htmls[0].name_url_html)==0){
        // /
        size_t buffer_size = correct_size(htmls[0].name_url_html);
        if (buffer_size == 0){
            *request_return = 1;
            sprintf(explanation, "correct_size = %lu /", buffer_size);
            response=error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return=-1000;
                return NULL;
            }
            if(passed<0){
                *request_return = -(*request_return);
            }
            return response;
        }
        char buffer_file[buffer_size];
        passed = send_html(htmls[0].name_url_html, buffer_file);
        if(passed<0){
            *request_return = 2;
            sprintf(explanation, "send_html = %d /", passed);
            response=error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return=-1000;
                return NULL;
            }
            if(passed<0){
                *request_return = -(*request_return);
            }
            return response;
        }
        response = malloc(buffer_size);
        if(response == NULL){
            *request_return = -1000;
            return NULL;
        }
        strcpy(response, buffer_file);
    }else if(strcmp(html_doc,htmls[1].name_url_html)==0){
        // /registration
        size_t buffer_size = correct_size(htmls[1].name_url_html);
        if(buffer_size==0){
            *request_return = 1;
            sprintf(explanation, "correct_size=%lu", buffer_size);
            response=error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return=-1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        char buffer_file[buffer_size];
        passed = send_html(htmls[1].name_url_html, buffer_file);
        if(passed<0){
            *request_return = 2;
            sprintf(explanation, "send_html=%d", passed);
            response=error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return=-1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        response=malloc(buffer_size);
        if(response == NULL){
            *request_return = -1000;
            return NULL;
        }
        strcpy(response, buffer_file);
    }else if(strcmp(html_doc,htmls[2].name_url_html)==0){
        // /rooms
        size_t id_user = *information_for_html;
        size_t buffer_size = correct_size(htmls[2].name_url_html);
        if(buffer_size==0){
            *request_return = 1;
            sprintf(explanation, "correct_size=%lu", buffer_size);
            response=error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return=-1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        char *check;
        //insert rooms
        if(num_rooms!=0){
            char buffer_room[(MAX_LENGTH+MAX_LENGTH+31)];
            check = malloc(sizeof(buffer_room)*num_rooms);
            if(check == NULL){
                *request_return = -1000;
                return NULL;
            }
            *check='\0';
            for(size_t i=0;i<num_rooms;i++){
                sprintf(buffer_room,
                        "<option value=\"%s\">%s</option>\n",
                        rooms[i].name_room,
                        rooms[i].name_room);
                strcat(check, buffer_room);
            }
        }else{
            check = malloc(1);
            *check ='\0';
        }

        char buffer_file[buffer_size+strlen(check)+strlen(users[id_user].name)];
        passed = send_html(htmls[2].name_url_html, buffer_file);
        if(passed<0){
            *request_return = 2;
            sprintf(explanation, "send_html=%d", passed);
            response=error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return=-1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        passed = insert_string(buffer_file, "<!-- INSERT_ROOMS -->",
                check, buffer_file, sizeof(buffer_file));
        if(passed<0){
            *request_return = 3;
            sprintf(explanation, "<!-- INSERT_ROOMS --> insert_string=%d", passed);
            response=error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return=-1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        free(check);
        check=NULL;
        
        //insert name
        passed = insert_string(buffer_file, "<!-- INSERT_THIS_USER -->",
                users[id_user].name,
                buffer_file, sizeof(buffer_file));
        if(passed<0){
            *request_return = 4;
            sprintf(explanation, "<!-- INSERT_THIS_USER --> insert_string=%d", passed);
            response=error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return=-1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        response=malloc(strlen(buffer_file)+1);
        if(response == NULL){
            *request_return = -1000;
            return NULL;
        }
        strcpy(response, buffer_file);
    }else if(strcmp(html_doc,htmls[3].name_url_html)==0){
        // /create_room
        size_t buffer_size = correct_size(htmls[3].name_url_html);
        if(buffer_size==0){
            *request_return = 1;
            strcpy(explanation, "correct_size /create_room");
            response=error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return=-1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        char buffer_file[buffer_size];
        passed=send_html(htmls[3].name_url_html,buffer_file);
        if(passed!=0){
            *request_return = 2;
            sprintf(explanation,"send_html /create_room = %d", passed);
            response=error_handler(buffer, explanation, 
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return=-1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        response=malloc(buffer_size);
        if(response == NULL){
            *request_return = -1000;
            return NULL;
        }
        strcpy(response, buffer_file);
    }else if(strcmp(html_doc,htmls[4].name_url_html)==0){
        // /rooms/&
        size_t id_user = *information_for_html;
        information_for_html += 1;
        size_t id_room = *information_for_html;
        size_t buffer_size= correct_size(htmls[4].name_url_html);
        if(buffer_size==0){
            *request_return = 1;
            strcpy(explanation,"correct_size = 0 /rooms/& ");
            response=error_handler(buffer, explanation, 
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return=-1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        buffer_size+=45;        //for balanse
        buffer_size+=(40+2*MAX_LENGTH)*rooms[id_room].counter_users+
            MAX_LENGTH+10;    //for users
        char buffer_file[buffer_size];
        passed = send_html(htmls[4].name_url_html, buffer_file);
        if(passed!=0){
            *request_return = 2;
            sprintf(explanation, "send_html = %d", passed);
            response=error_handler(buffer, explanation, 
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return=-1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        char *check;
        //for balanse
        check=malloc(30);
        if(check == NULL){
            *request_return = -1000;
            return NULL;
        }
        size_t id_user_room=0;
        passed=s_user_name_room(users[id_user].name,
                rooms[id_room].name_room, &id_user_room);
        if(passed<0){
            *request_return = 3;
            sprintf(explanation, "s_user_name_room = %d", passed);
            response=error_handler(buffer, explanation, 
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return=-1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
	check = str_f_money(rooms[id_room].user[id_user_room].balance);
        passed = insert_string(buffer_file, "<!-- INSERT_MONEY_USER -->",
                check, buffer_file, sizeof(buffer_file));
        free(check);
        check=NULL;
        if(passed!=0){
            *request_return = 4;
            sprintf(explanation, "insert_string = %d", passed);
            response=error_handler(buffer, explanation, 
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return=-1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        /////////////////////////////////////
        passed = insert_string(buffer_file, "<!-- INSERT_ROOM -->",
                rooms[id_room].name_room,
                buffer_file, sizeof(buffer_file));
        if(passed!=0){
            *request_return = 6;
            sprintf(explanation, "insert_string = %d", passed);
            response=error_handler(buffer, explanation, 
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return=-1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        //for users 
        if(rooms[id_room].counter_users>1){
            char buffer_user[45+(2*MAX_LENGTH)];
            char buffer_users[sizeof(buffer_user)*
                rooms[id_room].counter_users];
            buffer_users[0]='\0';
            for(size_t i=0;i<rooms[id_room].counter_users;i++){
                if(strcmp(rooms[id_room].user[i].name,
                            users[id_user].name)!=0){
                    sprintf(buffer_user, "<option value=\"%s\">%s</option>\n",
                            rooms[id_room].user[i].name,
                            rooms[id_room].user[i].name);
                    strcat(buffer_users, buffer_user);
                }
            }
            check=malloc(strlen(buffer_users)+1);
            if(check == NULL){
                *request_return = -1000;
                return NULL;
            }
            strcpy(check, buffer_users);
        }else{
            check=malloc(1);
            if(check == NULL){
                *request_return = -1000;
                return NULL;
            }
            strcpy(check,"\0");
        }
        passed = insert_string(buffer_file, "<!-- INSERT_USERS -->",
                check, buffer_file, sizeof(buffer_file));
        free(check);
        check=NULL;
        if(passed!=0){
            *request_return = 5;
            sprintf(explanation, "insert_string = %d", passed);
            response=error_handler(buffer, explanation, 
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return=-1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        //////////////////////////////////////////////////////////
        response = malloc(buffer_size);
        if(response == NULL){
            *request_return = -1000;
            return NULL;
        }
        strcpy(response, buffer_file);
    }else if(strcmp(html_doc,htmls[5].name_url_html)==0){
        // /rooms/&/confirmation
        size_t id_user = *information_for_html;
        information_for_html+=1;
        size_t id_room = *information_for_html;
        size_t buffer_size= correct_size(htmls[5].name_url_html);
        if(buffer_size==0){
            *request_return = 1; 
            sprintf(explanation,"correct_size = %d /rooms/& ", passed);
            response = error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return = -1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        buffer_size+=MAX_LENGTH;    //for user selection
        buffer_size+=30;             //for money
        buffer_size+=MAX_LENGTH;    //for name room
        char buffer_file[buffer_size];
        passed = send_html(htmls[5].name_url_html, buffer_file);
        if(passed!=0){
            *request_return = 2;
            sprintf(explanation, "send_html = %d", passed);
            response = error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return = -1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        //for money
        char *check;
        check=malloc(45);
        if(check == NULL){
            *request_return = -1000;
            return NULL;
        }
	check = str_f_money(users[id_user].lastoperation.money);
        insert_string(buffer_file, "<!-- MONEY_COAST -->",
                check, buffer_file, sizeof(buffer_file));
        free(check);
        check=NULL;
        //for user selection
        insert_string(buffer_file, "<!-- USER_SELECT -->",
                users[id_user].lastoperation.user2,
                buffer_file, sizeof(buffer_file));
        //for name room
        insert_string(buffer_file, "<!-- INSERT_ROOM -->",
                rooms[id_room].name_room, buffer_file, sizeof(buffer_file));

        //////////////////////////////////////////////////
        response = malloc(buffer_size);
        if(response == NULL){
            *request_return = -1000;
            return NULL;
        }
        strcpy(response, buffer_file);
    }else if(strcmp(html_doc,htmls[6].name_url_html)==0){
        // /rooms/&/confirmaion_bank
        char * process;
        size_t id_user = *information_for_html;
        information_for_html+=1;
        size_t id_room = *information_for_html; 
        size_t buffer_size= correct_size(htmls[6].name_url_html);

        if(buffer_size==0){
            *request_return = 1;
            strcpy(explanation, "correct_size = 0 /rooms/& ");
            response = error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return = -1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        process = view_processing(users[id_user].name, rooms[id_room].name_room,
                &passed);
        if(passed<0){
            *request_return = 2;
            sprintf(explanation, "view_processing = %d", passed);
            response = error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return = -1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        buffer_size+=strlen(process);
        buffer_size+=MAX_LENGTH;            //for name_room
        char buffer_file[buffer_size];
        passed = send_html(htmls[6].name_url_html, buffer_file);
        if(passed!=0){
            free(process);
            process=NULL;
            *request_return =  3;
            sprintf(explanation, "send_html = %d", passed);
            response = error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return = -1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        //for process
        insert_string(buffer_file, "<!-- INPUT_REQUEST -->",
                process, buffer_file, sizeof(buffer_file));
        free(process);
        process=NULL;
        insert_string(buffer_file, "<!-- INSERT_ROOM -->",
                rooms[id_room].name_room, buffer_file, sizeof(buffer_file));
        response = malloc(buffer_size);
        if(response == NULL){
            *request_return = -1000;
            return NULL;
        }
        strcpy(response, buffer_file);
    }else if(strcmp(html_doc,htmls[7].name_url_html)==0){
        // /rooms/&/history
        size_t id_room = *information_for_html;
        char * check;
        check = send_history(id_room, &passed);
        if(passed != 0){

            *request_return =  1;
            sprintf(explanation, "Send_history = %d", passed);
            response = error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return = -1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        size_t buffer_size= correct_size(htmls[7].name_url_html);
        if(buffer_size==0){
            *request_return = 2;
            strcpy(explanation, "correct_size = 0 /rooms/&/history ");
            response = error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return = -1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        buffer_size+=strlen(check);
        buffer_size+=MAX_LENGTH;        //for name_room
        char buffer_file[buffer_size];
        passed = send_html(htmls[7].name_url_html, buffer_file);
        if(passed!=0){
            *request_return =  3;
            sprintf(explanation, "send_html = %d", passed);
            response = error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return = -1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        //for process
        insert_string(buffer_file, "<!-- INSERT_HEAR -->",
                check, buffer_file, sizeof(buffer_file));
        free(check);
        check=NULL;
        insert_string(buffer_file, "<!-- INSERT_ROOM -->",
                rooms[id_room].name_room, buffer_file, sizeof(buffer_file));
        response = malloc(buffer_size);
        if(response == NULL){
            *request_return = -1000;
            return NULL;
        }
        strcpy(response, buffer_file);
    }else if(strcmp(html_doc,htmls[8].name_url_html)==0){
        // /profile/{name_user}
        size_t buffer_size = correct_size(htmls[8].name_url_html);
        if(buffer_size==0){
            *request_return = 1;
            strcpy(explanation, "correct_size = 0");
            response = error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return = -1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        buffer_size+=strlen(users[*information_for_html].login);
        buffer_size+=strlen(users[*information_for_html].name);
        buffer_size+=strlen(users[*information_for_html].password);
        buffer_size+=strlen(users[*information_for_html].IP);
        char buffer_file[buffer_size];
        passed = send_html(htmls[8].name_url_html, buffer_file);
        if(passed<0){
            *request_return = 2;
            sprintf(explanation, "send_html=%d", passed);
            response = error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return = -1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        //insert login
        passed = insert_string(buffer_file,"<!-- LOGIN_THIS_USER -->\0",
                users[*information_for_html].login,
                buffer_file, buffer_size);
        if(passed<0){
            *request_return = 3;
            sprintf(explanation, "insert_string=%d", passed);
            response = error_handler(buffer, explanation,
                    *request_return, &err,&passed);
            if(response == NULL){
                *request_return = -1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        //insert name
        passed = insert_string(buffer_file,"<!-- NAME_THIS_USER -->\0",
                users[*information_for_html].name,
                buffer_file, buffer_size);
        if(passed<0){
            *request_return =  4;
            sprintf(explanation, "insert_string=%d", passed);
            response = error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return = -1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        //insert password
        passed = insert_string(buffer_file,"<!-- PASSWORD_THIS_USER -->\0",
                users[*information_for_html].password,
                buffer_file, buffer_size);
        if(passed<0){
            *request_return =  5;
            sprintf(explanation, "insert_string=%d", passed);
            response = error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return = -1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        //insert IP
        passed = insert_string(buffer_file,"<!-- IP_THIS_USER -->\0",
                users[*information_for_html].IP,
                buffer_file, buffer_size);
        if(passed<0){
            *request_return =  6;
            sprintf(explanation, "insert_string=%d", passed);
            response = error_handler(buffer, explanation,
                    *request_return, &err, &passed);
            if(response == NULL){
                *request_return = -1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
        response=malloc(strlen(buffer_file)+1);
        if(response == NULL){
            *request_return = -1000;
            return NULL;
        }
        strcpy(response, buffer_file);
    }else if(strcmp(html_doc,htmls[9].name_url_html)==0){
        puts("This html not create");
        *request_return = -1000;
        return NULL;
    }else{
        puts("handle_html: WHAT?!");
        *request_return = -1000;

        return NULL;
    }
    return response;
}

char * handle_request(char *buffer, char *IP, int *request_return, Error *err){
	char *response; //result
    char *response_html;
	int passed=0;//return function(int)
	size_t passeduns=0;//return function(unsigned)
	char explanation[100];//correct problem
	HTTP_Context HTTP_ex;
	create_http_struct(buffer, &HTTP_ex);
//CHECK HTTP PROTOCOL
	if (strcmp(HTTP_ex.Protocol,"HTTP/1.1")!=0){
        *request_return = 1;
        sprintf(explanation, "HTTP_ex.Protocol== %s", HTTP_ex.Protocol);
		response = error_handler(buffer, explanation,
                *request_return, err, &passed);
		if(response == NULL){
			*request_return = -1000;
			return NULL;
		}
        if(passed < 0){
            *request_return = -(*request_return);
        }
        return response;
    }
//INSERT IN BUFFER.txt
	char pwd_request[MAX_LENGTH_PWD];
    sprintf(pwd_request,"%s/%s",now_pwd_dirrectory,"buffer.txt");
    FILE *html_file = fopen(pwd_request,"a");
	if(html_file==NULL){
        *request_return = 2;
        strcpy(explanation, HTTP_ex.Protocol);
		response = error_handler(buffer, explanation, 
                *request_return, err, &passed);
		if(response == NULL){
			*request_return = -1000;
			return NULL;
		}
        if(passed < 0){
            *request_return = -(*request_return);
        }
        return response;
	}
	fprintf(html_file,"\n\nNext:\n\n%s",buffer);
    fclose(html_file);
    size_t information_for_html[3];
    information_for_html[0]=0;
    information_for_html[1]=0;
    information_for_html[2]=0;
    
    if(!((strcmp(HTTP_ex.URL, "/registration\0")==0)||
            (strcmp(HTTP_ex.URL, "/\0")==0))&&
        !(s_user_IP_users(IP, &passeduns)==0)){
        response = malloc(strsize("HTTP/1.1 303 See Other\r\n"
                "Content-Type: text/html; charset=utf-8\r\n"
                "Location: /registration\n\n")+1);
        if(response == NULL){
            *request_return = -1000;
            return NULL;
        }
        strcpy(response, "HTTP/1.1 303 See Other\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
                "Location: /registration\n\n");
        return response;
    }
//CREATE ANSWER
	if(strcmp(HTTP_ex.Method,"GET\0")==0){
	//GET:
		if((strcmp("/\0", HTTP_ex.URL)==0)||(strcmp("/login\0", HTTP_ex.URL)==0)){
        // /
            response_html=handle_html(buffer, htmls[0].name_url_html,
                    request_return, information_for_html);
            if(*request_return!=0){
                return response_html;
            }
            response = malloc(strsize("HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html;"
            	    "charset=utf-8\n\n%s",
			        response_html));
            sprintf(response,
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html;"
                    "charset=utf-8\n\n%s",
				    response_html);
            free(response_html);
		}else if(strcmp("/registration\0",HTTP_ex.URL)==0){
		// /registration
            response_html=handle_html(buffer, htmls[1].name_url_html,
                    request_return, information_for_html);
            if(*request_return!=0){
                return response_html;
            }
            response = malloc(strsize("HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html;"
                    "charset=utf-8\n\n%s",
                    response_html));
            sprintf(response,
				   	"HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html;"
                    "charset=utf-8\n\n%s",
				    response_html);
            free(response_html);
		}else if(strcmp("/rooms\0",HTTP_ex.URL)==0){
        // /rooms
            size_t id_user;
            passed=s_user_IP_users(IP, &id_user);
            if(passed!=0){
                *request_return = 1;
				sprintf(explanation, "s_user_IP_users = %d", passed);
                response = error_handler(buffer, explanation,
                        *request_return, err, &passed);
                if(response == NULL){
                    *request_return = -1000;
                    return NULL;
                }
                if(passed < 0){
                    *request_return = -(*request_return);
                }
                return response;
			}
            information_for_html[0]=id_user;
            response_html=handle_html(buffer, htmls[2].name_url_html,
                    request_return, information_for_html);
            if(*request_return!=0){
                return response_html;
            }
            response=malloc(strsize("HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html; charset=utf-8\n\n%s",
                    response_html));
            sprintf(response, "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html; charset=utf-8\n\n%s",
                    response_html);
            free(response_html);
        }else if(strcmp("/create_room\0", HTTP_ex.URL)==0){
            // /create_room
            response_html=handle_html(buffer, htmls[3].name_url_html,
                    request_return, information_for_html);
            if(*request_return!=0){
                return response_html;
            }
            response = malloc(strsize("HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html;"
					"charset=utf-8\n\n%s",
				    response_html));
            sprintf(response,
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html;"
					"charset=utf-8\n\n%s",
				    response_html);
            free(response_html);
        }else if(strcmp("/profile\0", HTTP_ex.URL)==0){
            // quest 1(add all profile and information for his)
			*request_return = 1;
            strcpy(explanation , "no url");
            response = error_handler(buffer, explanation,
                    *request_return, err, &passed);
            if(response == NULL){
                *request_return = -1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }else if(strncmp("/profile/", HTTP_ex.URL, 9)==0){
            // /profile/{name_user}
            size_t id_user;
            char *urlCounter=HTTP_ex.URL;
            urlCounter+=9;
            passed = s_user_name_users(urlCounter, &id_user);
            if(passed!=0){
                *request_return = 1;
				sprintf(explanation, "s_user_name_users=%d", passed);
                response = error_handler(buffer, explanation,
                        *request_return, err, &passed);
                if(response == NULL){
                    *request_return = -1000;
                    return NULL;
                }
                if(passed < 0){
                    *request_return = -(*request_return);
                }
                return response;
			}
            information_for_html[0]=id_user;
            response_html=handle_html(buffer, htmls[8].name_url_html,
                    request_return, information_for_html);
            if(*request_return!=0){
                return response_html;
            }
            response=malloc(strsize("HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html; charset=utf-8\n\n%s",
                    response_html));
            sprintf(response, "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html; charset=utf-8\n\n%s",
                    response_html);
            free(response_html);
        }else if(strncmp("/rooms/", HTTP_ex.URL, 7)==0){
            // /rooms/...
            char *index_symbol = HTTP_ex.URL;
            index_symbol+=7;
            size_t id_room;
            char name_room[MAX_LENGTH];
            size_t index_name_room = 0;
            while((*index_symbol!='\0')&&(*index_symbol!='/')){
                name_room[index_name_room] = *index_symbol;
                index_name_room+=1;
                index_symbol+=1;
            }
            if(*index_symbol!='\0'){
                index_symbol+=1;
            }
            name_room[index_name_room] = '\0';
            if(num_rooms==0){
                *request_return = 1;
                strcpy(explanation, "num_rooms == 0 /rooms/...");
                response = error_handler(buffer, explanation,
                        *request_return, err, &passed);
                if(response == NULL){
                    *request_return = -1000;
                    return NULL;
                }
                if(passed < 0){
                    *request_return = -(*request_return);
                }
                return response;
            }
            passed = s_room_name_rooms(name_room, &id_room);
            if(passed!=0){
                *request_return = 2;
                strcpy(explanation, "room_name not found /rooms/...");
                response = error_handler(buffer, explanation,
                        *request_return, err, &passed);
                if(response == NULL){
                    *request_return = -1000;
                    return NULL;
                }
                if(passed < 0){
                    *request_return = -(*request_return);
                }
                return response;
            }
            size_t index_user;
           
            passed = s_user_IP_users(IP, &index_user);
            if(passed!=0){
                *request_return = 3;
                sprintf(explanation, "s_user_IP_users = %d", passed); 
                response = error_handler(buffer, explanation,
                        *request_return, err, &passed);
                if(response == NULL){
                    *request_return = -1000;
                    return NULL;
                }
                if(passed < 0){
                    *request_return = -(*request_return);
                }
                return response;
            }
            if(*index_symbol=='\0'){
                information_for_html[0]=index_user;
                information_for_html[1]=id_room;
                response_html = handle_html(buffer, htmls[4].name_url_html,
                        request_return, information_for_html);
                if(*request_return!=0){
                    return response_html;
                }
                response= malloc(strsize("HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/html; charset=utf-8\n\n%s",
                            response_html));
                sprintf(response, "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/html; charset=utf-8\n\n%s",
                            response_html);
            }else if(strncmp("confirmation", index_symbol,12)==0){
                index_symbol+=12;
                if(*index_symbol=='\0'){
                    information_for_html[0]=index_user;
                    information_for_html[1]=id_room;
                    response_html = handle_html(buffer, htmls[5].name_url_html,
                            request_return, information_for_html);
                    if(*request_return!=0){
                        return response_html;
                    }
                    response= malloc(strsize("HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/html; charset=utf-8\n\n%s",
                                response_html));
                    sprintf(response, "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/html; charset=utf-8\n\n%s",
                                response_html);
                }else if(strncmp("_bank", index_symbol,5)==0){
                    information_for_html[0]=index_user;
                    information_for_html[1]=id_room;
                    response_html = handle_html(buffer, htmls[6].name_url_html,
                            request_return, information_for_html);
                    if(*request_return!=0){
                        return response_html;
                    }
                    response= malloc(strsize("HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/html; charset=utf-8\n\n%s",
                                response_html));
                    sprintf(response, "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/html; charset=utf-8\n\n%s",
                                response_html);
                }else{
                    *request_return = 4;
                    sprintf(explanation, "no url ", passed);
                    response = error_handler(buffer, explanation,
                            *request_return, err, &passed);
                    if(response == NULL){
                        *request_return = -1000;
                        return NULL;
                    }
                    if(passed < 0){
                        *request_return = -(*request_return);
                    }
                    return response;
                }
            }else if(strncmp("history", index_symbol,7)==0){
                information_for_html[0]=id_room;
                response_html = handle_html(buffer, htmls[7].name_url_html,
                        request_return, information_for_html);
                if(*request_return!=0){
                    return response_html;
                }
                response= malloc(strsize("HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/html; charset=utf-8\n\n%s",
                            response_html));
                sprintf(response, "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/html; charset=utf-8\n\n%s",
                            response_html);

            }else{
                *request_return = 5;
                strcpy(explanation, "no url");
                response = error_handler(buffer, explanation,
                        *request_return, err, &passed);
                if(response == NULL){
                    *request_return = -1000;
                    return NULL;
                }
                if(passed < 0){
                    *request_return = -(*request_return);
                }
                return response;
            }
		}else if(strcmp("/favicon.ico\0", HTTP_ex.URL)){
            response = malloc(strsize("HTTP/1.1 404 OK\n"
                        "Content-Type:text/html;charset=utf-8\n\n%s",
                    "<html><body><h1>NOT USE METHOD DEL</h1></body></html>"));
            if(response == NULL){
                *request_return = -1000;
                return NULL;
            }
            sprintf(response, "HTTP/1.1 404 OK\n"
                        "Content-Type:text/html;charset=utf-8\n\n%s",
                    "<html><body><h1>NOT USE METHOD DEL</h1></body></html>");
            return response;
        }else{
			*request_return = 1;
            strcpy(explanation, "no url");
            response = error_handler(buffer, explanation,
                    *request_return, err, &passed);
            if(response == NULL){
                *request_return = -1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
        }
    }else if (strcmp("POST\0", HTTP_ex.Method)==0){
//POST
		if((strcmp("/\0", HTTP_ex.URL)==0)||(strcmp("/login\0", HTTP_ex.URL)==0)){
			size_t index_user;
			char login[MAX_LENGTH];
			char password[MAX_LENGTH];
			passed=insert_field(buffer, "login=", login);
            if(passed==0){
                passed=insert_field(buffer, "password=", password);
				if(passed==0){
					passed = s_user_login_users(login, &index_user);
                    if(passed==0){
                        if(strcmp(users[index_user].password,password)==0){
                            size_t id_f_search;
                            char old_IP[MAX_LENGTH];
                            passed = s_user_IP_users(IP, &id_f_search);
                            if(passed>0){
                                //if user not in base
                                for(size_t i=num_IPs;i>id_f_search;i--){
                                    IPs[i]=IPs[i-1];
                                }
                                if(num_IPs+1<=MAX_IPs){
                                    num_IPs+=1;
                                }else{
                                    for(size_t i=id_f_search;i<num_IPs;i++){
                                        IPs[i]=IPs[i+1];
                                    }
                                    *request_return = 2;
                                    passed = -1;
                                    sprintf(explanation,
                                            "MAX_IPs < %lu /registration", num_IPs);
                                    response = error_handler(buffer, explanation, 
                                            *request_return, err, &passed);
                                    if(response == NULL){
                                        *request_return = -1000;
                                        return NULL;
                                    }
                                    if(passed < 0){
                                        *request_return = -(*request_return);
                                    }
                                    return response;
                                }
                                strcpy(IPs[id_f_search].IP, IP);
                                strcpy(IPs[id_f_search].login, login);
                                strcpy(old_IP, users[index_user].IP);
                                strcpy(users[index_user].IP, IP);
                                passed = s_user_IP_users(IP, &id_f_search);
                                IPs[id_f_search].login[0]='\0';
                            }else if(passed<0){
                                //user in base
                                strcpy(IPs[id_f_search].login, login);
                                strcpy(users[index_user].IP, IP);
                            }
                            response = malloc(strsize("HTTP/1.1 303 See Other\r\n"
                                    "Content-Type: text/html; charset=utf-8\r\n"
                                    "Location: /profile/%s\n\n",
                                    users[index_user].name));
                            if(response == NULL){
                                *request_return = -1000;
                                return NULL;
                            }
                            sprintf(response, "HTTP/1.1 303 See Other\r\n"
                                    "Content-Type: text/html; charset=utf-8\r\n"
                                    "Location: /profile/%s\n\n",
                                    users[index_user].name);
                        }else{
                            response = malloc(strsize("HTTP/1.1 205 Reset Content\r\n"
                                    "Content-Type: text/html; charset=utf-8\r\n"
                                    "Location: /\n\n"));
                            if(response == NULL){
                                *request_return = -1000;
                                return NULL;
                            }
                            *request_return = 1;
                            strcpy(response, "HTTP/1.1 205 Reset Content\r\n"
                                    "Content-Type: text/html; charset=utf-8\r\n"
                                    "Location: /\n\n");
                        }
                    }else{
                        response = malloc(strsize("HTTP/1.1 205 Reset Content\r\n"
                                "Content-Type: text/html; charset=utf-8\r\n"
                                "Location: /\n\n"));
                        if(response == NULL){
                            *request_return = -1000;
                            return NULL;
                        }
                        *request_return = 2;
                        strcpy(response, "HTTP/1.1 205 Reset Content\r\n"
                                "Content-Type: text/html; charset=utf-8\r\n"
                                "Location: /\n\n");
                   }
				}else{
                    *request_return = 3;
                    sprintf(explanation, "/ not found password = %d",*request_return);
                    response = error_handler(buffer, explanation, 
                            *request_return, err, &passed);
                    if(response == NULL){
                        *request_return = -1000;
                        return NULL;
                    }
                    if(passed < 0){
                        *request_return = -(*request_return);
                    }
                    return response;
				}
			}else{
                *request_return = 4;
                sprintf(explanation, "/ not found login = %d",*request_return);
                response = error_handler(buffer, explanation, 
                        *request_return, err, &passed);
                if(response == NULL){
                    *request_return = -1000;
                    return NULL;
                }
                if(passed < 0){
                    *request_return = -(*request_return);
                }
                return response;
			}
		}else if(strcmp("/registration\0", HTTP_ex.URL)==0){
			char login[MAX_LENGTH];
			char name[MAX_LENGTH];
			char password[MAX_LENGTH];
			if(((insert_field(buffer, "login=", login))!=0)||
				((insert_field(buffer, "username=", name))!=0)||
				(insert_field(buffer, "password=", password)!=0)){
                *request_return = 1;
                sprintf(explanation, "/ not found password or login or username= %d",
                        *request_return);
                response = error_handler(buffer, explanation, 
                        *request_return, err, &passed);
                if(response == NULL){
                    *request_return = -1000;
                    return NULL;
                }
                if(passed < 0){
                    *request_return = -(*request_return);
                }
                return response;
			}
            size_t id_user;
            passed=insert_account(login, name, password, IP);
            if(passed==0){
                passed = s_user_IP_users(IP, &id_user);
                response = malloc(strsize("HTTP/1.1 303 See Other\r\n"
                        "Content-Type: text/html; charset=utf-8\r\n"
                        "Location: /profile/%s\n\n",
                        users[id_user].name));
                if(response == NULL){
                    *request_return = -1000;
                    return NULL;
                }
                sprintf(response, "HTTP/1.1 303 See Other\r\n"
                        "Content-Type: text/html; charset=utf-8\r\n"
                        "Location: /profile/%s\n\n",
                        users[id_user].name);
            }else{
                if(passed==-1){
                    puts("Login in base");
                }else if(passed==-2){
                    puts("Name in base"); 
                }else if(passed==-4){
                    puts("Problem with insert base"); 
                }
                response = malloc(strsize("HTTP/1.1 205 Reset Content\r\n"
                        "Content-Type: text/html; charset=utf-8\r\n"
                        "Location: /registration\n\n"));
                if(response == NULL){
                    *request_return = -1000;
                    return NULL;
                }
                *request_return = 1;
                strcpy(response, "HTTP/1.1 205 Reset Content\r\n"
                        "Content-Type: text/html; charset=utf-8\r\n"
                        "Location: /registration\n\n");
            }
		}else if(strcmp("/rooms\0", HTTP_ex.URL)==0){
			char name_room[MAX_LENGTH];
			if(insert_field(buffer, "room=", name_room)==0){
                size_t id_user;
                passed= s_user_IP_users(IP, &id_user);
                if(passed!=0){
                    *request_return = 1;
                    sprintf(explanation, "/rooms s_user_IP_users = %d",passed);
                    response = error_handler(buffer, explanation, 
                            *request_return, err, &passed);
                    if(response == NULL){
                        *request_return = -1000;
                        return NULL;
                    }
                    if(passed < 0){
                        *request_return = -(*request_return);
                    }
                    return response;
                }
                passed = connect_room(users[id_user].name, name_room);
                if(passed<0){
                    *request_return = 2;
                    sprintf(explanation, "/rooms connect_room = %d",passed);
                    response = error_handler(buffer, explanation, 
                            *request_return, err, &passed);
                    if(response == NULL){
                        *request_return = -1000;
                        return NULL;
                    }
                    if(passed < 0){
                        *request_return = -(*request_return);
                    }
                    return response;
                }
        //cut this
        printf("Connect_room = %d \n", passed);
        //cut this
				response = malloc(strsize("HTTP/1.1 303 See Other\r\n"
						"Content-Type: text/html; charset=utf-8\r\n"
						"Location: /rooms/%s\n\n",name_room));
				if(response == NULL){
					*request_return = -1000;
					return NULL;
				}
			   	sprintf(response,"HTTP/1.1 303 See Other\r\n"
						"Content-Type: text/html; charset=utf-8\r\n"
						"Location: /rooms/%s\n\n",name_room);

			}else{
                response = malloc(strsize("HTTP/1.1 205 Reset Content\r\n"
                        "Content-Type: text/html; charset=utf-8\r\n"
                        "Location: /rooms\n\n"));
                if(response == NULL){
                    *request_return = -1000;
                    return NULL;
                }
                strcpy(response, "HTTP/1.1 205 Reset Content\r\n"
                        "Content-Type: text/html; charset=utf-8\r\n"
                        "Location: /rooms\n\n");
			}
		}else if(strcmp("/create_room\0",HTTP_ex.URL)==0){
			char name_room[MAX_LENGTH];
            passed = insert_field(buffer, "name=", name_room);
			if(passed!=0){
                *request_return = 1;
                sprintf(explanation, "insert_field = %d /create_room", passed);
                response = error_handler(buffer, explanation, 
                        *request_return, err, &passed);
                if(response == NULL){
                    *request_return = -1000;
                    return NULL;
                }
                if(passed < 0){
                    *request_return = -(*request_return);
                }
                return response;
			}
            passed = create_room(name_room);
			if(passed!=0){
                *request_return = 2;
                sprintf(explanation, "create_room = %d /create_room", passed);
                response = error_handler(buffer, explanation, 
                        *request_return, err, &passed);
                if(response == NULL){
                    *request_return = -1000;
                    return NULL;
                }
                if(passed < 0){
                    *request_return = -(*request_return);
                }
                return response;
			}
			response = malloc(strsize("HTTP/1.1 303 See Other\r\n"
					"Content-Type: text/html; charset=utf-8\r\n"
					"Location: /rooms\n\n")+1);
			if(response == NULL){
				*request_return = -1000;
				return NULL;
			}
            strcpy(response, "HTTP/1.1 303 See Other\r\n"
				"Content-Type: text/html; charset=utf-8\r\n"
					"Location: /rooms\n\n");
        }else if(strncmp( "/rooms/", HTTP_ex.URL, 7)==0){
            char *index_symbol = HTTP_ex.URL;
            index_symbol+=7;
            size_t id_room;
            size_t id_user;
            char name_room[MAX_LENGTH];
            size_t index_name_room = 0;
            while((*index_symbol!='\0')&&(*index_symbol!='/')){
                name_room[index_name_room] = *index_symbol;
                index_name_room+=1;
                index_symbol+=1;
            }
            if(*index_symbol!='\0'){
                index_symbol+=1;
            }
            name_room[index_name_room] = '\0';
            if(num_rooms==0){
                *request_return = 1;
                strcpy(explanation, "num_rooms == 0 /rooms/...");
                response = error_handler(buffer, explanation,
                        *request_return, err, &passed);
                if(response == NULL){
                    *request_return = -1000;
                    return NULL;
                }
                if(passed < 0){
                    *request_return = -(*request_return);
                }
                return response;
            }
            passed = s_room_name_rooms(name_room, &id_room);
            if(passed!=0){
                *request_return = 2;
                strcpy(explanation, "room_name not found /rooms/...");
                response = error_handler(buffer, explanation,
                        *request_return, err, &passed);
                if(response == NULL){
                    *request_return = -1000;
                    return NULL;
                }
                if(passed < 0){
                    *request_return = -(*request_return);
                }
                return response;
            }
            passed = s_user_IP_users(IP, &id_user);
            if(passed!=0){
                *request_return = 3;
                sprintf(explanation, "s_user_IP_users = %d", passed); 
                response = error_handler(buffer, explanation,
                        *request_return, err, &passed);
                if(response == NULL){
                    *request_return = -1000;
                    return NULL;
                }
                if(passed < 0){
                    *request_return = -(*request_return);
                }
                return response;
            }
            //id_user id_room
            if(*index_symbol=='\0'){
                // /rooms/&
                char s_money[30];
                char s_user_1[MAX_LENGTH];
                char *count_money;
                size_t money;
                if((insert_field(buffer, "name_select=", s_user_1)!=0)||
                    (insert_field(buffer, "money=", s_money)!=0)){
                    *request_return = 4;
                    sprintf(explanation, "/rooms/& "
                            "not found name_select or money = %d",
                            *request_return);
                    response = error_handler(buffer, explanation, 
                            *request_return, err, &passed);
                    if(response == NULL){
                        *request_return = -1000;
                        return NULL;
                    }
                    if(passed < 0){
                        *request_return = -(*request_return);
                    }
                    return response;
                }
                money = strtoull(s_money,&count_money,10); 
                if(count_money==NULL){
                    *request_return = 6;
                    sprintf(explanation, "/rooms/& "
                            "strtoull for s_money (%s) = NULL",
                            s_money);
                    response = error_handler(buffer, explanation, 
                            *request_return, err, &passed);
                    if(response == NULL){
                        *request_return = -1000;
                        return NULL;
                    }
                    if(passed < 0){
                        *request_return = -(*request_return);
                    }
                    return response;
                }
		passed = give_money(users[id_user].name,
				s_user_1,
				rooms[id_room].name_room,
				money);
		if(passed != 0){
                    *request_return = 15;
		    sprintf(explanation, "give_money = %d", passed);
                    response = error_handler(buffer, explanation, 
                            *request_return, err, &passed);
                    if(response == NULL){
                        *request_return = -1000;
                        return NULL;
                    }
                    if(passed < 0){
                        *request_return = -(*request_return);
                    }
                    return response;
		}
				response = malloc(strsize("HTTP/1.1 303 See Other\r\n"
						"Content-Type: text/html; charset=utf-8\r\n"
						"Location: /rooms/%s/confirmation\n\n",
                        rooms[id_room].name_room));
				if(response == NULL){
					*request_return = -1000;
					return NULL;
				}
			   	sprintf(response,"HTTP/1.1 303 See Other\r\n"
						"Content-Type: text/html; charset=utf-8\r\n"
						"Location: /rooms/%s/confirmation\n\n",
                        rooms[id_room].name_room);
                   //quest 5 
            }else if(strncmp("confirmation", index_symbol,12)==0){
                index_symbol+=12;
                if(*index_symbol=='\0'){
                    // /rooms/&/confirmation
                    char s_money[30];
                    char *count_money;
                    size_t money;
                    if(insert_field(buffer, "money=", s_money)!=0){
                        *request_return = 4;
                        sprintf(explanation, "/rooms/& "
                                "not found money = %d",
                                *request_return);
                        response = error_handler(buffer, explanation, 
                                *request_return, err, &passed);
                        if(response == NULL){
                            *request_return = -1000;
                            return NULL;
                        }
                        if(passed < 0){
                            *request_return = -(*request_return);
                        }
                        return response;
                    }
                    money = strtoull(s_money, &count_money,10);
                    if(count_money==NULL){
                        *request_return = 5;
                        sprintf(explanation, "/rooms/&/confirmation "
                                "strtoull for s_money (%s) = NULL",
                                s_money);
                        response = error_handler(buffer, explanation, 
                                *request_return, err, &passed);
                        if(response == NULL){
                            *request_return = -1000;
                            return NULL;
                        }
                        if(passed < 0){
                            *request_return = -(*request_return);
                        }
                        return response;
                    }
                    create_processing(users[id_user].name,
                            rooms[id_room].name_room, money);
                    response = malloc(strsize("HTTP/1.1 303 See Other\r\n"
                            "Content-Type: text/html; charset=utf-8\r\n"
                            "Location: /rooms/%s\n\n",name_room));
                    if(response == NULL){
                        *request_return = -1000;
                        return NULL;
                    }
                    sprintf(response,"HTTP/1.1 303 See Other\r\n"
                            "Content-Type: text/html; charset=utf-8\r\n"
                            "Location: /rooms/%s\n\n",name_room);
                }else if(strncmp("_bank", index_symbol, 5)==0){
                    // /rooms/&/confirmation_bank
                    size_t now=0;
                    char choice[MAX_LENGTH];
                    while(now<rooms[id_room].counter_request){
                        if(strstr(rooms[id_room].request[now].uninterviewed,
                                    users[id_user].name)!=NULL){
				//user in uninterviewed
                            passed = insert_field(buffer,
                                    rooms[id_room].request[now].recipient,
                                    choice);
                            if(passed==-1){
				    //if not choice
                                for(size_t i=now;
                                        i<rooms[id_room].counter_request-1;
                                        i++){
                                    rooms[id_room].request[i]=rooms[id_room].request[i+1];
                                }
                                rooms[id_room].counter_request-=1;
                            }else{
                                passed = decision_processing(
                                        rooms[id_room].request[now].recipient,
                                        rooms[id_room].name_room,
                                        users[id_user].name);
                                if(passed >= 0){
                                    now+=1;
                                }else if(passed<0){
                                    *request_return = 7;
                                    sprintf(explanation,
                                            "decision_processing = %d", passed);
                                    response = error_handler(buffer, explanation,
                                            *request_return, err, &passed);
                                    if(response == NULL){
                                        *request_return = -1000;
                                        return NULL;
                                    }
                                    if(passed < 0){
                                        *request_return = -(*request_return);
                                    }
                                    return response;
                                }
                            }
                        }else{
                            now+=1;
                        }
                    }
                    response = malloc(strsize("HTTP/1.1 303 See Other\r\n"
                            "Content-Type: text/html; charset=utf-8\r\n"
                            "Location: /rooms/%s\n\n",name_room));
                    if(response == NULL){
                        *request_return = -1000;
                        return NULL;
                    }
                    sprintf(response,"HTTP/1.1 303 See Other\r\n"
                            "Content-Type: text/html; charset=utf-8\r\n"
                            "Location: /rooms/%s\n\n",name_room);

                }else{
                    *request_return = 4;
                    sprintf(explanation, "no url ", passed);
                    response = error_handler(buffer, explanation,
                            *request_return, err, &passed);
                    if(response == NULL){
                        *request_return = -1000;
                        return NULL;
                    }
                    if(passed < 0){
                        *request_return = -(*request_return);
                    }
                    return response;
                }
            }else{
                *request_return = 5;
                strcpy(explanation, "no url");
                response = error_handler(buffer, explanation,
                        *request_return, err, &passed);
                if(response == NULL){
                    *request_return = -1000;
                    return NULL;
                }
                if(passed < 0){
                    *request_return = -(*request_return);
                }
                return response;
            }
		}else{
            *request_return = 1;
            strcpy(explanation, "what_url?? ?????????");
            response = error_handler(buffer, explanation, 
                    *request_return, err, &passed);
            if(response == NULL){
                *request_return = -1000;
                return NULL;
            }
            if(passed < 0){
                *request_return = -(*request_return);
            }
            return response;
		}
    }else if (strcmp("HEA\0",HTTP_ex.Method)==0){
        response = malloc(strsize("HTTP/1.1 404 OK\n"
					"Content-Type:text/html;charset=utf-8\n\n%s",
                "<html><body><h1>NOT USE METHOD HEAD</h1></body></html>"));
		if(response == NULL){
			*request_return = -1000;
			return NULL;
		}
		sprintf(response, "HTTP/1.1 404 OK\n"
					"Content-Type:text/html;charset=utf-8\n\n%s",
                "<html><body><h1>NOT USE METHOD HEAD</h1></body></html>");
		return response;
    }else if (strcmp("PUT\0",HTTP_ex.Method)==0){
        response = malloc(strsize("HTTP/1.1 404 OK\n"
					"Content-Type:text/html;charset=utf-8\n\n%s",
                "<html><body><h1>NOT USE METHOD PUTS</h1></body></html>"));
		if(response == NULL){
			*request_return = -1000;
			return NULL;
		}
		sprintf(response, "HTTP/1.1 404 OK\n"
					"Content-Type:text/html;charset=utf-8\n\n%s",
                "<html><body><h1>NOT USE METHOD PUTS</h1></body></html>");
		return response;
    }else if (strcmp("DEL\0", HTTP_ex.Method)==0){
        response = malloc(strsize("HTTP/1.1 404 OK\n"
					"Content-Type:text/html;charset=utf-8\n\n%s",
                "<html><body><h1>NOT USE METHOD DEL</h1></body></html>"));
		if(response == NULL){
			*request_return = -1000;
			return NULL;
		}
		sprintf(response, "HTTP/1.1 404 OK\n"
					"Content-Type:text/html;charset=utf-8\n\n%s",
                "<html><body><h1>NOT USE METHOD DEL</h1></body></html>");
		return response;
    }else{
        response = malloc(strsize("HTTP/1.1 404 OK\n"
					"Content-Type:text/html;charset=utf-8\n\n%s",
                "<html><body><h1>WHATS METHOD!?</h1></body></html>"));
		if(response == NULL){
			*request_return = -1000;
			return NULL;
		}
		sprintf(response, "HTTP/1.1 404 OK\n"
					"Content-Type:text/html;charset=utf-8\n\n%s",
                "<html><body><h1>WHATS METHOD!?</h1></body></html>");
		return response;
	}
    printf("\n");
	return response;
}

int main(void){

    if(stat("./rooms", &st) == -1){
        mkdir("./rooms", 0777);
    }
    html_way_create();
	setlocale(LC_ALL, "");

	int serverSocket, clientSocket;
	struct sockaddr_in serverAddr, clientAddr;
	socklen_t addrLen;
    addrLen = sizeof(struct sockaddr_in);
	char recvBuffer[MAX_BUFFER_SIZE];
    char *response;
	int bytesReceived, resultHandle = 0;
	char ipAddressStr[INET_ADDRSTRLEN];
	char logFilePath[MAX_PATH_LEN];
	getcwd(logFilePath, MAX_LENGTH_PWD);
    sprintf(logFilePath,"%s/%s",now_pwd_dirrectory,"buffer.txt");
    Error error;
    int passed=0;
    size_t passedUns=0;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket==-1){
		perror("Socket creation failed");
        goto CLEANUP;
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(PORT);

	if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr))<0){
		perror("Bind failed");
        goto CLEANUP;
	}

	if (listen(serverSocket, 3)<0){
		perror("Listen failed");
        goto CLEANUP;
	}

	printf("Server running on port %d...\n",PORT);
    insert_account("admin", "admin", "Dimontys","192.168.3.100");
	while(resultHandle!=-1000){
		clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
		if (clientSocket<0){
			perror("Accept failed");
            goto CLEANUP;
		}
		passedUns=read(clientSocket, recvBuffer, MAX_BUFFER_SIZE);
        if(passedUns==0){
            close(clientSocket);
            continue;
        }
        recvBuffer[passedUns]='\0';
        strcpy(ipAddressStr ,inet_ntoa(clientAddr.sin_addr));
        resultHandle = 0;
        response = handle_request(recvBuffer, ipAddressStr, &resultHandle, &error);
        if(response == NULL){
            resultHandle = -1000;
        }else{
            FILE *html_file = fopen(logFilePath ,"a");
            if(html_file==NULL){
                resultHandle =-100;
                response = error_handler(recvBuffer, "Not open buffer.txt", 
                        resultHandle, &error, &passed);
                if(response == NULL){
                    resultHandle = -1000;
                    return resultHandle;
                }
                if(passed < 0){
                    resultHandle = -(resultHandle);
                }
            }else{
	            fprintf(html_file,"\n\nAnswer = %d :\n%s",resultHandle, 
	                    response);
	            fclose(html_file);
	    }
	                                //cut this
	                                print_users();
	                                print_rooms();
	                                //cut this
	            puts(" ");
	    write(clientSocket,response,strlen(response)+1);
            free(response);
        }
		close(clientSocket);
	}
	puts("ERROR");
	printf("Explanation = %s\nIn file err = %s\nResult = %d",
		error.function, error.a?"YES":"NO",error.result);
CLEANUP:
    if(serverSocket >0){
	    close(serverSocket);
    }
    if(clientSocket >0){
        close(clientSocket);
    }
    return 0;
}

