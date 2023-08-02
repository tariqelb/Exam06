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
//#include <netinet/in.h>
#include <arpa/inet.h>

struct	client
{
	int					fd;
	struct sockaddr_in	addr;	
	int			id;
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
	int			id;
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

void	ft_free_clt(struct client **clt , int nbr_of_clt)
{
	int i;

	i = 0;
	while (i < nbr_of_clt)
	{
		if (clt[i] != NULL)
			free(clt[i]);
		i++;
	}
	if (clt != NULL)
		free(clt);
}

struct client	**ft_new_client(int nbr_of_clt, struct client **clt)
{
	if (nbr_of_clt == 0)
	{
		clt = (struct client **) malloc(sizeof(struct client *) * 2);
		if (clt == NULL)
		{
			write(2, "Fatal error 7\n", 14);
			exit(1);
		}
		clt[0] = (struct client *) malloc(sizeof(struct client));
		if (clt[0] == NULL)
		{
			free(clt);
			write(2, "Fatal error 8\n", 14);
			exit(1);
		}
		clt[0]->fd = -1;
		clt[1] = NULL;
		return (clt);
	}
	else
	{
		struct client **temp;
		int		i;
		temp = (struct client **) malloc(sizeof(struct client *) * (nbr_of_clt + 2));
		if (temp == NULL)
		{
			ft_free_clt(clt, nbr_of_clt);
			write(2, "Fatal error 9\n", 14);
			exit(1);
		}
		i = 0;
		while (i <= nbr_of_clt)
		{
			temp[i] = (struct client *) malloc(sizeof(struct client));
			if (temp[i] == NULL)
			{
				ft_free_clt(clt, nbr_of_clt);
				ft_free_clt(temp, i);
				write(2, "Fatal error 10\n", 14);
				exit(1);
			}
			if (i < nbr_of_clt)
			{
				temp[i]->fd = clt[i]->fd;
				temp[i]->addr = clt[i]->addr;
				temp[i]->id = clt[i]->id;
			}
			i++;
		}
		temp[i - 1]->fd = -1;
		temp[i] = NULL;
		ft_free_clt(clt, nbr_of_clt);
		clt = temp;
		return (clt);
	}
}

void	ft_send_welcome_msg(struct server *srv, struct client **clt)
{
	int	i;
	char	msg[100];
	i = 0;
	while (i < srv->nbr_of_clt - 1)
	{
		sprintf(msg, "%s%d%s", "server: client ", clt[srv->nbr_of_clt - 1]->id , " just arrived\n");
		std::cout << "clt fd : " << clt[i]->fd << std::endl;
		send(clt[i]->fd, msg, 32, 0);
		i++;
	}
}

struct client	**ft_handle_connection(struct server *srv, struct client **clt)
{
	int 		i;
	int		clt_index;
	socklen_t	clt_size;

	i = 0;
	std::cout << "In handle connection." << std::endl;
	if (FD_ISSET(srv->fd, &srv->tmp_read))
	{
		std::cout << "newClient" << std::endl;
		clt = ft_new_client(srv->nbr_of_clt, clt);
		std::cout << "number of clients : " << srv->nbr_of_clt << std::endl;
		std::cout << "after fd  : " << clt[0]->fd << std::endl;
		srv->nbr_of_clt++;
		std::cout << "number of clients : " << srv->nbr_of_clt << std::endl;
		clt_size = sizeof(clt[srv->nbr_of_clt - 1]->addr);
		clt[srv->nbr_of_clt - 1]->fd = accept(srv->fd, (struct sockaddr *)&clt[srv->nbr_of_clt - 1]->addr, &clt_size);
		if (clt[srv->nbr_of_clt - 1]->fd < 0)
		{
			ft_free_clt(clt, srv->nbr_of_clt);
			write(2, "Fatal error 6\n", 14);
			exit(1);
		}	
		std::cout << "The new client has fd : " << clt[srv->nbr_of_clt - 1]->fd << std::endl;
		int flags = fcntl(clt[srv->nbr_of_clt - 1]->fd , F_GETFL, 0);
		fcntl(clt[srv->nbr_of_clt - 1]->fd , F_SETFL, flags | O_NONBLOCK);
		FD_SET(clt[srv->nbr_of_clt - 1]->fd, &srv->read);
		srv->id++;
		clt[srv->nbr_of_clt - 1]->id = srv->id;
		ft_send_welcome_msg(srv, clt);
		return (clt);//exit(1);
	}
	return (clt);	
}

int	main(int ac, char **av)
{
	int			port;
	int			status;
	int			backlog;
	client			**clt;
	server			srv;	
	int			reuse;

	srv.fd = -1;
	srv.fd_max = -1;
	srv.id = -1;
	srv.nbr_of_clt = 0;
	if (ac != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		exit(1);
	}
	port = atoi(av[1]);
	if (port < 1024 && port > 65535)
	{
		write(2, "Fatal error 1\n", 14);
		exit(1);
	}
	srv.fd = socket(AF_INET, SOCK_STREAM, 0);
	if (srv.fd == -1)
	{
		write(2, "Fatal error 2\n", 14);
		exit(1);
	}
	int flags = fcntl(srv.fd, F_GETFL, 0);
	status = fcntl(srv.fd, F_SETFL, flags | O_NONBLOCK);
	if (status == -1)
	{
		write(2, "Fatal error 3\n", 14);
		exit(1);
	}
	reuse = 1;
	status = setsockopt(srv.fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
	if (status == -1)
	{
		write(2, "Fatal error 3\n", 14);
		exit(1);
	}
	srv.addr.sin_family = AF_INET;
	srv.addr.sin_port = htons(port);
	//srv.addr.sin_addr.s_addr = INADDR_ANY;
	inet_pton(AF_INET, "127.0.0.1", &srv.addr.sin_addr);
	status = bind(srv.fd, (struct sockaddr *)&srv.addr, sizeof(srv.addr));
	if (status == -1)
	{
		write(2, "Fatal error 3\n", 14);
		exit(1);
	}
	backlog = SOMAXCONN;
	status = listen(srv.fd, backlog);
	if (status == -1)
	{
		write(2, "Fatal error 4\n", 14);
		close(srv.fd);
		exit(1);	
	}
	FD_ZERO(&srv.read);
	FD_ZERO(&srv.write);
	FD_SET(srv.fd, &srv.read);
	FD_SET(srv.fd, &srv.write);
	srv.fd_max = srv.fd + 1;
	while (1)
	{
		FD_ZERO(&srv.tmp_read);
		FD_ZERO(&srv.tmp_write);
		srv.tmp_read = srv.read;
		srv.tmp_write = srv.write;	
		std::cout << "Wait in select :" << std::endl;
		status = select(srv.fd_max, &srv.tmp_read, &srv.tmp_write, 0 , 0);
		if (status <= 0)
		{
			write(2, "Fatal error 5\n", 14);
			exit(1);
		}
		else
		{
			std::cout << "after in select :" << std::endl;
			clt = ft_handle_connection(&srv, clt);	
		}
	}
	
	
	return (0);
}
