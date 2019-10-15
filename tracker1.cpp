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
#define FILE_BUFF 524288
pthread_t thread1[10];
int listenfd = 0, connfd[10] = {0};
struct sockaddr_in serv_addr; 
char readBuff[SIZE];
char sendBuff[SIZE];
int temp;
map< string, string> user;
map< string, int > user_active;
map< int, string > user_active_inverse;

void initialization()
{
	ifstream fin;
	fin.open(".user", ios::in);
	string str, username, pass;
	char delim[] = " ", *ptr;
	while(fin)
	{
		getline(fin, str);
		if(str.size() > 0)
		{
			char arr[100];
			strcpy( arr, str.c_str());
			
			ptr = strtok( arr, delim);
			username = string(ptr);

			ptr = strtok( NULL, delim);
			pass = string(ptr);
			//cout<< username << " " << pass << "\n";
			user.insert( make_pair(username, pass));
		}
	}
	remove( "peer_info.txt");
	/*
	map<string, string>::iterator it;
	for(it=user.begin();it!=user.end();it++)
		cout<<it->first<<" "<<it->second<<"\n";
	*/
}

bool check_owner(string gid)
{
	string str;
	ifstream fin(gid.c_str());
	getline(fin, str);
	if(user_active.find(str) != user_active.end())
		return true;
	return false;
}

bool check_group(string username, string gid)
{
	string str, grp = ".group/" + gid;
	ifstream fin(grp.c_str());
	while(fin)
	{
		getline(fin, str);
		if(str.compare(username) == 0)
		{
			return true;
		}
	}
	return false;
}

void login(string userid, string pass, int i)
{
	//cout << userid << " " << pass << "\n";
	if( user.find(userid) != user.end() && (pass.compare(user[userid]))==0 )
	{
		//username = userid;
		user_active.insert(make_pair(userid, connfd[i]));
		user_active_inverse.insert(make_pair(connfd[i], userid));
		
		strcpy(sendBuff, "Login Successfull\n");
	}
	else
	{
		strcpy(sendBuff, "Invalid Login Details\n");
	}
	//cout<<sendBuff<<"\n";
	write(connfd[i], sendBuff, SIZE);
}

void create_group(string gid, int i)
{
	string grppath = ".group/" + gid;
	ifstream f(grppath.c_str());
	if(f.good())
	{
		//cout<<"Already Exist"<<"\n";
		strcpy(sendBuff, "Group Already Exist\n");
		write(connfd[i], sendBuff, SIZE);
	}
	else
	{
		//cout<<"New Group"<<"\n";
		ofstream fout, fout1;
		fout1.open(".group/group_info", ios::app);
		fout1 << gid << " " << user_active_inverse[connfd[i]] << "\n";

		fout.open(grppath.c_str());
		fout << user_active_inverse[connfd[i]] << "\n";
		strcpy(sendBuff, "Group Created\n");
		write(connfd[i], sendBuff, SIZE);
		fout.close();
		fout1.close();
	}
}

void join_group( string gid, int i)
{
	string grp = ".group/" + gid;
	ifstream f(grp.c_str());
	if(f.good())
	{
		//cout<<"Group Exist"<<"\n";
		if(check_owner(grp))
		{
			ofstream fout;
			fout.open(grp.c_str(), ios::app);
			fout << user_active_inverse[connfd[i]] << "\n";
			strcpy(sendBuff, "Group Joined\n");
			write(connfd[i], sendBuff, SIZE);
			fout.close();
		}
		else
		{

			ofstream fout(".group/pending_request", ios::app);
			string groupowner, user = user_active_inverse[connfd[i]];
			ifstream fin(grp.c_str(), ios::in);
			getline(fin, groupowner);
			fout << user << " " << gid << " " << groupowner << "\n";

			strcpy(sendBuff, "Owner Inactive\n");
			write(connfd[i], sendBuff, SIZE);	
		}
	}
	else
	{
		//cout<<"No Group"<<"\n";
		strcpy(sendBuff, "No Group Exist\n");
		write(connfd[i], sendBuff, SIZE);
		
	}
}


void leave_group( string gid, int i)
{
	ifstream f(gid.c_str());
	if(f.good())
	{
		//cout<<"Group Exist"<<"\n";
		string username = user_active_inverse[connfd[i]];
		string str, tmp = ".group/tmp";
		ofstream fout(tmp.c_str(),ios::app);
		
		while(f)
		{
			getline(f, str);
			if( str.compare(username) != 0)
				fout << str << "\n";
		}

		fout.close();
		remove(gid.c_str());
		rename(tmp.c_str(), gid.c_str());

		strcpy(sendBuff, "Group Leaved\n");
		write(connfd[i], sendBuff, SIZE);

	}
	else
	{
		//cout<<"No Group"<<"\n";
		strcpy(sendBuff, "No Group Exist\n");
		write(connfd[i], sendBuff, SIZE);
		
	}
}

void list_requests(string gid, string username, int i)
{
	string grp = ".group/" + gid;
	ifstream f(grp.c_str());
	string grpowner;
	if(f.good())
	{
		getline(f, grpowner);
		if( !(grpowner == username))
		{
			strcpy(sendBuff, "Group Access Denied\n");
		}
		else
		{
			string str, groupname, send, groupowner, requesteduser;
			char *ptr, delim[] = " ";
			ifstream fin(".group/pending_request", ios::in);
			while(fin)
			{
				getline(fin, str);
				if(str.size() == 0)
					continue;
				char arr[100];
				strcpy( arr, str.c_str());
				//cout << str << " xx " << arr << "aaaaa\n";
				
				ptr = strtok( arr, delim);
				requesteduser = string(ptr);

				ptr = strtok( NULL, delim);
				groupname = string(ptr);

				ptr = strtok( NULL, delim);
				groupowner = string(ptr);
				
				//cout << requesteduser << " " << groupowner << " " << groupname << "\n";
				if(groupowner.compare(username) == 0 && groupname.compare(gid) == 0)
				{
					send = requesteduser + " " + gid + "\n";
					strcpy(sendBuff, send.c_str());
					write(connfd[i], sendBuff, SIZE);
				}
			}
			fin.close();
			
		}
	}
	else
	{
		strcpy(sendBuff, "No Group Exist\n");
		write(connfd[i], sendBuff, SIZE);
	}
}

void accept_request(string gid, string username, string userjoining, int i)
{

}

void upload_file(string filepath, string gid, string hash, int i)
{
	string grp = ".group/" + gid;
	string username = user_active_inverse[connfd[i]];
	ifstream f(filepath.c_str());
	ifstream f2(grp.c_str());
	
	if(f.good() && f2.good())
	{
		if(!check_group(username, gid))
		{
			strcpy(sendBuff, "Group Access Denied\n");
		}
		else
		{
			ofstream fout("file_info.txt", ios::app);
			fout << filepath << " " << user_active_inverse[connfd[i]] << " " << gid << " " << hash << "\n";
			fout.close();
			strcpy(sendBuff, "File Uploaded Successfully\n");
		}
	}
	else
	{
		strcpy(sendBuff, "Wrong Arguments\n");
	}
	write(connfd[i], sendBuff, SIZE);
}

void download_file( string gid, string filepath, string destpath, int i)
{
	bool f;
	string str, file, user, username, port, groupname, filehash;
	char *ptr, delim[] = " ";
	ifstream fin("file_info.txt");
	username = user_active_inverse[connfd[i]];
	if(!check_group(username, gid))
	{
		strcpy(sendBuff, "Group Access Denied\n");
	}
	else
	{	
		while(fin)
		{
			getline(fin, str);
			if(str.size() == 0)
				break;
				
			char arr[100];
			strcpy( arr, str.c_str());
			//cout << str << " xx " << arr << "aaaaa\n";
			
			ptr = strtok( arr, delim);
			file = string(ptr);

			ptr = strtok( NULL, delim);
			username = string(ptr);

			ptr = strtok( NULL, delim);
			groupname = string(ptr);

			ptr = strtok( NULL, delim);
			filehash = string(ptr);

			string filename = "";
			int i = file.size();
			i--;
			while(i>=0 && file[i]!='/')
			{
				filename = file[i] + filename;
				i--;
			}
			//cout << filename << "\n";
			
			if( filename.compare(filepath) == 0 && (user_active.find(username) != user_active.end()) && groupname.compare( gid) == 0 )
			{
				f = true;
				break;
			}
			
		}
		fin.close();
		if(f)
		{
			//cout << username << " xx " << file << " xxx " << groupname << "\n";
			fin.open("peer_info.txt", ios::in);
			while(fin)
			{
				getline(fin, str);
				if(str.size() == 0)
					break;
				char arr[100];
				strcpy( arr, str.c_str());
			
				ptr = strtok( arr, delim);
				user = string(ptr);

				ptr = strtok( NULL, delim);
				port = string(ptr);

				if( user.compare(username) == 0 )
					break;
				
			}
			fin.close();
			//cout << username << " " << port << "\n";
			str = "FILE " + file + "\n";
			strcpy(sendBuff, str.c_str());
			int cfd = user_active[username];
			write(cfd, sendBuff, SIZE);

			str = "PORT " + port + " " + filehash + "\n";
			strcpy(sendBuff, str.c_str());
			write(connfd[i], sendBuff, SIZE);
			
			
		}
		else
		{
			strcpy(sendBuff, "Wrong Arguments\n");
			write(connfd[i], sendBuff, SIZE);
		}
	}
}

void logout( int i)
{
	string username = user_active_inverse[connfd[i]];
	user_active_inverse.erase(connfd[i]);
	user_active.erase(username);
	string str, user, port;
	ifstream fin("peer_info.txt");
	ofstream fout("tmp_peer_info.txt", ios::app);
	char delim[] = " ", *ptr;
	while(fin)
	{
		getline(fin, str);
		if(str.size() == 0)
			break;
		char arr[100];
		strcpy( arr, str.c_str());
	
		ptr = strtok( arr, delim);
		user = string(ptr);

		ptr = strtok( NULL, delim);
		port = string(ptr);

		if( user.compare(username) != 0 )
			fout << user << " " << port << "\n";
		
	}

	fin.close();
	fout.close();
	remove("peer_info.txt");
	rename("tmp_peer_info.txt", "peer_info.txt");

	strcpy( sendBuff, "Logout Successfully\n");
	write( connfd[i], sendBuff, SIZE);
}

void *readFromPeer(void *parameter)
{
	int *param = (int *)parameter;
	char delim[] = " ";
	char *ptr;
	int dst, in, out, i = param[0];
	while(1)
	{
		read(connfd[i], readBuff, SIZE);
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
			fout << userid << " " << pass;
			user.insert(make_pair(userid, pass));
			strcpy(sendBuff, "User Created\n");
			write(connfd[i], sendBuff, SIZE);
		}
		else if(strcmp(ptr, "create_group") == 0)
		{
			if(user_active_inverse.find(connfd[i]) != user_active_inverse.end() )
			{
				string gid;
				ptr = strtok(NULL, delim);
				gid = string(ptr);
				gid = gid.substr(0, gid.size()-1);
				create_group(gid, i);
			}
			else
			{
				strcpy(sendBuff, "Enter Login Details\n");
				write(connfd[i], sendBuff, SIZE);
			}
		}
		else if(strcmp(ptr, "join_group") == 0)
		{
			if(user_active_inverse.find(connfd[i]) != user_active_inverse.end() )
			{
				string gid;;
				ptr = strtok(NULL, delim);
				gid = string(ptr);
				gid = gid.substr(0, gid.size()-1);
				join_group(gid, i);
			}
			else
			{
				strcpy(sendBuff, "Enter Login Details\n");
				write(connfd[i], sendBuff, SIZE);
			}
		}
		else if(strcmp(ptr, "leave_group") == 0)
		{
			if(user_active_inverse.find(connfd[i]) != user_active_inverse.end() )
			{
				string gid = ".group/";
				ptr = strtok(NULL, delim);
				gid += string(ptr);
				gid = gid.substr(0, gid.size()-1);
				leave_group(gid, i);
			}
			else
			{
				strcpy(sendBuff, "Enter Login Details\n");
				write(connfd[i], sendBuff, SIZE);
			}
		}
		else if(strcmp(ptr, "upload_file") == 0 )
		{
			if(user_active_inverse.find(connfd[i]) != user_active_inverse.end() )
			{
				ptr = strtok(NULL, delim);
				string filepath = string(ptr);
				ptr = strtok(NULL, delim);
				string gid = string(ptr);
				//gid = gid.substr(0, gid.size()-1);
				ptr = strtok(NULL, delim);
				string hash = string(ptr);
				hash = hash.substr(0, hash.size()-1);
				//cout << filepath << filepath.size() << " " << gid << "\n";
				upload_file(filepath, gid, hash, i);
			}
			else
			{
				strcpy(sendBuff, "Enter Login Details\n");
				write(connfd[i], sendBuff, SIZE);
			}
		}
		else if(strcmp(ptr, "download_file") == 0 )
		{
			if(user_active_inverse.find(connfd[i]) != user_active_inverse.end() )
			{
				string gid, filepath, destpath;
				
				ptr = strtok(NULL, delim);
				gid = string(ptr);
				
				ptr = strtok(NULL, delim);
				filepath = string(ptr);
				
				ptr = strtok(NULL, delim);
				destpath = string(ptr);
				destpath = destpath.substr(0, destpath.size()-1);
				
				//cout << gid << " " << filepath << " " << destpath << "\n";
				if(!check_group(user_active_inverse[connfd[i]], gid))
				{
					strcpy(sendBuff, "Group Access Denied\n");
					write(connfd[i], sendBuff, SIZE);
				}
				else
					download_file(gid, filepath, destpath, i);
			}
			else
			{
				strcpy(sendBuff, "Enter Login Details\n");
				write(connfd[i], sendBuff, SIZE);
			}	
		}
		else if(strcmp(ptr, "list_groups\n") == 0)
		{
			if(user_active_inverse.find(connfd[i]) != user_active_inverse.end() )
			{
				ifstream fin(".group/group_info");
				while(fin)
				{
					string str;
					getline(fin, str);
					str += "\n";
					//cout << str ;
					strcpy(sendBuff, str.c_str());
					write( connfd[i], sendBuff, SIZE);
				}
			}
			else
			{
				strcpy(sendBuff, "Enter Login Details\n");
				write(connfd[i], sendBuff, SIZE);
			}
		}
		else if(strcmp(ptr, "list_files") == 0)
		{
			if(user_active_inverse.find(connfd[i]) != user_active_inverse.end() )
			{
				ptr = strtok(NULL, delim);
				string gid = string(ptr);
				gid = gid.substr(0, gid.size()-1);
				string username = user_active_inverse[connfd[i]];

				if(!check_group(username, gid))
				{
					strcpy(sendBuff, "Group Access Denied\n");
					write(connfd[i], sendBuff, SIZE);
				}
				else
				{
					ifstream fin("file_info.txt");
					string file, groupname, str;
					while(fin)
					{
						getline(fin, str);
						if(str.size() == 0)
							break;
						char arr[100];
						strcpy( arr, str.c_str());
						//cout << str << " xx " << arr << "aaaaa\n";
						ptr = strtok( arr, delim);
						file = string(ptr);

						ptr = strtok( NULL, delim);
						
						ptr = strtok( NULL, delim);
						groupname = string(ptr);
						
						//cout << file << " " << groupname << "\n";
						if( gid.compare(groupname) == 0)
						{
							file += "\n";
							strcpy(sendBuff, file.c_str());
							write(connfd[i], sendBuff, SIZE);
						}
					}
				}
			}
			else
			{
				strcpy(sendBuff, "Enter Login Details\n");
				write(connfd[i], sendBuff, SIZE);
			}
		}
		else if(strcmp(ptr, "list_requests") == 0)
		{ 
			if(user_active_inverse.find(connfd[i]) != user_active_inverse.end() )
			{
				string gid, username;
				username = user_active_inverse[connfd[i]];

				ptr = strtok(NULL, delim);
				gid = string(ptr);
				gid = gid.substr(0, gid.size()-1);

				list_requests(gid, username, i);
			}
			else
			{
				strcpy(sendBuff, "Enter Login Details\n");
				write(connfd[i], sendBuff, SIZE);
			}
		}
		else if(strcmp(ptr, "accept_request") == 0)
		{ 
			if(user_active_inverse.find(connfd[i]) != user_active_inverse.end() )
			{
				string gid, username, userjoining;
				username = user_active_inverse[connfd[i]];

				ptr = strtok(NULL, delim);
				gid = string(ptr);

				ptr = strtok(NULL, delim);
				userjoining = string(ptr);
				
				userjoining = userjoining.substr(0, userjoining.size()-1);

				accept_request(gid, username, userjoining, i);
			}
			else
			{
				strcpy(sendBuff, "Enter Login Details\n");
				write(connfd[i], sendBuff, SIZE);
			}
		}
		else if(strcmp(ptr, "logout\n") == 0)
		{
			if(user_active_inverse.find(connfd[i]) != user_active_inverse.end() )
			{
				logout(i);
			}
			else
			{
				strcpy(sendBuff, "Enter Login Details\n");
				write(connfd[i], sendBuff, SIZE);
			}
		}
		else
		{
			strcpy(sendBuff, "Wrong Command\n");
			write(connfd[i], sendBuff, SIZE);
		}
		memset(sendBuff, '\0', SIZE);
		memset(readBuff, '\0', SIZE);
	}
	
}

	
int main(int argc, char *argv[])
{
	initialization();
	
	pthread_attr_t attr;
	pthread_attr_init( &attr);
	
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
		pthread_create(&thread1[i], &attr,readFromPeer,(void *)parameter);
		i++;
	}
	i=0;
	while(i<10)
	{
		pthread_join(thread1[i],NULL);
		i++;
	}
	i=0;
	while(i<10)
	{
		close(connfd[i]);
		i++;
	}
	return 0;
}