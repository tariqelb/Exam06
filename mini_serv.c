/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tel-bouh <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/03 12:35:02 by tel-bouh          #+#    #+#             */
/*   Updated: 2023/08/06 19:14:38 by tel-bouh         ###   ########.fr       */
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
	char				buff[1000000];
	int					size;
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

struct client	**ft_close_fd(struct client **clt, int nbr)
{
	int i;

	i = 0;
	while (i < nbr)
	{
		close(clt[i]->fd);
		i++;
	}
	return (clt);
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
		clt[0]->size = 0;
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
			ft_close_fd(clt, nbr_of_clt);
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
				ft_close_fd(clt, nbr_of_clt);
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
				temp[i]->size = clt[i]->size;
				int j = 0;
				while (j < clt[i]->size)
				{
					temp[i]->buff[j] = clt[i]->buff[j];
					j++;
				}
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
	sprintf(msg, "%s%d%s", "server: client ", clt[srv->nbr_of_clt - 1]->id , " just arrived\n");
	write(1, msg, strlen(msg));
}

struct client	**ft_close_connection(struct server *srv, struct client **clt, int index)
{
	int		i;
	int		j;
	char	msg[1000000];
	char	left_msg[1000000];
	int		id;
	int		left_msg_flag;

	left_msg_flag = 0;
	i = 0;
	FD_CLR(clt[index]->fd, &srv->read);
	id = clt[index]->id;
	if (clt[index]->size)
	{
		sprintf(left_msg, "%s%d%s%s", "client: ", id, " ", clt[index]->buff);
		write(1, left_msg, strlen(left_msg));
		left_msg_flag = 1;
	}
	sprintf(msg, "%s%d%s", "server: client ", id, " just left\n");
	write(1, msg, strlen(msg));
	if (srv->nbr_of_clt == 1)
	{
		close(clt[0]->fd);
		clt = ft_free_clt(clt, srv->nbr_of_clt);
		srv->nbr_of_clt--;
	}
	else
	{
		struct client **temp = NULL;
		temp = (struct client **) malloc(sizeof(struct client *) * srv->nbr_of_clt);
		if (temp == NULL)
		{		
			clt = ft_close_fd(clt, srv->nbr_of_clt);
			clt = ft_free_clt(clt, srv->nbr_of_clt);
			write(2, "Fatal error\n", 12);
			exit(1);
		}
		i = 0;
		j = 0;
		while (j < srv->nbr_of_clt)
		{
			if (j == index)
			{
				close(clt[j]->fd);
				j++;
			}
			else
			{
				temp[i] = (struct client *) malloc(sizeof(struct client));
				if (temp[i] == NULL)
				{
					clt = ft_close_fd(clt, srv->nbr_of_clt);
					clt = ft_free_clt(clt, srv->nbr_of_clt);
					temp = ft_free_clt(temp, i);
					write(2, "Fatal error\n", 12);
					exit(1);
				}
				temp[i]->fd = clt[j]->fd;
				temp[i]->addr = clt[j]->addr;
				temp[i]->id = clt[j]->id;
				i++;
				j++;
			}
		}
		temp[i] = NULL;
		clt = ft_free_clt(clt, srv->nbr_of_clt);
		srv->nbr_of_clt--;
		clt = temp;
		sprintf(msg, "%s%d%s", "server: client ", id, " just left\n");
		i = 0;
		while (i < srv->nbr_of_clt)
		{
			if (left_msg_flag)	
				send(clt[i]->fd, left_msg, strlen(left_msg), 0);
			send(clt[i]->fd, msg, strlen(msg), 0);
			i++;	
		}
	}
	return (clt);
}

void	ft_get_line(char **temp, char **line)
{
	int i;

	i = 0;
	while (line[0][i] != 0)
	{
		if (line[0][i] == '\n')
		{
			i++;
			break;
		}
		else
		{
			i++;
		}
	}
	strcpy(temp[0], &line[0][i]);
	line[0][i] = 0;
}

struct client	**ft_send_message(struct server *srv, struct client **clt, int index, char line[])
{
	int 	i;
	int		sd;
	char	temp[1000];
	int		id;
	char	*tmp;
	char	*tmp_line;
	int		size;
	int		std1;

	std1 = 0;
	tmp = NULL;
	size = strlen(line) + 1;
	tmp = (char *) malloc(sizeof(char) * size);
	if (tmp == NULL)
	{
		clt = ft_close_fd(clt, srv->nbr_of_clt);
		clt = ft_free_clt(clt, srv->nbr_of_clt);
		write(2, "Fatal error\n", 12);
		exit(1);
	}
	tmp_line = (char *) malloc(sizeof(char) * size);
	if (tmp_line == NULL)
	{
		clt = ft_close_fd(clt, srv->nbr_of_clt);
		clt = ft_free_clt(clt, srv->nbr_of_clt);
		free(tmp);
		write(2, "Fatal error\n", 12);
		exit(1);
	}
	id = clt[index]->id;
	i = 0;
	while (i < srv->nbr_of_clt)
	{
		strcpy(tmp_line, line);
		if (i != index)
		{
			char	ch[2];
			sd = recv(clt[i]->fd, ch, 0, 0);
			if (sd < 0)
			{
				clt = ft_close_connection(srv, clt, i);
				ft_max_fd(&srv[0], clt);	
			}
			while (strlen(tmp_line))
			{
				ft_get_line(&tmp, &tmp_line);
				sprintf(temp, "%s%d%s%s", "client ", id, ": ", tmp_line);
				sd = send(clt[i]->fd, temp, strlen(temp), 0);
				if (std1 == 0)
				{
					write(1, temp, strlen(temp));
				}
				memset(temp, 0, 1000);
				memset(tmp_line, 0, size);
				strcpy(tmp_line, tmp);
				memset(tmp, 0, size);
			}
			std1 = 1;
		}
		i++;
	}
	free(tmp);
	free(tmp_line);
	return (clt);
}

struct client	**ft_handle_connection(struct server *srv, struct client **clt)
{
	int 		i;
	int			clt_index;
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
			char	line[1000000];
			memset(line, 0, 1000000);
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
				strcpy(&clt[i]->buff[clt[i]->size], line);
				clt[i]->size = strlen(clt[i]->buff);
				if (strstr(line, "\n") != NULL)
					clt = ft_send_message(srv, clt, i, line);
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
