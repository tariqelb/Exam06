#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

int maxfd, id = 0, sockfd = 0;
int clients[65000];
char buffer[200100];
fd_set set, rd, wr; 

void e(char *err)
{
	write(2, err, strlen(err));
	exit(1);
}

void	sendit(int connfd)
{
	for (int fd = 3; fd <= maxfd; ++fd)
		if (fd != connfd && FD_ISSET(fd, &wr))
			send(fd, buffer, strlen(buffer), 0);
}

int main(int ac, char **av)
{
	if (ac != 2)
		e("Wrong number of arguments\n");

	int connfd;
	socklen_t len;
	struct sockaddr_in servaddr, cli; 

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1)
		e("Fatal error\n");
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1])); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		e("Fatal error\n");

	if (listen(sockfd, 10) != 0)
		e("Fatal error\n");

	len = sizeof(cli);
	maxfd = sockfd;
	FD_ZERO(&set);
	FD_SET(sockfd, &set);

	while(1)
	{
		rd = wr = set;
		if (select(maxfd + 1, &rd, &wr, 0, 0) < 0)
			continue;
		if (FD_ISSET(sockfd, &rd))
		{
			connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
			if (connfd < 0)
				e("Fatal error\n");
			FD_SET(connfd, &set);
			sprintf(buffer, "server: client %d just arrived\n", id);
			clients[connfd] = id++;
			sendit(connfd);
			if(maxfd < connfd)
				maxfd = connfd;
			continue;
		}
		for (int fd = 3; fd <= maxfd; ++fd)
		{
			if (FD_ISSET(fd, &rd))
			{
				int r = 1;
				char message[200000];
				bzero (&message, sizeof(message));
				while(r == 1 && message[strlen(message) - 1] != '\n')
					r = recv(fd, message + strlen(message), 1, 0);
				if (r < 1)
				{
					sprintf(buffer, "server: client %d just left\n", clients[fd]);
					FD_CLR(fd, &set);
					close(fd);
				}
				else
					sprintf(buffer, "client %d: %s", clients[fd], message);
				sendit(fd);
			}
		}
	}
}
