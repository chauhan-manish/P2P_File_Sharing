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
#define FILE_BUFF 524288
int sockfd, sockfd1;
int s_port;
int listenfd, connfd;
char recvBuff[SIZE], sendBuff[SIZE];
int temp; 
pthread_t thread1,thread2, thread3;
string username = "user";
char *ip;
string filetosend, destpath, filehash;

void *clientToTracker(void *);
void *clientToClient(void *);
void *readFromTracker(void *);
void *writeToTracker(void *);

void *readFromTracker(void *arg)
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
			ofstream fout("peer_info.txt", ios::app);
			fout << username << " " << s_port << "\n";
		}
		else if( strcmp(ptr, "Logout") == 0 )
		{
			username = "user";
		}
		else if( strcmp(ptr, "Invalid") == 0 )
		{
			username = "user";
		}
		else if( strcmp(ptr, "PORT") == 0 )
		{
			ptr = strtok(NULL, delim);
			int port = atoi(ptr);
			
			ptr = strtok(NULL, delim);
			filehash = string(ptr);
			
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
			//cout << filetosend << "\n";
		}
		memset(sendBuff, '\0', SIZE);
		memset(recvBuff, '\0', SIZE);
	}
}
void *writeToTracker(void *arg)
{
	char delim[]=" ", tmpBuff[SIZE], Buff[FILE_BUFF];
	char *ptr;
	while(1)
	{
		//printf("%s: ",username);
		fgets(sendBuff, SIZE, stdin);
		strcpy(tmpBuff, sendBuff);
		ptr = strtok(tmpBuff, delim);
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
			
			//cout << destpath << "\n";
		}
		else if(strcmp(ptr, "upload_file") == 0)
		{
			string file;
			ptr = strtok(NULL, delim);
			file = string(ptr);

			SHA_CTX ctx;
			SHA1_Init(&ctx);

			int in, fd = open(file.c_str(), O_RDONLY);
			while(1)
			{
				in = read(fd, Buff, FILE_BUFF);
				if( in <= 0)
					break;
				SHA1_Update(&ctx, Buff, in);
			}
			
			unsigned char hash[SHA_DIGEST_LENGTH];
			SHA1_Final(hash, &ctx);
			string sbuff(sendBuff);
			sbuff[sbuff.size()-1] = ' ';
			
			memset(sendBuff, '\0', SIZE);
			strcpy(sendBuff, sbuff.c_str());
			//string sName(reinterpret_cast<char*>(name));
			strcat(sendBuff, reinterpret_cast<char*>(hash));
			strcat(sendBuff, "\n");
			
			//cout << sendBuff << "\n";
		}
		write(sockfd, sendBuff, SIZE);
	}
}

void *writefile(void *arg)
{
	int src = open(filetosend.c_str(), O_RDONLY);
	int in, out;
	char Buff[FILE_BUFF];
	while (1)
	{
		in = read(src, Buff, FILE_BUFF);
		if (in <= 0) 
			break;

		out = write(connfd, Buff, in);
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
	
	SHA_CTX ctx;
	SHA1_Init(&ctx);
	char Buff[FILE_BUFF];
	while (1)
	{
		in = read(sockfd1, Buff, FILE_BUFF);
		if (in <= 0) 
			break;

		SHA1_Update(&ctx, Buff, in);
		out = write(dst, Buff, in);
		if (out <= 0) 
			break;
	}
	unsigned char hash[SHA_DIGEST_LENGTH];
	SHA1_Final(hash, &ctx);
	
	if( strcmp(filehash.c_str(), reinterpret_cast<char *>(hash)))
		cout << "File Downloaded Successfully\n";
	else
		cout << "Error in Downloading File\n";

	close(dst);
}

void server()
{
	pthread_t thread1;
	struct sockaddr_in serv_addr;
	pthread_attr_t attr;
	pthread_attr_init( &attr);
	
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
		pthread_create(&thread1, &attr, writefile, (void *)parameter);
	}
	pthread_join(thread1, NULL);
	
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
    	printf("\nSocket cannot be created\n");
    } 
	
    memset(&serv_addr, '0', sizeof(serv_addr)); 
	
    serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port); 
	if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0)
    {
        printf("\nError\n");
    } 
	
    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
		printf("\nConnection Failed \n");
    }
	
	pthread_create( &thread1, &attr1, readFromTracker, (void *)p);
	pthread_create( &thread2, &attr2, writeToTracker, (void *)p);	
	
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
	pthread_attr_t attr;
	pthread_attr_init( &attr);
    int *p;
	if((sockfd1 = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\nSocket cannot be created\n");
    } 
	
    memset(&serv_addr, '0', sizeof(serv_addr)); 
	
    serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port); 
	if(inet_pton( AF_INET, ip, &serv_addr.sin_addr)<=0)
    {
        printf("\nError\n");
    } 
	
    if( connect( sockfd1, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
		printf("\nConnection Failed \n");
    }
	
	pthread_create( &thread1, &attr, readfile, (void *)p);
	
	pthread_join(thread1,NULL);
}
int main(int argc, char *argv[])
{
	srand( time(0) );
	s_port = rand()%10000 + 5000;
	if(argc < 2)
    {
        printf("Less Number of Arguments\n");
        return 1;
    }
    ip = argv[1];
	
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