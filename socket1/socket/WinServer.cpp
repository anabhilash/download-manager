#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

DWORD WINAPI SocketHandler(void*);
int getMethod(char *);
char *tostring(long);
char *getsize(char *);
void socket_server() 
{

	//The port you want the server to listen on
	int host_port= 1101;

	unsigned short wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD( 2, 2 );
 	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 || ( LOBYTE( wsaData.wVersion ) != 2 ||HIBYTE( wsaData.wVersion ) != 2 )) 
	{
	    fprintf(stderr, "No sock dll %d\n",WSAGetLastError());
		goto FINISH;
	}

	//Initialize sockets and set options
	int hsock;
	int * p_int ;
	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if(hsock == -1)
	{
		printf("Error initializing socket %d\n",WSAGetLastError());
		goto FINISH;
	}
	
	p_int = (int*)malloc(sizeof(int));
	*p_int = 1;
	if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
		(setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
		printf("Error setting options %d\n", WSAGetLastError());
		free(p_int);
		goto FINISH;
	}
	free(p_int);

	//Bind and listen
	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET ;
	my_addr.sin_port = htons(host_port);
	
	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = INADDR_ANY ;
	
	/* if you get error in bind 
	make sure nothing else is listening on that port */
	if( bind( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
		fprintf(stderr,"Error binding to socket %d\n",WSAGetLastError());
		goto FINISH;
	}
	if(listen( hsock, 10) == -1 ){
		fprintf(stderr, "Error listening %d\n",WSAGetLastError());
		goto FINISH;
	}
	
	//Now lets do the actual server stuff

	int* csock;
	sockaddr_in sadr;
	int	addr_size = sizeof(SOCKADDR);
	
	while(true)
	{
		printf("waiting for a connection\n");
		csock = (int*)malloc(sizeof(int));
		
		if((*csock = accept( hsock, (SOCKADDR*)&sadr, &addr_size))!= INVALID_SOCKET )
		{
			printf("asdf\n");
			CreateThread(0,0,&SocketHandler, (void*)csock , 0,0);
		}
		else
		{
			fprintf(stderr, "Error accepting %d\n",WSAGetLastError());
		}
	}

FINISH:
;
}




DWORD WINAPI SocketHandler(void* lp)
{
    int *csock = (int*)lp;
	char * num;
	FILE *fp;
	char recvbuf[1024], file_name[16];
	char sendbuf[1024];
	int recvbuf_len = 1024, recv_byte_cnt,type_of_method, fStart;
	int i = 0, j = 0, k = 0;
	memset(file_name, 0, 16);
	memset(recvbuf, 0, recvbuf_len);
	memset(sendbuf, 0, 1024);
	if((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0))==SOCKET_ERROR)
	{
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		free (csock);
		return 0;
	}
	else
	{
		type_of_method=getMethod(recvbuf);
		printf("%s", recvbuf);
		switch (type_of_method)
		{
		case 1:
			fStart = 4;
			break;
		case 2:
			fStart = 5;
			while (recvbuf[fStart] != '\r')
			{
				file_name[i] = recvbuf[fStart];
				i++;
				fStart++;
			}

			file_name[i] = '\0';
			
			num=getsize(file_name);
			if (num == NULL)
			{
				strcat(sendbuf, "404\r\n");
			}
			else
			{
				strcat(sendbuf, "200\r\n");
				strcat(sendbuf, num);
			}
			send(*csock, sendbuf, 100, 0);
			break;
		}
	}
}
char * getsize(char * file_name)
{
	FILE *fp=NULL;

	fp = fopen(file_name, "r");
	if (fp == NULL)
	{
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	return tostring(ftell(fp));
}
char *tostring(long size)
{
	int i = 0;
	char *num;
	num = (char *)malloc(sizeof(char) * 100);
	memset(num, 0, sizeof(num));
	while (size != 0)
	{
		num[i] = size % 10+'0';
		size /= 10;
		i++;
	}
	num[i] = '\0';
	strrev(num);
	return num;
}
FILE * getFile(char *file_name)
{
	FILE *fp = NULL;
	fp = fopen(file_name, "r");
	return NULL;
}
int getMethod(char *recvbuf)
{
	int mask;
	mask=strncmp(recvbuf, "GET", 3);
	if (mask == 0)
	{
		return 1;					//GET METHOD
	}
	else
	{
		mask = strncmp(recvbuf, "HEAD", 4);
		if (mask == 0)
		{
			return 2;				//POST METHOD
		}
	}
	return -1;
}