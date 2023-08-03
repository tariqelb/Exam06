/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tel-bouh <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/03 12:35:02 by tel-bouh          #+#    #+#             */
/*   Updated: 2023/08/03 12:57:13 by tel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
#include <arpa/inet.h>

struct	client
{
	int					fd;
	struct sockaddr_in	addr;	
	int					id;
};

struct	server
{
	struct sockaddr_in	addr;
	int 				fd;
	fd_set				read;
	fd_set				tmp_read;
	int					fd_max;
	int					nbr_of_clt;
	int					id;
};


void	ft_max_fd(struct server *srv, struct client **clt)
{
	int i;

	i = 0;
	srv->fd_max = srv->fd;
	while (i < srv->nbr_of_clt)
	{
		if (srv->fd_max < clt[i]->fd)
			srv->fd_max = clt[i]->fd;
		i++;
	}
	srv->fd_max++;
}

struct client **ft_free_clt(struct client **clt , int nbr_of_clt)
{
	int i;

	i = 0;
	if (clt == NULL)
		return (clt);
	while (i < nbr_of_clt)
	{
		if (clt[i] != NULL)
			free(clt[i]);
		i++;
	}
	free(clt);
	clt = NULL;
	return (clt);
}

struct client	**ft_new_client(int nbr_of_clt, struct client **clt)
{
	if (nbr_of_clt == 0)
	{
		clt = (struct client **) malloc(sizeof(struct client *) * 2);
		if (clt == NULL)
		{
			write(2, "Fatal error\n", 12);
			exit(1);
		}
		clt[0] = (struct client *) malloc(sizeof(struct client));
		if (clt[0] == NULL)
		{
			free(clt);
			write(2, "Fatal error\n", 12);
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
			clt = ft_free_clt(clt, nbr_of_clt);
			write(2, "Fatal error\n", 12);
			exit(1);
		}
		i = 0;
		while (i <= nbr_of_clt)
		{
			temp[i] = (struct client *) malloc(sizeof(struct client));
			if (temp[i] == NULL)
			{
				clt = ft_free_clt(clt, nbr_of_clt);
				temp = ft_free_clt(temp, i);
				write(2, "Fatal error\n", 12);
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
		clt = ft_free_clt(clt, nbr_of_clt);
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
		send(clt[i]->fd, msg, strlen(msg), 0);
		i++;
	}
}

struct client	**ft_close_connection(struct server *srv, struct client **clt, int index)
{
	int	i;
	int	j;
	char	msg[100];
	int	id;
	
	FD_CLR(clt[index]->fd, &srv->read);

	id = clt[index]->id;
	if (srv->nbr_of_clt == 1)
	{
		clt = ft_free_clt(clt, srv->nbr_of_clt);
		srv->nbr_of_clt--;
	}
	else
	{
		struct client **temp = NULL;
		temp = (struct client **) malloc(sizeof(struct client *) * srv->nbr_of_clt);
		if (temp == NULL)
		{		
			clt = ft_free_clt(clt, srv->nbr_of_clt);
			write(2, "Fatal error\n", 12);
			exit(1);
		}
		i = 0;
		j = 0;
		while (i < srv->nbr_of_clt - 1)
		{
			if (j == index)
				j++;
			temp[i] = (struct client *) malloc(sizeof(struct client));
			if (temp[i] == NULL)
			{
				clt = ft_free_clt(clt, srv->nbr_of_clt);
				clt = ft_free_clt(temp, i);
				write(2, "Fatal error\n", 12);
				exit(1);
			}
			temp[i]->fd = clt[j]->fd;
			temp[i]->addr = clt[j]->addr;
			temp[i]->id = clt[j]->id;
			i++;
			j++;
		}
		temp[i] = NULL;
		clt = ft_free_clt(clt, srv->nbr_of_clt);
		srv->nbr_of_clt--;
		clt = temp;
		sprintf(msg, "%s%d%s", "server: client ", id, " just left\n");
		i = 0;
		while (i < srv->nbr_of_clt)
		{
			send(clt[i]->fd, msg, strlen(msg), 0);
			i++;	
		}
	}
	return (clt);
}

void	ft_send_message(struct server *srv, struct client **clt, int index, char line[])
{
	int i;
	int	sd;

	i = 0;
	while (i < srv->nbr_of_clt)
	{
		if (i != index)
		{
			char	temp[2];
			sd = recv(clt[i]->fd, line, 0, 0);
			if (sd < 0)
			{
				clt = ft_close_connection(srv, clt, i);
				ft_max_fd(&srv[0], clt);	
			}
			sd = send(clt[i]->fd, line, strlen(line), 0);
		}
		i++;
	}
}

struct client	**ft_handle_connection(struct server *srv, struct client **clt)
{
	int 		i;
	int		clt_index;
	socklen_t	clt_size;

	i = 0;
	if (FD_ISSET(srv->fd, &srv->tmp_read))
	{
		clt = ft_new_client(srv->nbr_of_clt, clt);
		srv->nbr_of_clt++;
		clt_size = sizeof(clt[srv->nbr_of_clt - 1]->addr);
		clt[srv->nbr_of_clt - 1]->fd = accept(srv->fd, (struct sockaddr *)&clt[srv->nbr_of_clt - 1]->addr, &clt_size);
		if (clt[srv->nbr_of_clt - 1]->fd < 0)
		{
			clt = ft_free_clt(clt, srv->nbr_of_clt);
			write(2, "Fatal error\n", 12);
			exit(1);
		}	
		int flags = fcntl(clt[srv->nbr_of_clt - 1]->fd , F_GETFL, 0);
		fcntl(clt[srv->nbr_of_clt - 1]->fd , F_SETFL, flags | O_NONBLOCK);
		FD_SET(clt[srv->nbr_of_clt - 1]->fd, &srv->read);
		srv->id++;
		clt[srv->nbr_of_clt - 1]->id = srv->id;
		ft_send_welcome_msg(srv, clt);
		ft_max_fd(&srv[0], clt);
		return (clt);
	}
	i = 0;
	while (i < srv->nbr_of_clt)
	{
		if (FD_ISSET(clt[i]->fd, &srv->tmp_read))
		{
			int		rd;
			char	line[1000];
			memset(line, 0, 1000);
			rd = 0;
			rd = recv(clt[i]->fd, line, 9999, 0);
			line[rd] = 0;
			if (rd <= 0)
			{
				clt = ft_close_connection(&srv[0], clt, i);
				ft_max_fd(&srv[0], clt);
			}
			else
			{
				ft_send_message(srv, clt, i, line);
			}
		}
		i++;
	}
	return (clt);	
}

int	main(int ac, char **av)
{
	int				port;
	int				status;
	int				backlog;
	struct client	**clt;
	struct server	srv;	
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
	if (port < 1024 || port > 65535)
	{
		write(2, "Fatal error\n", 12);
		exit(1);
	}
	srv.fd = socket(AF_INET, SOCK_STREAM, 0);
	if (srv.fd == -1)
	{
		write(2, "Fatal error\n", 12);
		exit(1);
	}
	int on = 1;
	int flags = setsockopt(srv.fd, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(on));
	if (flags < 0)
	{
		write(2, "Fatal error\n", 12);
		exit(1);
	}
	flags = fcntl(srv.fd, F_GETFL, 0);
	status = fcntl(srv.fd, F_SETFL, flags | O_NONBLOCK);
	if (status == -1)
	{
		write(2, "Fatal error\n", 12);
		exit(1);
	}
	reuse = 1;
	status = setsockopt(srv.fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
	if (status == -1)
	{
		write(2, "Fatal error\n", 12);
		exit(1);
	}
	srv.addr.sin_family = AF_INET;
	srv.addr.sin_port = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &srv.addr.sin_addr);
	status = bind(srv.fd, (struct sockaddr *)&srv.addr, sizeof(srv.addr));
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
		close(srv.fd);
		exit(1);	
	}
	FD_ZERO(&srv.read);
	FD_SET(srv.fd, &srv.read);
	srv.fd_max = srv.fd + 1;
	while (1)
	{
		FD_ZERO(&srv.tmp_read);
		srv.tmp_read = srv.read;
		//int i = 0; if (clt != NULL) {while (clt[i]) {i++;}}
		//std::cout << "Wait in select : " << i <<  std::endl;
		status = select(srv.fd_max, &srv.tmp_read, 0, 0, 0);
		if (status <= 0)
		{
			write(2, "Fatal error\n", 12);
			exit(1);
		}
		else
		{
			clt = ft_handle_connection(&srv, clt);	
		}
	}
	return (0);
}
