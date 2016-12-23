#include <winsock2.h>
#include "stdafx.h"
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <conio.h>
#define max_threads 8
typedef struct myown
{
	char *file_name;
	long size;
	long start;
	struct sockaddr_in myaddr;

}DATA;
void addMethod(char *, int);
int http404(char *);
int getsocket()
{
	int hsock;
	int * p_int ;
	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if(hsock == -1){
		printf("Error initializing socket %d\n",WSAGetLastError());
		return -1;
	}
	
	p_int = (int*)malloc(sizeof(int));
	*p_int = 1;
	if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
		(setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
		printf("Error setting options %d\n", WSAGetLastError());
		free(p_int);
		return -1;
	}
	free(p_int);

	return hsock;
}

void socket_client()
{

	//The port and address you want to connect to
	int host_port= 1101;
	char* host_name="127.0.0.1";

	//Initialize socket support WINDOWS ONLY!
	unsigned short wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );
 	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 || ( LOBYTE( wsaData.wVersion ) != 2 ||
		    HIBYTE( wsaData.wVersion ) != 2 )) {
	    fprintf(stderr, "Could not find sock dll %d\n",WSAGetLastError());
		goto FINISH;
	}

	//Initialize sockets and set any options

	//Connect to the server
	struct sockaddr_in my_addr;

	my_addr.sin_family = AF_INET ;
	my_addr.sin_port = htons(host_port);
	
	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = inet_addr(host_name);

	//if( connect( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR ){
	//	fprintf(stderr, "Error connecting socket %d\n", WSAGetLastError());
	//	goto FINISH;
	//}

	//Now lets do the client related stuff
	char file_name[10];
	char request[1024];
	char response[1024];
	int file_name_len = 1024;
	int bytecount;
	int c,size,part_size;
	FILE *fp;
	struct sockaddr_in  temp;
	temp = my_addr;
	while(true) {
		memset(request,0,1024);
		memset(response, 0, 1024);
		addMethod(request, 2);
		int hsock = getsocket();
		my_addr = temp;
		
		
		if( connect(hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR )
		{
			fprintf(stderr, "Error connecting socket %d\n", WSAGetLastError());
			goto FINISH;
		}
		
		
		memset(file_name, '\0', file_name_len);
		printf("Enter the file name");
		scanf("%s", file_name);
		strcat(request, file_name);
		strcat(request, "\r\n");

		if( (bytecount=send(hsock, request, strlen(request),0))==SOCKET_ERROR)
		{
			fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
			goto FINISH;
		}
		
		
		printf("Sent bytes %d\n", bytecount);
		//memset(file_name, '\0', file_name_len);
		
		
		if((bytecount = recv(hsock, response, 100, 0))==SOCKET_ERROR)
		{
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			goto FINISH;
		}
		
		if (!http404(response))
		{
			fp=fopen(file_name,"w");
			fclose(fp);
			size=getlength(response);
			size%max_threads ? part_size = (size / max_threads) + 1 : (size / max_threads);
			closesocket(hsock);
			DATA *dataThread;
			dataThread = (DATA *)malloc(sizeof(DATA));
			dataThread->file_name = file_name;
			dataThread->size = part_size;
			dataThread->myaddr = my_addr;
			for (int j = 0; j < max_threads - 1; j++)
			{
				dataThread->start = j*part_size;
				
				CreateThread(0, 0, &SocketHandler, (void*)(dataThread), 0, 0);
			}
			
		}
		else
		{
			printf("asdf");
		}
		
	}

	
FINISH:
;
}
DWORD WINAPI SocketHandler(void* lp)
{
	DATA *csock = (DATA*)lp;
	int sockid = getsocket();
	char buffer;
	
	connect(sockid, (sockaddr*)(&csock->myaddr), sizeof(csock->myaddr));
	char request[1024];
	memset(request, 0, 1024);
	addMethod(request, 1);
	strcat(request, csock->file_name);
	strcat(request, "\r\n");
	addField(request, 1);
	strcat(request, tostring(csock->start));
	strcat(request, ":");
	strcat(request, tostring(csock->start + csock->size));
	send(sockid, request, 1024,0);
	
}
char *tostring(int num)
{
	int i = 0;

}
int getlength(char *response)
{
	int num = 0;
	int i = 4;
	while (response[i] != '\0')
	{
		num = num * 10 + response[i] - '0';
		i++;
	}
	return num;
}
int http404(char *response)
{
	if (!strncmp(request, "200", 3))
	{
		return 0;
	}
	return 1;

}
void addMethod(char *request, int type)
{
	int i = 0;
	if (type == 1)
	{
		strcpy(request, "GET ");
	}
	else if (type == 2)
	{
		strcpy(request, "HEAD ");

	}
	else
	{
		printf("anonymous");
		exit(0);
	}
}

void addField(char *request, int which)
{
	if (which == 99)
	{
		strcat(request, "range ");
	}
	else
	{
		printf("anonymous");
		exit(0);
	}
}