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
int sockfd, sockfd1;
int n = 0, s_port;
int listenfd = 0, connfd;
char recvBuff[SIZE];int temp;
char sendBuff[SIZE];
pthread_t thread1,thread2, thread3;
string username = "user";
char *ip = "127.0.0.1";
string filetosend, destpath;

void *clientToTracker(void *);
void *clientToClient(void *);


void *readd(void *arg)
{
	char *ptr, delim[] = " ";
	while(1)
	{
		read(sockfd,recvBuff,SIZE);
		printf("server: %s",recvBuff);
		//getchar();
		ptr = strtok(recvBuff, delim);
		if( strcmp(ptr, "Login") == 0 )
		{
			ofstream fout("tracker_info.txt", ios::app);
			fout << username << " " << s_port << "\n";
		}
		else if( strcmp(ptr, "Invalid") == 0 )
		{
			username = "user";
		}
		else if( strcmp(ptr, "PORT") == 0 )
		{
			ptr = strtok(NULL, delim);
			int port = atoi(ptr);
			
			pthread_attr_t attr;
			pthread_t thread4;
			pthread_attr_init(&attr);
			
			int *parameter = (int *)malloc(sizeof(int));
			parameter[0] = port;
				
			pthread_create(&thread4 ,&attr, clientToClient, (void *)parameter);
			pthread_join(thread4, NULL);
    
		}
		else if( strcmp(ptr, "FILE") == 0 )
		{
			ptr = strtok(NULL, delim);
			filetosend = string(ptr);
			filetosend = filetosend.substr(0, filetosend.size()-1);
			cout << filetosend << "\n";
		}
		memset(sendBuff, '\0', SIZE);
		memset(recvBuff, '\0', SIZE);
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
			ptr = strtok(NULL, delim);
			username = string(ptr);
		}
		else if(strcmp(ptr, "download_file") == 0)
		{
			string file;
			ptr = strtok(NULL, delim);
			ptr = strtok(NULL, delim);
			file = string(ptr);

			ptr = strtok(NULL, delim);
			destpath = string(ptr);
			destpath = destpath.substr(0, destpath.size()-1);
			destpath += file;
			
			cout << destpath << "\n";
		}
		else if(strcmp(ptr, "upload_file") == 0)
		{

		}
		write(sockfd, sendBuff, SIZE);
		
	}
}

void *writefile(void *arg)
{
	int src = open(filetosend.c_str(), O_RDONLY);
	int in, out;
	while (1)
	{
		in = read(src, sendBuff, SIZE);
		if (in <= 0) 
			break;
		out = write(connfd, sendBuff, in);
		if (out <= 0) 
			break;
	}
	cout << "File Uploaded Successfully\n";
	close(src);
	close(connfd);
}

void *readfile(void *arg)
{
	int dst = creat( destpath.c_str(), 0666);
	int in, out;
	
	while (1)
	{
		in = read(sockfd1, recvBuff, SIZE);
		if (in <= 0) 
			break;
		out = write(dst, recvBuff, in);
		if (out <= 0) 
			break;
	}
	cout << "File Downloaded Successfully\n";

	close(dst);
}

void server()
{
	pthread_t thread1;
	struct sockaddr_in serv_addr;
	//int *param = (int *)arg;
	//int port = param[0];
	pthread_attr_t custom1;
	pthread_attr_init(&custom1);
	
	int i = 0;
   	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
   	memset(&serv_addr, '0', sizeof(serv_addr));
   	memset(sendBuff, '0', sizeof(sendBuff)); 
	
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(s_port);
	
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	
    listen(listenfd, 10);
	while(1)
	{
		int *parameter;
		connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
		pthread_create(&thread1, &custom1,writefile,(void *)parameter);
	}
	pthread_join(thread1,NULL);
	
}

void *clientToTracker(void *arg)
{
	int *param = (int *)arg;
	int port = param[0];
	
	struct sockaddr_in serv_addr; 
	//int sockfd;
	pthread_attr_t attr1,attr2;
	pthread_attr_init( &attr1);
    pthread_attr_init( &attr2);
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
	
	pthread_create(&thread1,&attr1,readd,(void *)p);
	pthread_create(&thread2,&attr2,writee,(void *)p);	
	
	pthread_join(thread1,NULL);
	pthread_join(thread2,NULL);
	
	
}

void *clientToClient(void *arg)
{
	int *param = (int *)arg;
	int port = param[0];
	
	//cout<<port<<"\n";
	struct sockaddr_in serv_addr; 
	//int sockfd;
	pthread_attr_t custom1,custom2;
	pthread_attr_init(&custom1);
    //pthread_attr_init(&custom2);
	int *p;
	if((sockfd1 = socket(AF_INET, SOCK_STREAM, 0)) < 0)
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
	
    if( connect(sockfd1, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
		printf("\n Error : Connect Failed \n");
    }
	
	pthread_create(&thread1,&custom1,readfile,(void *)p);
	//pthread_create(&thread2,&custom2,writee,(void *)p);	
	
	pthread_join(thread1,NULL);
	//pthread_join(thread2,NULL);
	
	
}
int main(int argc, char *argv[])
{
	srand( time(0) );
	s_port = rand()%10000 + 5000;
	if(argc < 2)
    {
        printf("\n Usage: %s <ip of server> \n",argv[0]);
        return 1;
    } 
	
	pthread_attr_t attr;
	pthread_attr_init( &attr);

    int port = atoi(argv[2]);
	int *parameter = (int *)malloc(sizeof(int));
	parameter[0] = port;
		
	pthread_create( &thread1, &attr, clientToTracker, (void *)parameter);
	
	server();
	pthread_join(thread1,NULL);
    
	return 0;
}