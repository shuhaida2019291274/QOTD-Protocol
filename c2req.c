#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#define MAX 1000
#define PORT 1717
#define STAD struct sockaddr
void func(int sock_desc)
{
	char buff[MAX];
	int n;
	bzero(buff, sizeof(buff));
	printf("Text to server : ");
	n = 0;
	if((buff[n++] = getchar()) != '\n')
	{
		write(sock_desc, buff, sizeof(buff));
		bzero(buff, sizeof(buff));
		read(sock_desc, buff, sizeof(buff));
		printf("\nFrom server : %s\n", buff);
	}
}

int main()
{
	int sock_desc, connfd;
	struct sockaddr_in servaddr, client;

	// socket create and verification
	sock_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_desc == -1) {
		printf("Failed to create socket...\n");
		exit(0);
	}
	else
		printf("Socket successfully created…\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("192.168.56.103");
	servaddr.sin_port = htons(PORT);

	// connect the client socket to server socket
	if (connect(sock_desc, (STAD*)&servaddr, sizeof(servaddr)) != 0) {
		printf("Failed to connect with server...\n");
		exit(0);
	}
	else
		printf("Successfully connected with server...\n");

	// function for chat
	func(sock_desc);

	// close the socket
	close(sock_desc);
}
