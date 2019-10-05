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

using namespace std;
#define SIZE 512
int sockfd = 0, n = 0;
char recvBuff[SIZE];int temp;
struct sockaddr_in serv_addr; 
char sendBuff[SIZE];
pthread_t thread1,thread2;
string username = "client";

void upload(char filepath[100] )
{
	int in, out, src;
	src = open(filepath, O_RDONLY);
	/*
	if (sendFile(fp, net_buf, NET_BUF_SIZE))
	{ 
		sendto(sockfd, net_buf, NET_BUF_SIZE, sendrecvflag, (struct sockaddr*)&addr_con, addrlen); 
        break; 
    } 
  
    // send 
	sendto(sockfd, net_buf, NET_BUF_SIZE, sendrecvflag, (struct sockaddr*)&addr_con, addrlen); 
    clearBuf(net_buf); 
	*/
	while(1)
	{
		in = read(src, sendBuff, SIZE);
		if (in <= 0) 
			break;
		out = write(sockfd, sendBuff, in);
		if(out<=0)
			break;
	}
	printf("File Uploaded Successfully\n");
	write(sockfd, "OK", 512);

}

void download(char filepath[100] )
{
	int in, out, src;
	src = open(filepath, O_RDONLY);
	while(1)
	{
		in = read(src, sendBuff, SIZE);
		if (in <= 0) 
			break;
		out = write(sockfd, sendBuff, in);
		if(out<=0)
			break;
	}
	printf("File Uploaded Successfully\n");
	sendBuff[0] = 'O';
	sendBuff[0] = 'K';
	
	write(sockfd, sendBuff, 512);
}

void create_user(char userid[100], char password[100])
{
	write(sockfd, userid,100);
	write(sockfd, password, 100);
}


void *readd(void *arg)
{
	while(1)
	{
		read(sockfd,recvBuff,SIZE);
		printf("server: %s",recvBuff);
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
		ptr = strtok(sendBuff, delim);
		//printf("%s\n", ptr);
		
		if(strcmp(ptr, "login") == 0 )
		{
			write(sockfd,sendBuff,SIZE);
			read(sockfd,recvBuff,SIZE);
			printf("server: %s\n",recvBuff);
			if( strcmp(recvBuff, "Login Successfull") == 0 )
			{
				ptr = strtok(NULL, delim);
				username = string(ptr);
			}
		}
		else if(strcmp(ptr, "create_group") == 0)
		{
			if( username.compare("client") == 0 )
			{
				cout<<"Login Required\n";
			}
			else
			{
				write(sockfd,sendBuff,SIZE);
				read(sockfd,recvBuff,SIZE);
				printf("server: %s\n",recvBuff);
			}
		}
		else if(strcmp(ptr, "upload") == 0 )
		{
			write(sockfd,sendBuff,SIZE);
			ptr = strtok(NULL, delim);
			//printf("%s\n", ptr);
			sprintf(filepath, "%s", ptr), 
			upload(filepath);
		}
		else if(strcmp(ptr, "download") == 0)
		{
			write(sockfd,sendBuff,SIZE);
			ptr = strtok(NULL, delim);
			//printf("%s\n", ptr);
			sprintf(filepath, "%s", ptr), 
			download(filepath);
		}
		else if(strcmp(ptr, "list_files") == 0)
		{
			
		}
		else if(strcmp(ptr, "") == 0)
		{
			
		}
	}
}	
int main(int argc, char *argv[])
{
	pthread_attr_t custom1,custom2;
	pthread_attr_init(&custom1);
    pthread_attr_init(&custom2);
	
	int *p;
    if(argc < 2)
    {
        printf("\n Usage: %s <ip of server> \n",argv[0]);
        return 1;
    } 
	
    memset(recvBuff, '0',sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 
	
    memset(&serv_addr, '0', sizeof(serv_addr)); 
	
    serv_addr.sin_family = AF_INET;
	int port = atoi(argv[2]);
    serv_addr.sin_port = htons(port); 
	printf("Server address used is: %s\n", argv[1]);
    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    } 
	
    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
		printf("\n Error : Connect Failed \n");
		return 1;
    } 
	
	pthread_create(&thread1,&custom1,readd,(void *)p);
	pthread_create(&thread2,&custom2,writee,(void *)p);	
	pthread_join(thread2,NULL);
	pthread_join(thread1,NULL);
    
	return 0;
}