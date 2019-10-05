#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <pthread.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include <sys/stat.h>

using namespace std;
#define SIZE 512
pthread_t thread1[10],thread2[10];
int listenfd = 0, connfd[10] = {0};
struct sockaddr_in serv_addr; 
char readBuff[SIZE];
char sendBuff[SIZE];
int temp;
map< string, string> user;
map< string, int > user_active;
pthread_mutex_t lck;

void initialization()
{
	ifstream fin;
	fin.open(".user", ios::in);
	string str, tmp, x;
	while(fin)
	{
		getline(fin, str);
		tmp="";
		for(int i=0; i<str.size(); i++)
		{
			if(str[i]!=' ')
				tmp += str[i];
			else
			{
				x = tmp;
				tmp="";
			}
		}
		if(str.size()>1)
			user.insert(make_pair(x,tmp));
	}
	/*
	map<string, string>::iterator it;
	for(it=user.begin();it!=user.end();it++)
		cout<<it->first<<" "<<it->second<<"\n";
	*/
}
void upload(char filepath[100], int i)
{
	int in, out, dst;
	dst = creat(filepath, 0666);
	while(1)
	{
		in = read(connfd[i], readBuff, SIZE);
		if(in <= 0)
			break;
		if(readBuff[0] == 'O' && readBuff[1] == 'K' )
			break;
		out = write(dst, readBuff, in);
		if(out <= 0) 
			break;
	}
	printf("File Uploaded Successfully\n");
}

void login(string userid, string pass, int i)
{
	if( user.find(userid) != user.end() && (pass.compare(user[userid]))==0 )
	{
		user_active.insert(make_pair(userid, connfd[i]));
		strcpy(sendBuff, "Login Successfull");
	}
	else
	{
		strcpy(sendBuff, "Invalid Login Details");
	}
	//cout<<sendBuff<<"\n";
	write(connfd[i], sendBuff, SIZE);
}
void *readd(void *parameter)
{
	int *param = (int *)parameter;
	char delim[]=" ", filepath[100];
	char *ptr;
	int dst, in, out, i = param[0];
	while(1)
	{
		read(connfd[i],readBuff, SIZE);
		printf("client: %s", readBuff);
		ptr = strtok(readBuff, delim);
		
		if(strcmp(ptr, "login") == 0)
		{
			string userid, pass;
			ptr = strtok(NULL, delim);
			userid = string(ptr);
			ptr = strtok(NULL, delim);
			pass = string(ptr);
			pass = pass.substr(0, pass.size()-1);
			//cout<<userid<<" "<<pass<<"\n";
			login(userid, pass, i);
		}
		else if(strcmp(ptr, "create_user") == 0)
		{
			string userid, pass;
			ofstream fout;
			fout.open(".user", ios::app);
			ptr = strtok(NULL, delim);
			userid = string(ptr);
			ptr = strtok(NULL, delim);
			pass = string(ptr);
			//cout<<userid<<" "<<pass<<"\n";
			fout<<userid<<" "<<pass;
			user.insert(make_pair(userid, pass));
		}
		else if(strcmp(ptr, "create_group") == 0)
		{
			string gid = ".group/";
			ptr = strtok(NULL, delim);
			gid += string(ptr);
			struct stat buffer;
			gid = gid.substr(0, gid.size()-1);
			
			//cout<<gid<<" "<<gid.size()<<"\n";
			ifstream f(gid.c_str());
    		if(f.good())
			{
				//cout<<"Already Exist"<<"\n";
				strcpy(sendBuff, "Group Already Exist");
				write(connfd[i], sendBuff, SIZE);
			}
			else
			{
				//cout<<"New Group"<<"\n";
				
			}
		}
		else if(strcmp(ptr, "upload") == 0 )
		{
			ptr = strtok(NULL, delim);
			printf("%s\n", ptr);
			sprintf(filepath, ".database/%s", ptr), 
			upload(filepath, i);
		}
		//fflush(stdin);
		memset(sendBuff, '\0', SIZE);
		memset(readBuff, '\0', SIZE);
	}
	
}
void *writee(void *parameter)
{
	int *param = (int *)parameter;
	int i = param[0];
	while(1)
	{
		fgets(sendBuff, 512, stdin);
		write(connfd[i],sendBuff,512);
		/*if(strcmpi(readBuff,"Bye")!=0)
		{
			close(connfd);
		}*/
		memset(sendBuff, '\0', SIZE);
		memset(readBuff, '\0', SIZE);
	}
	
}	
int main(int argc, char *argv[])
{
	initialization();
	if (pthread_mutex_init(&lck, NULL) != 0) 
    { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    } 
	pthread_attr_t custom1,custom2;
	pthread_attr_init(&custom1);
	pthread_attr_init(&custom2);
	
	int i = 0;
   	listenfd = socket(AF_INET, SOCK_STREAM, 0);
   	memset(&serv_addr, '0', sizeof(serv_addr));
   	memset(sendBuff, '0', sizeof(sendBuff)); 
	
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int port = atoi(argv[1]);
    serv_addr.sin_port = htons(port);
	
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
	
    listen(listenfd, 10);
	while(i<10)
	{
		connfd[i] = accept(listenfd, (struct sockaddr*)NULL, NULL); 
		int *parameter = (int *)malloc(sizeof(int));
		parameter[0] = i;
		pthread_create(&thread1[i], &custom1,readd,(void *)parameter);
		pthread_create(&thread2[i], &custom2,writee,(void *)parameter);
		i++;
	}
	i=0;
	while(i<10)
	{
		pthread_join(thread1[i],NULL);
		pthread_join(thread2[i],NULL);
		i++;
	}
	i=0;
	while(i<10)
	{
		close(connfd[i]);
		i++;
	}
	pthread_mutex_destroy(&lck);
	return 0;
}