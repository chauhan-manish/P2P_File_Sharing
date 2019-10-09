#include <bits/stdc++.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <pthread.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include <time.h>

using namespace std;
#define SIZE 512

int sockfd = 0, n = 0, s_port;
int listenfd = 0, connfd;
char recvBuff[SIZE];int temp;
struct sockaddr_in serv_addr; 
char sendBuff[SIZE];
pthread_t thread1,thread2, thread3;
string username = "user";

void client(char *, int);
void *readd(void *arg)
{
	char *ptr, delim[] = " ";
	while(1)
	{
		read(sockfd,recvBuff,SIZE);
		printf("server: %s",recvBuff);
		ptr = strtok(recvBuff, delim);
		if( strcmp(ptr, "Login") != 0 )
		{
			//cout << "lol\n";
			username = "user";
		}
		else
		{
			//cout << "sg\n";
			ofstream fout("tracker_info.txt", ios::app);
			fout << username << " " << s_port << "\n";
		}
		if( strcmp(ptr, "PORT") == 0 )
		{
			ptr = strtok(sendBuff, delim);
			int port = atoi(ptr);
			char *ip = "127.0.0.1";
			client(ip,port);
		}
	}
}
void *writee(void *arg)
{
	char delim[]=" ", filepath[100];
	char *ptr;
	int in, out, src;
	while(1)
	{
		//printf("%s: ",username);
		fgets(sendBuff, 512, stdin);
		write(sockfd,sendBuff,SIZE);
			
		ptr = strtok(sendBuff, delim);
		//printf("%s\n", ptr);
		
		if(strcmp(ptr, "login") == 0 )
		{
			ptr = strtok(NULL, delim);
			username = string(ptr);
		}
		else if(strcmp(ptr, "create_group") == 0)
		{
			if( username.compare("user") == 0 )
			{
				cout<<"Login Required\n";
			}
		}
		else if(strcmp(ptr, "join_group") == 0)
		{
			if( username.compare("user") == 0 )
			{
				cout<<"Login Required\n";
			}
		}
		else if(strcmp(ptr, "upload") == 0 )
		{
			
		}
		else if(strcmp(ptr, "download") == 0)
		{
			
		}
		else if(strcmp(ptr, "list_files") == 0)
		{
			
		}
		else if(strcmp(ptr, "") == 0)
		{
			
		}
	}
}	

void *server( void *arg)
{
	pthread_t thread1;
	struct sockaddr_in serv_addr;
	//int *param = (int *)arg;
	//int port = param[0];
	pthread_attr_t custom1,custom2;
	pthread_attr_init(&custom1);
	pthread_attr_init(&custom2);
	
	int i = 0;
   	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
   	memset(&serv_addr, '0', sizeof(serv_addr));
   	memset(sendBuff, '0', sizeof(sendBuff)); 
	
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(s_port);
	
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	
    listen(listenfd, 10);
	int *parameter;
	connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
	pthread_create(&thread1, &custom1,readd,(void *)parameter);
	pthread_join(thread1,NULL);
	close(connfd);
	
}

void client(char *ip, int port)
{
	pthread_attr_t custom1,custom2;
	pthread_attr_init(&custom1);
    pthread_attr_init(&custom2);
	int *p;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
    } 
	
    memset(&serv_addr, '0', sizeof(serv_addr)); 
	
    serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port); 
	printf("Server address used is: %s\n", ip);
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
    } 
	
    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
		printf("\n Error : Connect Failed \n");
    }
	
	pthread_create(&thread1,&custom1,readd,(void *)p);
	pthread_create(&thread2,&custom2,writee,(void *)p);	
	
	pthread_join(thread1,NULL);
	pthread_join(thread2,NULL);
	
	
}
int main(int argc, char *argv[])
{
	pthread_attr_t custom3;
	pthread_attr_init(&custom3);
	srand( time(0) );
	s_port = rand()%10000 + 5000;
	int *p;
    if(argc < 2)
    {
        printf("\n Usage: %s <ip of server> \n",argv[0]);
        return 1;
    } 
	
    memset(recvBuff, '0',sizeof(recvBuff));
    int port = atoi(argv[2]);
    
	pthread_create(&thread1,&custom3,server,(void *)p);
	
	client(argv[1], port);
	pthread_join(thread3,NULL);
    
	return 0;
}