#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <iostream>


int	main(int ac, char **av)
{
	int					fd;
	int					port;
	struct sockaddr_in	addr;
	int					status;
	int					backlog;

	if (ac != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		exit(1);
	}
	port = atoi(av[1]);
	if (port < 1024 && port > 65535)
	{
		write(2, "Fatal error\n", 12);
		exit(1);
	}
	fd = socket(2, 1, 0);
	if (fd == -1)
	{
		write(2, "Fatal error\n", 12);
		exit(1);
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
	status = bind(fd, addr, sizeof(addr));
	if (status == -1)
	{
		write(2, "Fatal error\n", 12);
		exit(1);
	}
	backlog = SOMAXCONN;
	status = listen(fd, backlog);
	if (status == -1)
	{
		write(2, "Fatal error\n", 12);
		close(fd);
		exit(1);	
	}

	return (0);
}
