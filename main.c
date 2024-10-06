#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <locale.h>
#include <math.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define strsize(args...) snprintf(NULL, 0, args) + sizeof('\0')
#define PORT 8000
#define MAX_REQUEST 8  
#define MAX_USERS 100
#define MAX_HTML_COUNTER 10
#define MAX_LENGTH 50
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
	char user2[MAX_LENGTH];
	size_t rooms;
	size_t money;
}LastOperation;

typedef struct{
	char login[MAX_LENGTH];
	char password[MAX_LENGTH];
	char name[MAX_LENGTH];
	char IP[16];
	size_t balance[MAX_ROOMS];
	LastOperation lastoperation;	
}User;

typedef struct{
	char name[MAX_LENGTH];
	char login[MAX_LENGTH];
}Name;

typedef struct{
	char IP[16];
	char login[MAX_LENGTH];
}IP;

typedef struct{
	char name_url_html[100];
}HTML_ADDR;

typedef struct{
	int users_unread;
	size_t money;
	char recipient[MAX_LENGTH];
	char uninterviewed[MAX_LENGTH*MAX_USERS_IN_ROOM-1];
}Request;

typedef struct{
	unsigned char counter_users;
	unsigned char counter_request;
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

size_t num_users = 0;
size_t num_rooms = 0;
//new
//////////////////////////////////////////////
int insert_string(char * file_string, char * search_string, char * inserting_string, char * result, size_t result_size){
	size_t len_search = strlen(search_string);
	size_t len_insert = strlen(inserting_string);
	size_t len_file   = strlen(file_string);
	size_t len_result = len_file - len_search + len_insert+1;
	if(result_size<len_result){
		return -1;
	}
	if(len_search==0){
		strcpy(result, file_string);
		return -2;
	}
	char * c_file = strstr(file_string, search_string);
	if(c_file != NULL){
		if(len_insert!=0){
			if(c_file-file_string!=0){
				if(*(c_file+strlen(search_string))!='\0'){
					if(strncpy(result, file_string, c_file-file_string)==NULL){
						printf("Error strcpy");
						return -3;
					}
					result[c_file-file_string]='\0';
					if(strcat(result, inserting_string)==NULL){
						printf("Error strcat");
						return -4;
					}
					if(strcat(result, c_file+len_search)==NULL){
						printf("Error strcat");
						return -5;
					}
					return 0;
				}else{
					if(strncpy(result, file_string, c_file-file_string)==NULL){
						printf("Error strncpy");
						return -3;
					}
					result[c_file-file_string]='\0';
					if(strcat(result, inserting_string)==NULL){
						printf("Error strcat");
						return -4;
					}
					return 0;
				}
			}else{
				if(strcpy(result, inserting_string)==NULL){
					printf("Error strcpy");
					return -4;
				}
				if(strcat(result, c_file+len_search)==NULL){
					printf("Error strcat");
					return -5;
				}
				return 0;
			}
		}else{
			if(strcpy(result, file_string)==NULL){
				printf("Error strcpy");
				return -6;
			}
			return 1;
		}
	}else{
		strcpy(result, file_string);
		return -7;
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
	// Проверяем, что файл существует и доступен для чтения
	if (access(path_file, R_OK) != 0) {
		fclose(html_file);
		return 0;
	}
	fseek(html_file, 0, SEEK_END);
	file_size = ftell(html_file);
	rewind(html_file);
	return  file_size;
}

int send_html(char *path_file, char * result){
	FILE *html_file;
	size_t file_size;
	html_file = fopen(path_file, "rb");
	if (html_file == NULL) {
		perror("fopen");
		return -1;
	}
	// Проверяем, что файл существует и доступен для чтения
	if (access(path_file, R_OK) != 0) {
		fclose(html_file);
		return -2;
	}

	fseek(html_file, 0, SEEK_END);
	file_size = ftell(html_file);
	rewind(html_file);
	if (fread(result, sizeof(char), file_size, html_file) != file_size) {
		fclose(html_file);
		return -4;
	}
	// Добавляем нуль-терминатор в конец строки
	result[file_size] = '\0';

	fclose(html_file);
	return 0;
}

int create_room(char * buffer_name){
	if ((num_rooms+1)<=MAX_ROOMS){
		if(strlen(buffer_name)<MAX_LENGTH){
			char *link_dir;
			
			if((link_dir = getcwd(NULL,0))==NULL){
				perror("getcwd error");
				return -5;
			}else{
				char link_history[4096];
				strcpy(link_history, link_dir);
				free(link_dir);
				strcat(link_history,"/rooms/");
				strcat(link_history,buffer_name);
				FILE *file = fopen(link_history, "w");
				if(file == NULL){
					perror("Error opening file");
					return 1;
				}else{
					fputs("",file);
					fclose(file);
					if(strcpy(rooms[num_rooms].name_room, buffer_name)!=NULL){
						rooms[num_rooms].counter_users=0;
						rooms[num_rooms].counter_request=0;
						strcpy(rooms[num_rooms].users_names,"");
						for(size_t i = 0;i<MAX_REQUEST;i++){
							rooms[num_rooms].request[i].money = 0;
							strcpy(rooms[num_rooms].request[i].recipient,"");
							strcpy(rooms[num_rooms].request[i].uninterviewed,"");
							rooms[num_rooms].request[i].users_unread = 0;
						}
						num_rooms+=1;
						return 0;
					}else{
						return -3;
					}
				}
			}
		}else{
			return -2;
		}
	}else{
		return -1;
	}
}

size_t binary_search(char * buffer){
	size_t low=0;
	size_t high=num_users-1;
	size_t mid=low;
	int cmp=0;
	while (low <= high){
		mid = low + (high - low) / 2;
		cmp = strcmp(users[mid].login, buffer);
		if (cmp == 0){
			
			return mid;
		}else if (cmp < 0){
			low = mid + 1;
		}else{
			if(high==0){
				return 0;
			}
			high = mid - 1;
		}
	}
	return low;
}

int help_insert_base(char *login, char *name, char *IP, size_t *t){
	//binary_search
	*t = binary_search(login);
	t++;
	size_t low = 0;
	size_t mid = low;
	size_t high = num_users - 1;
	int cmp=0;
	while(low<=high){
		mid = low +(high - low)/2;
		cmp = strcmp(names[mid].name, name);
		if(cmp == 0){
			low = high + 1;
		}else if(cmp<0){
			low = mid + 1;
		}else{
			if(high == 0){
				break;
			}
			high = mid - 1;
		}
	}
	if(cmp != 0){
		mid = low;
	}
	*t = mid;
	t++;
	low = 0;
	mid = low;
	high = num_users - 1;
	cmp = 0;
	while(low <=high){
		mid = low + (high - low)/2;
		cmp = strcmp(IPs[mid].IP, IP);
		if(cmp == 0){
			low = high + 1;
		}else if(cmp<0){
			low = mid+1;
		}else{
			if(high == 0){
				break;
			}
			high = mid - 1;
		}
	}
	if (cmp!=0){
		mid= low;
	}
	*t = mid;
	return 0;	
}

int insert_base(char *login, char *name, char *password, char *IP, size_t *p){
	//help_insert_base
	size_t k[3];
	if(num_users == 0){
		k[0]=0;
		k[1]=0;
		k[2]=0;
		strcpy(users[k[0]].login, login);
		strcpy(users[k[0]].name, name);
		strcpy(users[k[0]].password, password);
		strcpy(users[k[0]].IP, IP);


		strcpy(names[k[1]].name, name);
		strcpy(names[k[1]].login, name);

		strcpy(IPs[k[2]].login, login);
		strcpy(IPs[k[2]].IP, IP);

		for(size_t i=0;i<MAX_ROOMS;i++){
			users[k[0]].balance[i]=*p;
			if((i+1)<MAX_ROOMS){
				p+=1;
			}
		}
		num_users += 1;
	}else{
		help_insert_base(login, name, IP, k);
	
		if(strcmp( users[k[0]].login, login)!=0){
			for(size_t i= num_users; i>k[0]; i--){
				users[i] = users[i-1];
			}
			for(size_t i= num_users; i>k[1]; i--){
				names[i] = names[i-1];
			}	
			for(size_t i= num_users; i>k[2] ;i--){
				IPs[i] = IPs[i-1];
			}


			strcpy(users[k[0]].login, login);
			strcpy(users[k[0]].name, name);
			strcpy(users[k[0]].password, password);
			strcpy(users[k[0]].IP, IP);


			strcpy(names[k[1]].name, name);
			strcpy(names[k[1]].login, name);

			strcpy(IPs[k[2]].login, login);
			strcpy(IPs[k[2]].IP, IP);

			for(size_t i=0;i<MAX_ROOMS;i++){
				users[k[0]].balance[i]=*p;
				if((i+1)<MAX_ROOMS){
					p+=1;
				}
			}
			num_users += 1;
		}else{
			strcpy(users[k[0]].login, login);
			strcpy(users[k[0]].name, name);
			strcpy(users[k[0]].password, password);
			strcpy(users[k[0]].IP, IP);


			strcpy(names[k[1]].name, name);
			strcpy(names[k[1]].login, name);

			strcpy(IPs[k[2]].login, login);
			strcpy(IPs[k[2]].IP, IP);

			for(size_t i=0;i<MAX_ROOMS;i++){
				users[k[0]].balance[i]=*p;
				if((i+1)<MAX_ROOMS){
					p+=1;
				}
			}
		}
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

int IP_for_login(char * user_IP, size_t *id_login){
	//binary_search
	size_t low=0;
	size_t mid=low;
	size_t high=num_users - 1;
	int cmp=0;
	while(low<=high){
		mid = low + (high - low) / 2;
		cmp = strcmp(IPs[mid].IP, user_IP);
		if(cmp == 0){
			low = high+1;
		}else if(cmp<0){
			low = mid + 1;
		}else{
			if(high==0){
				break;
			}else{
				high = mid - 1;
			}
		}
	}
	if(cmp!=0){
		mid = low;
	}
	low = binary_search(IPs[mid].login);
	if(strcmp(users[low].IP, user_IP)!=0){
		return -1;
	}
	*id_login = low;
	return 0;
}

int name_for_login(char *name, size_t *id_login){
	//binary_search
	size_t low = 0;
	size_t mid = low;
	size_t high = num_users - 1;

	int cmp=-1;
	while(low<=high){
		mid = low + (high - low) / 2;
		cmp = strcmp(names[mid].name, name);
		if(cmp==0){
			low = high + 1;
		}else if(cmp<0){
			low = mid + 1;
		}else{
			if(high ==0){
				break;
			}else{
				high = mid - 1;
			}
		}
	}
	if(cmp!=0){
		mid = low;
	}
	high = binary_search(names[mid].login);

	if(strcmp(users[high].name, name)!=0){
		return -1;
	}
	*id_login = high;
	return 0;
}

void html_way_create(void){
	for(unsigned short int i = 0;i < MAX_HTML_COUNTER; i++){
		strcat(htmls[i].name_url_html, "/home/dmitriy/Progect/htmls/");
	}
	strcat(htmls[0].name_url_html, "login.html\0");
	strcat(htmls[1].name_url_html, "registration.html\0");
	strcat(htmls[2].name_url_html, "rooms.html\0");
	strcat(htmls[3].name_url_html, "game.html\0");
	strcat(htmls[4].name_url_html, "base.html\0");
	strcat(htmls[5].name_url_html, "create_room.html\0");
	strcat(htmls[6].name_url_html, "confirmation.html\0");
	strcat(htmls[7].name_url_html, "confirmation_bank.html\0");
	strcat(htmls[8].name_url_html, "history.html\0");
	
}

int insert_account(char * login, char * name, char * password, char * IP){
	//binary_search, name_for_login, insert_base
	size_t balance[MAX_ROOMS];
	size_t index =binary_search(login);
	for(int j=0; j<MAX_ROOMS;j++){
		balance[j]=15000000;
	}
	
	if((strcmp(users[index].login, login))!=0){
		if((name_for_login(name, &index))!=0){
			if((insert_base(login, name, password, IP, balance))==0){
				users[index].lastoperation.money = 0;
				users[index].lastoperation.rooms = 0;
				strcpy(users[index].lastoperation.user2, "Jocker");
				return 0;
			}else{
				return -4;
			}
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

int error_handler(char * buffer, char * explanation, int error, Error * err){
	FILE *html_file = fopen("/home/dmitriy/Progect/error_buffer.txt","a");
	if(html_file!=NULL){
		fprintf(html_file,"%s\nExplanation = %s\nError number = %d\n",buffer, explanation, error);
		fclose(html_file);
		err->a=true;
		strcpy(err->function,explanation);
		err->result=error;
		return 0;
	}else{
		html_file = fopen("/home/dmitriy/Progect/error_buffer1.txt","a");
		if(html_file!=NULL){
			fprintf(html_file,"%s\nExplanation = %s\nError number = %d\n",buffer, explanation, error);
			fclose(html_file);
			err->a=true;
			strcpy(err->function,explanation);
			err->result=error;
			return 1;
		}
		return -1;
	}
}
/* //Experience whith run server(rewind server analyze)
int decision_processing(char * user_request, int room, char answer){
	//?
	unsigned char counter=rooms[room].counter_request-1;
	unsigned char i=pow(2,counter-1);
	unsigned char k= counter;
	while(k>=0){
		if((answer&i)==i){
			strcat(rooms[room].request[k].uninterviewed,user_request);
			rooms[room].request[k].users_unread-=1;
			if(rooms[room].request[k].users_unread==0){
				for(unsigned char j=k;j<counter-1;j++){
					rooms[room].request[j]=rooms[room].request[j+1];
				}
				rooms[room].counter_request-=1;
			}
		}else{
			for(unsigned char j=k;j<counter-1;j++){
				rooms[room].request[j]=rooms[room].request[j+1];
			}
			rooms[room].counter_request-=1;
		}
		k-=1;
	}
	return 0;
}
*/
int create_processing(char * user_request, int room, unsigned money){
	//insert_string
	Room *r = &rooms[room];
	if(r->counter_request>=MAX_REQUEST){
		return -1;
	}
	if(strstr(r->users_names, user_request)==NULL){
		return -2;
	}
	r->counter_request+=1;
	r->request[r->counter_request-1].money = money;
	strcpy(r->request[r->counter_request-1].recipient, user_request);

	char buffer[MAX_LENGTH*MAX_USERS_IN_ROOM]="";
	for(unsigned char i=0;i<r->counter_users;i++){
		strcat(buffer, r->user[i].name); 
	}
	insert_string(buffer, user_request, "", buffer, sizeof(buffer));
	strcpy(r->request[r->counter_request-1].uninterviewed, buffer);
	r->request[r->counter_request-1].users_unread= r->counter_users-1;	
	return 0;
}

int money_transfer_bank(char *name, int room, size_t money){
	//create_processing
	if(create_processing(name, room, money)){
		return 0;
	}else{
		return -1;
	}
}

int view_processing(char * user_request,int room, char * result){
	char process[MAX_LENGTH+150];
	strcpy(result, "");
	for(int i=0;i<rooms[room].counter_request;i++){
		if(strstr(rooms[room].request[i].uninterviewed, user_request)!=NULL){
			sprintf(process,"<input type=\"checkbox\" id=\"%s\" "
					"name=\"%s\" value=\"%lu\">"
					"<label for=\"%s\"> %s %lu</label><br>",
					rooms[room].request[i].recipient,
					rooms[room].request[i].recipient,
					rooms[room].request[i].money,
				   	rooms[room].request[i].recipient,
				   	rooms[room].request[i].recipient,
				   	rooms[room].request[i].money);
			strcat(result, process);			
		}
	}
	return 0;
}
//experience
int connect_room(char *name, size_t room){
	//name_for_login
	if(room>=num_rooms){
		return -2;
	}
	size_t k;
	if(name_for_login(name, &k)!=0){
		return -3;
	}
	if(strstr(rooms[room].users_names, name)!=0){
		rooms[room].counter_users += 1;
		size_t low=0;
		size_t high=rooms[room].counter_users-1;
		int cmp=0;
		size_t mid =low;
		while(low<=high){
			mid=low+(high - low)/2;
			cmp= strcmp(rooms[room].user[mid].name, name);
			if(cmp==0){
				low = high+1;
			}else if(cmp<0){
				low = mid + 1;
			}else{
				if(high==0){
					break;
				}else{
					high = mid - 1;
				}
			}
		}
		if(cmp!=0){
			mid = low;
		}
		for(size_t i = rooms[room].counter_users; i>mid;i--){
			rooms[room].user[i] = rooms[room].user[i-1];
		}
		strcpy(rooms[room].user[mid].name, name);
		strcpy(rooms[room].user[mid].login, users[mid].login);
		strcat(rooms[room].users_names, name);
		return 0;
	}else{
		return 1;
	}	
	
	
}

char * error_send(void){
	char *response;
	response = malloc(strsize("HTTP/1.1 404 OK\n"
				"Content-Type:text/html;charset=utf-8\n%s",
				"<html><body><h1>Not found</h1></body></html>"));
	if(response == NULL){
		return NULL;
	}
	sprintf(response, "HTTP/1.1 404 OK\n"
				"Content-Type:text/html;charset=utf-8\n%s",
				"<html><body><h1>Not found</h1></body></html>");
	return response;
}

int dataProcessing(char *buffer, char *IP, Error *err, HTTP_Context *HTTP_ex){
	int passed=0;
	//size_t passeduns=0;
	if(strcmp(HTTP_ex->Method, "GET\0")==0){
		puts("Error");
		passed= -100;
		return passed;
	}else if(strcmp(HTTP_ex->Method, "POST\0") == 0){
		if(strcmp("/\0", HTTP_ex->URL)==0){
			size_t index_user;
			char buffer_login[MAX_LENGTH];
			char buffer_password[MAX_LENGTH];
			passed = insert_field(buffer, "login=", buffer_login);
			if(passed==0){
				passed = insert_field(buffer, "password=", buffer_password);
				if(passed == 0){
					index_user=binary_search(buffer_login);
					if((strcmp(users[index_user].password, buffer_password)==0)&&
							(strcmp(users[index_user].login, buffer_login)==0)){
						passed = 0;
					}else{
						return -50;// Not password or login this account
					}
				}else{
					error_handler(buffer,"insert_field error /\0", passed, err);
				}	
			}else{
				error_handler(buffer,"insert_field error /\0", passed, err);
			}

		}else if(strcmp("/registration\0",HTTP_ex->URL)==0){
			char buffer_login[MAX_LENGTH];
			char buffer_name[MAX_LENGTH];
			char buffer_password[MAX_LENGTH];
			passed = insert_field(buffer, "login=", buffer_login);
			if(passed!=0){
				error_handler(buffer,"insert_field error /registration\0", passed, err);
				return passed;
			}
			passed = insert_field(buffer, "name=", buffer_name);
			if(passed!=0){
				error_handler(buffer,"insert_field error /registration\0", passed, err);
				return passed;
			}
			passed = insert_field(buffer, "password=", buffer_password);
			if(passed!=0){
				error_handler(buffer,"insert_field error /registration\0", passed, err);
				return passed;
			}
			passed = insert_account(buffer_login, buffer_name, buffer_password, IP);
			if(passed!=0){
				error_handler(buffer, "insert_account err /registration\0", passed, err); 
			}
		}else if(strncmp("/rooms/\0", HTTP_ex->URL, 7)==0){
			char *index_symbol = HTTP_ex->URL;
			index_symbol +=7;
			char name_room[MAX_LENGTH];
			size_t index_room;
			size_t index_char_in_name = 0;
			while((*index_symbol!='\0')&&(*index_symbol!='/')){
				name_room[index_char_in_name] = *index_symbol;
				index_char_in_name++;
				index_symbol++;
			}
			if(*index_symbol!='\0'){
				index_symbol+=1;
			}
			name_room[index_char_in_name]='\0';
			if(num_rooms==0){
				passed = -25;
				error_handler(buffer, "num_rooms = 0 err /rooms/&\0", passed, err);
				return passed;
			}
			size_t low=0;
			size_t high=num_rooms-1;
			size_t mid=0;
			int cmp=0;

			while(low<=high){
				mid = low +(high-low)/2;
				cmp = strcmp(rooms[mid].name_room, name_room);
				if(cmp==0){
					low = high+1;
				}else if(cmp<0){
					low = mid+1;
				}else{
					if(high == 0){
						break;
					}else{
						high = mid - 1;
					}
				}
			}
			
			if(cmp!=0){
				passed=-125;
				return passed;
			}
			index_room = mid;
			size_t index_user;
			passed = IP_for_login(IP, &index_user);
			if(passed!=0){
				error_handler(buffer, "IP_for_login err /rooms/& ", passed, err);
				return passed;
			}
			connect_room(users[index_user].name, index_room);
//index_room, name_room,index_user 
			if(*index_symbol=='\0'){
				// /rooms/&
			}else if(strcmp(index_symbol, "/c\0")==0){
				// /rooms/&/conformation_bank
			}else{
				passed = -100;
				error_handler(buffer, "What?! /rooms/&/!?!?!\0", passed, err);
			}
		}else if(strcmp("/create_room\0",HTTP_ex->URL)==0){
			char name_room[MAX_LENGTH];
			passed = insert_field(buffer, "name=", name_room);
			if(passed!=0){
				error_handler(buffer, "insert_field(name=) err /create_room", passed, err);
				return passed;
			}
			passed = create_room(name_room);
			if(passed!=0){
				error_handler(buffer, "create_room err /create_room", passed, err);
				return passed;
			}
		}else{
			passed = -100;
			return passed;
		}
	}else{
		passed = -100;
		return passed;
	}
	return passed;
}

char * handle_request(char *buffer, char *IP, int *request_return, Error *err){
	char *response; //result
	int passed=0;
	size_t passeduns=0;	
	HTTP_Context HTTP_ex;
	create_http_struct(buffer, &HTTP_ex);
//CHECK HTTP PROTOCOL
	if (strcmp(HTTP_ex.Protocol,"HTTP/1.1")!=0){
		response = error_send();
		if(response == NULL){
			*request_return = -10;
			return NULL;
		}
        *request_return = -1;
		error_handler(buffer, "HTTP_ex.Protocol ", *request_return, err);
        return response; 
    }
//INSERT IN BUFFER.txt
	FILE *html_file = fopen("/home/dmitriy/Progect/buffer.txt","a");
	if(html_file==NULL){
		response = error_send();
		if(response == NULL){
			*request_return = -10;
			return NULL;
		}
        *request_return = -2;
        error_handler(buffer, "buffer.txt not open", *request_return, err);
        return response; 
	}
	fprintf(html_file,"%s\n",buffer);
    fclose(html_file);
//CREATE ANSWER
	char explanation[100];
	if (strcmp(HTTP_ex.Method,"GET\0")==0){
	//GET:	
		if(strcmp("/\0", HTTP_ex.URL)==0){
        // / 
            size_t buffer_size = correct_size(htmls[0].name_url_html);
            if (buffer_size == 0){
				response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -3;
				sprintf(explanation, "correct_size = %lu /", buffer_size); 
                error_handler(buffer, explanation, *request_return, err);
                return response;
			}
            char buffer_file[buffer_size];
			passed = send_html(htmls[0].name_url_html, buffer_file);	
            if(passed<0){
				response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -3;
				sprintf(explanation, "send_html = %d /", passed); 
                error_handler(buffer, explanation, *request_return, err);
                return response;
            }
            buffer_file[buffer_size] = '\0';
            response = malloc(strsize("HTTP/1.1 200 OK\nContent-Type: text/html;"
					"charset=utf-8\n\n%s",
				   buffer_file));
            sprintf(response,
				   	"HTTP/1.1 200 OK\nContent-Type: text/html;"
					"charset=utf-8\n\n%s",
				   buffer_file);
/*        
		}else if(strcmp("/base\0", HTTP_ex.URL)==0){
        // /base
            size_t buffer_size = correct_size(htmls[4].name_url_html)+1;
            if(buffer_size<0){
                sprintf(response, "HTTP/1.1 404 OK\nContent-Type:text/html;charset=utf-8\n\%s", "<html><body><h1>Иди нафиг, ты сюда не должен был заходить</h1></body></html>" );
                write(client_socket, response, strlen(response));
                close(client_socket);
                error_handler(buffer, "correct_size /base", -1, err);
                return -1;
            }
            char buffer_file[buffer_size];
            send_html(htmls[0].name_url_html, buffer_file);
            buffer_file[buffer_size-1] = '\0';
            sprintf(response, "HTTP/1.1 200 OK\nContent-Type: text/html; charset=utf-8\n\n%s", buffer_file);
		}else if(strcmp("/game\0", HTTP_ex.URL)==0){
        //GET /game HTTP/1.1
            size_t buffer_size = correct_size(htmls[2].name_url_html)+1;
            if(buffer_size<0){
                sprintf(response, "HTTP/1.1 404 OK\nContent-Type:text/html;charset=utf-8\n\%s", "<html><body><h1>Иди нафиг, ты сюда не должен был заходить</h1></body></html>" );
                write(client_socket, response, strlen(response));
                close(client_socket);
                error_handler(buffer, "correct_size /game", -1, err);
                return -1;
            }
            char buffer_file[buffer_size];
            send_html(htmls[0].name_url_html, buffer_file);
            buffer_file[buffer_size-1] = '\0';
            sprintf(response, "HTTP/1.1 200 OK\nContent-Type: text/html; charset=utf-8\n\n%s", buffer_file);
*/
		}else if(strcmp("/registration\0",HTTP_ex.URL)==0){
		// /registration
			size_t buffer_size = correct_size(htmls[1].name_url_html);
			if(buffer_size==0){
				response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -3;
				sprintf(explanation, "correct_size=%lu", buffer_size);
                error_handler(buffer, explanation,
					   	*request_return, err);
                return response;
            }
            char buffer_file[buffer_size];
			passed = send_html(htmls[1].name_url_html, buffer_file);
            if(passed<0){
				response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -3;
				sprintf(explanation, "send_html=%d", passed);
                error_handler(buffer, "send_html /registrarion",
					   	*request_return, err);
                return response;
			}
            buffer_file[buffer_size-1] = '\0';
			response=malloc(strsize("HTTP/1.1 200 OK\n"
					"Content-Type: text/html; charset=utf-8\n\n%s", buffer_file));
			sprintf(response, "HTTP/1.1 200 OK\n"
					"Content-Type: text/html; charset=utf-8\n\n%s", buffer_file);
		}else if(strcmp("/rooms\0",HTTP_ex.URL)==0){
        // /rooms
            size_t buffer_size = correct_size(htmls[2].name_url_html);
            if(buffer_size==0){
                response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -3;
				sprintf(explanation, "correct_size=%lu", buffer_size);
				error_handler(buffer, explanation, 
						*request_return, err);
                return response;
            }
            char buffer_file[buffer_size];
			passed = send_html(htmls[2].name_url_html, buffer_file);
            if(passed<0){
                response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -3;
				sprintf(explanation, "send_html=%d", passed);
				error_handler(buffer, explanation, 
						*request_return, err);
                return response;
			}
			size_t len;
			char buffer_room[(MAX_LENGTH+31)];
            char buffer_rooms[(MAX_LENGTH+31)*num_rooms];
			strcpy(buffer_rooms, "");            
            for(size_t i=0;i<num_rooms;i++){
                len  = sprintf(buffer_room,
					   	"<option value=\"%s\">%s</option>",
						rooms[i].name_room,
						rooms[i].name_room);
                buffer_room[len]='\n';
                strcat(buffer_rooms, buffer_room);
            }
			char buffer_all[strlen(buffer_rooms)+strlen(buffer_file)+1];
			passed = insert_string(buffer_file,"<-- INSERT_ROOMS -->",buffer_rooms,
				   	buffer_all, sizeof(buffer_all));
			if(passed<0){
                response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -3;
				sprintf(explanation, "insert_string=%d", passed);
				error_handler(buffer, explanation, 
						*request_return, err);
                return response;
			}
			response=malloc(strsize("HTTP/1.1 200 OK\n"
						"Content-Type: text/html; charset=utf-8\n\n%s",
						buffer_all));
            sprintf(response,
				   	"HTTP/1.1 200 OK\nContent-Type: text/html; charset=utf-8\n\n%s",
				   	buffer_all);
		}else if(strncmp("/rooms/",HTTP_ex.URL,7)==0){
        //GET /rooms/& HTTP/1.1
            char *index_symbol = HTTP_ex.URL;
            size_t num_room;
            index_symbol+=7;//index in name room
			char name_room [MAX_LENGTH];			
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
                response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -4;
				error_handler(buffer, "num_rooms == 0 /rooms/&&", 
						*request_return, err);
                return response;
			}
			size_t low=0;
			size_t high=num_rooms-1;
			size_t mid=0;
			int cmp=0;
				
			while(low<=high){
				mid = low +(high-low)/2;
				cmp = strcmp(rooms[mid].name_room,name_room);
				if(cmp == 0){
					low = high+1;
				}else if(cmp<0){
					low = mid+1;
				}else{
					if(high == 0){
						break;
					}else{
						high = mid-1;
					}
				}
			}

			if(cmp!=0){
                response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -5;
				error_handler(buffer, "room_name not found /rooms/...", 
						*request_return, err);
                return response;
			}		
			num_room = mid;
			size_t index_user;
			passed = IP_for_login(IP, &index_user);
			if(passed!=0){
                response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -3;
				sprintf(explanation, "IP_for_login = %d", passed);
				error_handler(buffer, explanation, 
						*request_return, err);
                return response;
			}
			passed = connect_room(users[index_user].name, num_room);
			if(passed!=0){
                response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -3;
				sprintf(explanation, "connect_room = %d", passed);
				error_handler(buffer, explanation, 
						*request_return, err);
                return response;
			}
			
			if(*index_symbol=='\0'){
		// /rooms/&
				size_t buffer_size= correct_size(htmls[3].name_url_html);
				if(buffer_size==0){
					response = error_send();
					if(response == NULL){
						*request_return = -10;
						return NULL;
					}
					*request_return = -3;
					error_handler(buffer, "correct_size = 0 /rooms/& ", 
							*request_return, err);
					return response;
				}
				char buffer_file[buffer_size];
				passed = send_html(htmls[3].name_url_html, buffer_file);
				if(passed!=0){
					response = error_send();
					if(response == NULL){
						*request_return = -10;
						return NULL;
					}
					*request_return = -3;
					sprintf(explanation, "send_html = %d", passed);
					error_handler(buffer, explanation, 
							*request_return, err);
					return response;
				}
				char buffer_users[(30+(2*MAX_LENGTH))*MAX_USERS_IN_ROOM];
				char buffer_user[30+(2*MAX_LENGTH)];  
				size_t k_user=0;
				strcpy(buffer_users, "");
				size_t k_users=1;
				
				for(size_t i=0;i<rooms[num_room].counter_users;i++){
					if(strcmp(rooms[num_room].user[i].name,
							   	users[index_user].name)!=0){
						k_user = sprintf(buffer_user, "option value=\"%s\">%s</option>",
								rooms[num_room].user[i].name,
							   	rooms[num_room].user[i].name);
						buffer_user[k_user]=' ';
						strcat(buffer_users, buffer_user);
						k_users+=k_user;
						buffer_users[k_users]='\n';	
					}
				}	
				char buffer_result[strlen(buffer_users)+strlen(buffer_file)+5];
				insert_string(buffer_file, "<!-- INSERT_USERS -->",
						buffer_users, buffer_result, sizeof(buffer_result));
				response = malloc(strsize("HTTP/1.1 200 OK\n"
							"Content-Type:text/html;charset=uts-8\n%s",
							buffer_result));
				if(response == NULL){
					*request_return = -1;
					return NULL;
				}
				sprintf(response, "HTTP/1.1 200 OK\n"
							"Content-Type:text/html;charset=uts-8\n%s",
							buffer_result);
			}else if(strcmp(index_symbol,"history\0")==0){
		// /rooms/&/history
				char * link_pwd=getcwd(NULL, 0);//
				if(link_pwd == NULL){
					*request_return = -11;
					return NULL;
				}				
				char link_history[strlen(link_pwd)+strlen(name_room)+10]; 
				strcpy(link_history, link_pwd);
				free(link_pwd);
				strcat(link_history,"/rooms/");
				strcat(link_history,name_room);
				size_t history_size = correct_size(link_history);
				if(history_size==0){
					response = error_send();
					if(response == NULL){
						*request_return = -10;
						return NULL;
					}
					*request_return = -3;
					error_handler(buffer, "correct_size=0 /rooms/&/history", 
							*request_return, err);
					return response;
				}
				char history_buffer[history_size];
				passed = send_html(link_history, history_buffer);
				if(passed!=0){
					response = error_send();
					if(response == NULL){
						*request_return = -10;
						return NULL;
					}
					*request_return = -3;
					sprintf(explanation, "send_html = %d", passed);
					error_handler(buffer, explanation, 
							*request_return, err);
					return response;
				}
				response=malloc(strsize("HTTP/1.1 404 OK\n"
							"Content-Type:text/html;charset=utf-8\n\%s",
							history_buffer));
				if(response == NULL){
					*request_return = -1;
					return NULL;
				}
				sprintf(response, "HTTP/1.1 404 OK\n"
						"Content-Type:text/html;charset=utf-8\n\%s",
						history_buffer);
			}else if(strcmp(index_symbol,"conformation_bank\0")==0){
		// /rooms/&/conformation_bank
				char process[(MAX_LENGTH+150)*MAX_REQUEST];
				passed = view_processing(users[index_user].name, num_room, process);
				if(passed!=0){
					response = error_send();
					if(response == NULL){
						*request_return = -10;
						return NULL;
					}
					*request_return = -3;
					sprintf(explanation, "view_processing = %d ", passed);
					error_handler(buffer, explanation, 
							*request_return, err);
					return response;
				}
				size_t buffer_size = correct_size(htmls[7].name_url_html);
				if(buffer_size==0){
					response = error_send();
					if(response == NULL){
						*request_return = -10;
						return NULL;
					}
					*request_return = -3;
					error_handler(buffer, "correct_size=0 /rooms/&/conformation_bank", 
							*request_return, err);
					return response;
				}
				char buffer_file[buffer_size];
				char buffer_result[buffer_size+strlen(process)+1+MAX_LENGTH];
				passed = send_html(htmls[7].name_url_html, buffer_file);
				if(passed!=0){
					response = error_send();
					if(response == NULL){
						*request_return = -10;
						return NULL;
					}
					*request_return = -3;
					sprintf(explanation, "send_html = %d ", passed);
					error_handler(buffer, explanation, 
							*request_return, err);
					return response;
				}
				passed = insert_string(buffer_file,"<--INPUT_REQUEST-->", process,
					   	buffer_result,
						sizeof(buffer_result));
				if(passed!=0){
					response = error_send();
					if(response == NULL){
						*request_return = -10;
						return NULL;
					}
					*request_return = -3;
					sprintf(explanation, "insert_string = %d ", passed);
					error_handler(buffer, explanation, 
							*request_return, err);
					return response;
				}
				char url[MAX_LENGTH];
				strncpy(url, HTTP_ex.Referer,strlen(HTTP_ex.Referer));
				url[strlen(HTTP_ex.Referer)]='\0';
				passed = insert_string(buffer_result,"<--INPUT_URL-->",
					   	url, buffer_result,
						sizeof(buffer_result));
				if(passed!=0){
					response = error_send();
					if(response == NULL){
						*request_return = -10;
						return NULL;
					}
					*request_return = -3;
					sprintf(explanation, "insert_string = %d ", passed);
					error_handler(buffer, explanation, 
							*request_return, err);
					return response;
				}
				response=malloc(strsize("HTTP/1.1 200 OK\n"
							"Content-Type:text/html;charset=utf-8\n%s",buffer_result));
				if(response == NULL){
					*request_return = -1;
					return NULL;
				}
				sprintf(response,"HTTP/1.1 200 OK\n"
							"Content-Type:text/html;charset=utf-8\n%s",buffer_result);
			}else if(strcmp(index_symbol,""	
			}else{
		// /rooms/&/!?!?
                response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -1;
				error_handler(buffer, "not create method /rooms/&/...", 
						*request_return, err);
                return response;
			}
       	}else if(strcmp("/create_room\0",HTTP_ex.URL)==0){
		    size_t buffer_size = correct_size(htmls[5].name_url_html);
		    if(buffer_size<0){
                response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -1;
				error_handler(buffer, "correct_size /create_room", 
						*request_return, err);
                return response;
            }
            char buffer_file[buffer_size];
            if(send_html(htmls[5].name_url_html,buffer_file)!=0){
                response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -1;
				error_handler(buffer, "send_html /create_room", 
						*request_return, err);
                return response;
            }else{
				response=malloc(strsize("HTTP/1.1 200 OK\n"
							"Content-Type:text/html;charset=utf-8\n\%s",
							buffer_file));
				if(response == NULL){
					*request_return = -1;
					return NULL;
				}
				sprintf(response, "HTTP/1.1 200 OK\n"
						"Content-Type:text/html;charset=utf-8\n\%s",
						buffer_file);
			}
		}else{
                response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -1;
				error_handler(buffer, "no url ", 
						*request_return, err);
                return response;
		}
        }
    }else if (strcmp("POST\0", HTTP_ex.Method)==0){
//POST
		if(strcmp("/\0",HTTP_ex.URL)==0){
			int index_user;
			char buffer_login[MAX_LENGTH];
			char buffer_password[MAX_LENGTH];
			if(insert_field(buffer, "login=", buffer_login)==0){
				if(insert_field(buffer, "password=", buffer_password)==0){
					index_user=binary_search(buffer_login);
					if(strcmp(users[index_user].password,buffer_password)==0){
						response = malloc(strsize("HTTP/1.1 303 See Other\n"
								"Content-Type: text/html; charset=utf-8\n"
								"Location: /rooms\n\n"));
						if(response == NULL){
							*request_return = -1;
							return NULL;
						}
						strcpy(response, "HTTP/1.1 303 See Other\n"
								"Content-Type: text/html; charset=utf-8\n"
								"Location: /rooms\n\n");
					}else{
						response = malloc(strsize("HTTP/1.1 303 See Other\n"
								"Content-Type: text/html; charset=utf-8\n"
								"Location: /\n\n"));
						if(response == NULL){
							*request_return = -1;
							return NULL;
						}
						strcpy(response, "HTTP/1.1 303 See Other\n"
								"Content-Type: text/html; charset=utf-8\n"
								"Location: /\n\n");
					}
				}else{
					response = malloc(strsize("HTTP/1.1 303 See Other\n"
							"Content-Type: text/html; charset=utf-8\n"
							"Location: /\n\n"));
					if(response == NULL){
						*request_return = -1;
						return NULL;
					}
					strcpy(response,"HTTP/1.1 303 See Other\n"
							"Content-Type: text/html; charset=utf-8\n"
							"Location: /\n\n");
				}
			}else{
				response = malloc(strsize("HTTP/1.1 303 See Other\n"
						"Content-Type: text/html; charset=utf-8\n"
						"Location: /\n\n"));
				if(response == NULL){
					*request_return = -1;
					return NULL;
				}
				strcpy(response,"HTTP/1.1 303 See Other\n"
						"Content-Type: text/html; charset=utf-8\n"
						"Location: /\n\n");
			}
		}else if(strcmp("/registration\0", HTTP_ex.URL)==0){
			puts("/registration");
			char login[MAX_LENGTH];
			char name[MAX_LENGTH];
			char password[MAX_LENGTH];
			if(((insert_field(buffer, "login=", login))!=0)||
				((insert_field(buffer, "name=", name))!=0)||
				(insert_field(buffer, "password=", password)!=0)){
				
						
				response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -1;
				error_handler(buffer, "insert_field /registration", 
						*request_return, err);
                return response;
			}
			printf("%s\n%s\n%s\n%s\n", login, name, password, IP);
			if(insert_account(login, name, password, IP)==0){
				puts("1");
				response = malloc(strsize("HTTP/1.1 303 See Other\n"
					"Content-Type: text/html; charset=utf-8\n"
					"Location: /\n\n"));
				if(response == NULL){
					*request_return =-1;
					return NULL;
				}
			   	strcpy(response,"HTTP/1.1 303 See Other\n"
					"Content-Type: text/html; charset=utf-8\n"
					"Location: /\n\n");
			}else{
				puts("4");
				response = malloc(strsize("HTTP/1.1 303 See Other\n"
						"Content-Type: text/html; charset=utf-8\n"
						"Location: /registration\n\n"));
				if(response == NULL){
					*request_return =-1;
					return NULL;
				}
				strcpy(response,"HTTP/1.1 303 See Other\n"
						"Content-Type: text/html; charset=utf-8\n"
						"Location: /registration\n\n");
			}
			puts("5");
		}else if(strcmp("/rooms\0", HTTP_ex.URL)==0){
			char name_room[MAX_LENGTH];
			if(insert_field(buffer, "room=", name_room)==0){
				response = malloc(strsize("HTTP/1.1 303 See Other\n"
						"Content-Type: text/html; charset=utf-8\n"
						"Location: /rooms/%s\n\n",name_room));
				if(response == NULL){
					*request_return = -1;
					return NULL;
				}	
			   	sprintf(response,"HTTP/1.1 303 See Other\n"
						"Content-Type: text/html; charset=utf-8\n"
						"Location: /rooms/%s\n\n",name_room);

			}else{
				response = malloc(strsize("HTTP/1.1 303 See Other\n"
						"Content-Type: text/html; charset=utf-8\n"
						"Location: /rooms\n\n"));
				if(response == NULL){
					*request_return = -1;
					return NULL;
				}
			   	strcpy(response,"HTTP/1.1 303 See Other\n"
						"Content-Type: text/html; charset=utf-8\n"
						"Location: /rooms\n\n");
			}
		}else if(strncmp("/rooms/",HTTP_ex.URL,7)==0){
            char *index_symbol = HTTP_ex.URL;
            size_t num_room;
            index_symbol+=7;//index in name room
			
			char name_room [MAX_LENGTH];			
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
                response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -1;
				error_handler(buffer, "num_rooms == 0 /rooms/...", 
						*request_return, err);
                return response;
			}
			size_t low=0;
			size_t high=num_rooms-1;
			size_t mid=0;
			int cmp=0;
				
			while(low<=high){
				mid = low +(high-low)/2;
				cmp = strcmp(rooms[mid].name_room,name_room);
				if(cmp == 0){
					low = high+1;
				}else if(cmp<0){
					low = mid+1;
				}else{
					if(high == 0){
						break;
					}else{
						high = mid-1;
					}
				}
			}

			if(cmp!=0){
                response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -1;
				error_handler(buffer, "room_name not found /rooms/...", 
						*request_return, err);
                return response;
			}		
			num_room = mid;
			size_t index_user;
			
			if(IP_for_login(IP, &index_user)!=0){
                response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -1;
				error_handler(buffer, "IP not currect /rooms/...", 
						*request_return, err);
                return response;
			}
			connect_room(users[index_user].name, num_room);
//index_user, num_room, name_room
///////////////////////////////////////////////////////////////////////////////////////	
			if(*index_symbol=='\0'){
		// /rooms/&
				char name_user[MAX_LENGTH];
				size_t index_user2;
				char money[15];
				size_t chang_balance;
				char bank[10];
				char *c;
				if(insert_head(buffer, "money", money)==0){
					chang_balance = strtoumax(money, &c, 10);	
					if(insert_head(buffer, "name_select=", name_user)==0){
						if(	users[index_user].balance[num_room]>=chang_balance){
							if(name_for_login(name_user, &index_user2)==0){
								users[index_user].balance[num_room] -=chang_balance;
								users[index_user2].balance[num_room] +=chang_balance;
								response = malloc(strsize("HTTP/1.1 303 See Other\n"
										"Content-Type: text/html; charset=utf-8\n"
										"Location: /rooms/%s\n\n",name_room));
								if(response == NULL){
									*request_return = -1;
									return NULL;
								}	
								sprintf(response,"HTTP/1.1 303 See Other\n"
										"Content-Type: text/html; charset=utf-8\n"
										"Location: /rooms/%s\n\n",name_room);
							}else{
								response=error_send();
								if(response == NULL){
									*request_return = -10;
									return NULL;
								}
								*request_return = -1;
								error_handler(buffer,
									   	"not search select name /rooms/&/",
									   	*request_return, err);
								return response;
							
							}	
						}else{
								response=error_send();
								if(response == NULL){
									*request_return = -10;
									return NULL;
								}
								*request_return = -1;
								error_handler(buffer,
									   	"balance so small /rooms/&/",
									   	*request_return, err);
								return response;
						
						}
					}if(insert_head(buffer, "bank=", bank)==0){
							if(strcmp(bank, "GIV")==0){
								money_transfer_bank(users[index_user].name,
									num_room, chang_balance);	
								response = malloc(strsize("HTTP/1.1 303 See Other\n"
										"Content-Type: text/html; charset=utf-8\n"
										"Location: /rooms/%s\n\n",name_room));
								if(response == NULL){
									*request_return = -1;
									return NULL;
								}	
								sprintf(response,"HTTP/1.1 303 See Other\n"
										"Content-Type: text/html; charset=utf-8\n"
										"Location: /rooms/%s\n\n",name_room);
							}else if(strcmp(bank, "PUSH")==0){
								if(	users[index_user].balance[num_room]>=chang_balance){
									users[index_user].balance[num_room] -=chang_balance;
									response = malloc(strsize("HTTP/1.1 303 See Other\n"
											"Content-Type: text/html; charset=utf-8\n"
											"Location: /rooms/%s\n\n",name_room));
									if(response == NULL){
										*request_return = -1;
										return NULL;
									}	
									sprintf(response,"HTTP/1.1 303 See Other\n"
											"Content-Type: text/html; charset=utf-8\n"
											"Location: /rooms/%s\n\n",name_room);
								}else{
									response = malloc(strsize("HTTP/1.1 303 See Other\n"
											"Content-Type: text/html; charset=utf-8\n"
											"Location: /rooms/%s\n\n",name_room));
									if(response == NULL){
										*request_return = -1;
										return NULL;
									}	
									sprintf(response,"HTTP/1.1 303 See Other\n"
											"Content-Type: text/html; charset=utf-8\n"
											"Location: /rooms/%s\n\n",name_room);
								}

						
						
							}else{
								response = error_send();
								if(response == NULL){
									*request_return = -10;
									return NULL;
								}
								*request_return = -1;
								error_handler(buffer, "bank !?!? /rooms/&",
											*request_return, err);
								return response;
							}
					}else{
						response = malloc(strsize("HTTP/1.1 303 See Other\n"
								"Content-Type: text/html; charset=utf-8\n"
								"Location: /rooms/%s\n\n",name_room));
						if(response == NULL){
							*request_return = -1;
							return NULL;
						}	
						sprintf(response,"HTTP/1.1 303 See Other\n"
								"Content-Type: text/html; charset=utf-8\n"
								"Location: /rooms/%s\n\n",name_room);
					}
				}else{
					response = malloc(strsize("HTTP/1.1 303 See Other\n"
							"Content-Type: text/html; charset=utf-8\n"
							"Location: /rooms/%s\n\n",name_room));
					if(response == NULL){
						*request_return = -1;
						return NULL;
					}	
					sprintf(response,"HTTP/1.1 303 See Other\n"
							"Content-Type: text/html; charset=utf-8\n"
							"Location: /rooms/%s\n\n",name_room);
				}
			}else if(strcmp(index_symbol,"conformation_bank\0")==0){
		// /rooms/&/conformation_bank
                response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -1;
				error_handler(buffer, "not_create /rooms/&/conformation_bank", 
						*request_return, err);
                return response;
			}else{
                response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				*request_return = -1;
				error_handler(buffer, "no url ", 
						*request_return, err);
                return response;
			}
		}else if(strcmp("/create_room\0",HTTP_ex.URL)==0){
			char name_room[MAX_LENGTH];
			if(insert_field(buffer, "name=",name_room)!=0){
				response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				error_handler(buffer, "insert_field /create_room",
					   	*request_return,err);
				return response;
			}
			if(create_room(name_room)!=0){
				response = error_send();
				if(response == NULL){
					*request_return = -10;
					return NULL;
				}
				error_handler(buffer, "create_room /create_room",
					   *request_return,err);
				return response;
			}
			response = malloc(strsize("HTTP/1.1 303 See Other\n"
					"Content-Type: text/html; charset=utf-8\n"
					"Location: /rooms/%s\n\n",name_room));
			if(response == NULL){
				*request_return = -1;
				return NULL;
			}	
			sprintf(response,"HTTP/1.1 303 See Other\n"
					"Content-Type: text/html; charset=utf-8\n"
					"Location: /rooms/%s\n\n",name_room);

		}else{
			response = error_send();
			if(response == NULL){
				*request_return = -10;
				return NULL;
			}
			*request_return = -1;
			error_handler(buffer, "what_url?? ?????????",
					*request_return, err);
			return response;
		}
    }else if (strcmp("HEA\0",HTTP_ex.Method)==0){
        response = malloc(strsize("HTTP/1.1 404 OK\n"
					"Content-Type:text/html;charset=utf-8\n\n%s",
                "<html><body><h1>NOT USE METHOD HEAD</h1></body></html>"));
		if(response == NULL){
			*request_return = -1;
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
			*request_return = -1;
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
			*request_return = -1;
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
			*request_return = -1;
			return NULL;
		}
		sprintf(response, "HTTP/1.1 404 OK\n"
					"Content-Type:text/html;charset=utf-8\n\n%s",
                "<html><body><h1>WHATS METHOD!?</h1></body></html>");
		return response;
	}
	*request_return = 0;
	return response;
}

int main(void){
	html_way_create();
	setlocale(LC_ALL, "");
	int server_socket, client_socket;
	struct sockaddr_in server_addr, client_addr;
	socklen_t addr_len = sizeof(client_addr);
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket==-1){
		perror("Socket creation failed");
		return 1;
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = INADDR_ANY;
	server_addr.sin_port = htons(PORT);
	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))<0){
		perror("Bind failed");
		return 1;
	}
	if (listen(server_socket, 3)<0){
		perror("Listen failed");
		return 1;
	}
	printf("Server running on port %d...\n",PORT);
	strcpy(users[0].login, "admin");
	strcpy(users[0].name, "admin");
	strcpy(users[0].password, "HvX0357\%");
	for(size_t i=0; i<MAX_ROOMS;i++){
		users[0].balance[i]=0;
	}	
	strcpy(names[0].name, "admin");
	strcpy(names[0].login, "login");
	strcpy(IPs[0].login, "login");
	strcpy(IPs[0].IP, "192.168.2.160");
	num_users+=1;
	char IP_main[10];
	char buffer[3024];
	int result_handle=0;
	char *exit_handler;
	Error err;
	while(result_handle!=-10){
		client_socket = accept(server_socket, (struct sockaddr*)&client_addr,
			   	&addr_len);
		if (client_socket<0){
			perror("Accept failed");
			return 1;
		}
		read(client_socket, buffer, sizeof(buffer));
		strcpy(IP_main ,inet_ntoa(client_addr.sin_addr));
	///////////////////////////////////////////////////////////////////////////////////
		exit_handler = handle_request(buffer, IP_main, &result_handle, &err);
		if(exit_handler == NULL){
	///////////////////////////////////////////////////////////////////////////////////
			result_handle =-10;
		}else{
	///////////////////////////////////////////////////////////////////////////////////
			write(client_socket,exit_handler,strlen(exit_handler)+1);
		}
		free(exit_handler);
		exit_handler = NULL;
		close(client_socket);
	}
	puts("ERROR");
	printf("Explanation = %s\nIn file err = %s\nResult = %d",
		err.function, err.a?"YES":"NO",err.result);	
	close(server_socket);
    return 0;
}
