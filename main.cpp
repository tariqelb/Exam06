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
#include <netinet/in.h>

struct	client
{
	int					fd;
	struct sockaddr_in	addr;	
};

struct	server
{
	struct sockaddr_in	addr;
	int 			fd;
	fd_set			read;
	fd_set			write;
	fd_set			tmp_read;
	fd_set			tmp_write;
	int			fd_max;
	int			nbr_of_clt;
};


void	ft_max_fd(struct server *srv, struct client *clt)
{
	int i;

	i = 0;
	srv->fd_max = srv->fd;
	while (i < srv->nbr_of_clt)
	{
		if (srv->fd_max < clt->fd)
			srv->fd_max = clt->fd;
		i++;
	}
}

struct client	*ft_new_client(int *nbr_of_clt, struct client *clt)
{
	struct client *new_clt = NULL;

	if (*nbr_of_clt == 0)
	{
	/*	new_clt = (struct client *) malloc(sizeof(struct client) * 2);
		if (new_clt == NULL)
		{
			write(2, "Fatal error\n", 12);
			exit(1);
		}
		new_clt[0].fd = -1;
		new_clt[1] = NULL;
		return (new_clt);
	}
	else
	{
		new_clt = (struct client *) malloc(sizeof(strcut client) * (*nbr_of_clt + 2));
		if (new_clt == NULL)
		{
			free(clt);
			write(2, "Fatal error\n", 12);
			exit(1);
		}
		int i = 0;
		while (i < *nbr_of_clt)
		{
			new_clt[i].fd =  clt[i].fd;
			new_clt[i].clt = clt[i].addr;
			i++;
		}
		new_clt[i].fd = -1;
		new_clt[i + 1] = NULL;
		free(clt);
		clt = NULL;*/
	}
		return (new_clt);
}
//newClient.fd = accept(web.servers[i].socketFd[j], (struct sockaddr *)&newClient.addr, &newClient.len);

struct client	*ft_handle_connection(struct server *srv, struct client *clt)
{
	int 	i;
	int	clt_index;

	i = 0;
	if (FD_ISSET(srv->fd, &srv->tmp_read))
	{
		clt = ft_new_client(&srv->nbr_of_clt, clt);
		clt[srv->nbr_of_clt].fd = accept(srv->fd, (struct sockaddr *)&clt[srv->nbr_of_clt].addr, sizeof(clt[srv->nbr_of_clt].addr));
		std::cout << "A new client seccefully created" << std::endl;
		free(clt);
		exit(1);
	}
	return (clt);	
}

int	main(int ac, char **av)
{
	int				fd;
	int				port;
	int				status;
	int				backlog;
	client			*clt;
	server			srv;	

	srv.fd = -1;
	srv.fd_max = -1;
	srv.nbr_of_clt = 0;
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
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		write(2, "Fatal error\n", 12);
		exit(1);
	}
	srv.addr.sin_family = AF_INET;
	srv.addr.sin_port = htons(port);
	srv.addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	status = bind(srv.fd, srv.addr, sizeof(srv.addr));
	if (status == -1)
	{
		write(2, "Fatal error\n", 12);
		exit(1);
	}
	backlog = SOMAXCONN;
	status = listen(srv.fd, backlog);
	if (status == -1)
	{
		write(2, "Fatal error\n", 12);
		close(fd);
		exit(1);	
	}
	FD_ZERO(&srv.read);
	FD_ZERO(&srv.write);
	FD_SET(srv.fd, &srv.read);
	FD_SET(srv.fd, &srv.write);
	while (1)
	{
		FD_ZERO(&srv.tmp_read);
		FD_ZERO(&srv.tmp_write);
		srv.tmp_read = srv.read;
		srv.tmp_write = srv.write;	
		status = select(srv.fd_max, &srv.tmp_read, &srv.tmp_write, 0 , 0);
		if (status <= 0)
		{
			write(2, "Fatal error\n", 12);
			exit(1);
		}
		else
			clt = ft_handle_connection(&srv, clt);	
	}
	
	
	return (0);
}
