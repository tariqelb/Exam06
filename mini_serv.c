/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tel-bouh <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/03 12:35:02 by tel-bouh          #+#    #+#             */
/*   Updated: 2024/01/03 18:29:08 by tel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>

struct	client
{
	struct sockaddr_in		addr;	
	int				fd;
	int				id;
	char				buff[1000000];
	int				size;
};

struct	server
{
	struct sockaddr_in		addr;
	int 				fd;
	int				fd_max;
	fd_set				read;
	fd_set				tmp_read;
	int				nbr_of_clt;
	int				id;
};

struct client		*clt;
struct server		srv;

void	ft_error(int status)
{
	if (status < 0)
	{
		int i = 0;
		while (i < srv.nbr_of_clt)
		{
			close(clt[i].fd);
			i++;
		}
		close(srv.fd);
		if (clt != NULL)
			free(clt);
		write(2, "Fatal error\n", 12);
		exit(1);	
	}
}

void	ft_max_fd()
{
	int i;

	i = 0;
	srv.fd_max = srv.fd;
	while (i < srv.nbr_of_clt)
	{
		if (srv.fd_max < clt[i].fd)
			srv.fd_max = clt[i].fd;
		i++;
	}
	srv.fd_max += 1;
}

void	ft_get_line(char temp[][1000000], char line[][1000000])
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

void	ft_send_welcome_msg()
{
	int	i;
	char	msg[1000];

	if (srv.nbr_of_clt)	
		sprintf(msg, "%s%d%s", "server: client ", clt[srv.nbr_of_clt - 1].id , " just arrived\n");
	i = 0;
	while (i < srv.nbr_of_clt - 1)
	{
		send(clt[i].fd, msg, strlen(msg), 0);
		i++;
	}
}

void	ft_new_client()
{
	if (srv.nbr_of_clt == 0)
	{
		clt = (struct client *) malloc(sizeof(struct client));
		if (clt == NULL)
			ft_error(-1);
		clt[0].fd = -1;
		clt[0].size = 0;
		memset(clt[0].buff, 0, 1000000);
	}
	else
	{
		struct client *temp = NULL;
		int		i;
		temp = (struct client *) malloc(sizeof(struct client ) * (srv.nbr_of_clt + 1));
		if (temp == NULL)
			ft_error(-1);
		i = 0;
		while (i < srv.nbr_of_clt)
		{
			temp[i].fd = clt[i].fd;
			temp[i].addr = clt[i].addr;
			temp[i].id = clt[i].id;
			temp[i].size = clt[i].size;
			int j = 0;
			while (j < clt[i].size)
			{
				temp[i].buff[j] = clt[i].buff[j];
				j++;
			}
			i++;
		}
		temp[i].fd = -1;
		temp[i].size = 0;
		memset(temp[i].buff, 0, 1000000);
		free(clt);
		clt = temp;
	}
}

void	ft_close_connection(int index)
{
	int		i;
	int		j;
	char		msg[200];
	int		id;

	i = 0;
	FD_CLR(clt[index].fd, &srv.read);
	id = clt[index].id;
	if (srv.nbr_of_clt == 1)
	{
		close(clt[0].fd);
		free(clt);
		srv.nbr_of_clt--;
	}
	else
	{
		struct client *temp = NULL;
		temp = (struct client *) malloc(sizeof(struct client ) * (srv.nbr_of_clt - 1));
		if (temp == NULL)
			ft_error(-1);
		i = 0;
		j = 0;
		while (j < srv.nbr_of_clt)
		{
			if (j == index)
			{
				close(clt[j].fd);
				j++;
			}
			else
			{
				temp[i].addr = clt[j].addr;
				temp[i].id = clt[j].id;
				temp[i].fd = clt[j].fd;
				temp[i].size = clt[j].size;
				int k = 0;
				while (k < clt[j].size)
				{
					temp[i].buff[k] = clt[j].buff[k];
					k++;
				}
				i++;
				j++;
			}
		}
		free(clt);
		srv.nbr_of_clt--;
		clt = temp;
		sprintf(msg, "%s%d%s", "server: client ", id, " just left\n");
		i = 0;
		while (i < srv.nbr_of_clt)
		{
			send(clt[i].fd, msg, strlen(msg), 0);
			i++;	
		}
	}
}

void	ft_send_message(int index, char *line)
{
	int 		i;
	char	temp[1000000];
	char	rest[1000000];
	char	the_line[1000000];

	i = 0;
	while (i < srv.nbr_of_clt)
	{
		strcpy(the_line, line);
		if (i != index)
		{
			while (strlen(the_line))
			{
				ft_get_line(&rest, &the_line);
				sprintf(temp, "%s%d%s%s", "client ", clt[index].id, ": ", the_line);
				send(clt[i].fd, temp, strlen(temp), 0);
				memset(temp, 0, 1000000);
				memset(the_line, 0, 1000000);
				strcpy(the_line, rest);
				memset(rest, 0, 1000000);
			}
		}
		i++;
	}
	clt[index].size = 0;
	memset(clt[index].buff, 0, 1000000);
}

void	ft_handle_connection()
{
	int 		i;
	socklen_t	clt_size;

	if (FD_ISSET(srv.fd, &srv.tmp_read))
	{
		int fd;
		ft_new_client();
		srv.nbr_of_clt++;
		clt_size = sizeof(clt[srv.nbr_of_clt - 1].addr);
		fd = accept(srv.fd, (struct sockaddr *)&clt[srv.nbr_of_clt - 1].addr, &clt_size);
		ft_error(fd);
		clt[srv.nbr_of_clt - 1].fd = fd;
		FD_SET(clt[srv.nbr_of_clt - 1].fd, &srv.read);
		srv.id++;
		clt[srv.nbr_of_clt - 1].id = srv.id;
		ft_send_welcome_msg();
		ft_max_fd();
	}
	i = 0;
	while (i < srv.nbr_of_clt)
	{
		if (FD_ISSET(clt[i].fd, &srv.tmp_read))
		{
			int		rd;
			char		line[1000000];

			memset(line, 0, 1000000);
			rd = 0;
			rd = recv(clt[i].fd, line, 999999, 0);
			if (rd <= 0)
			{
				ft_close_connection(i);
				ft_max_fd();
			}
			else
			{
				line[rd] = 0;
				strcpy(&clt[i].buff[clt[i].size], line);
				clt[i].size = strlen(clt[i].buff);
				if (strstr(line, "\n") != NULL)
					ft_send_message(i, clt[i].buff);
			}
		}
		i++;
	}
}

int	main(int ac, char **av)
{
	int					port;
	int					status;
	int					reuse;
	int					flags;

	clt = NULL;
	srv.id = -1;
	srv.nbr_of_clt = 0;
	if (ac != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		exit(1);
	}
	port = atoi(av[1]);
	if (port < 1024 || port > 65535)
		ft_error(-1);
	srv.fd = socket(AF_INET, SOCK_STREAM, 0);
	ft_error(srv.fd);
	ft_error(status);
	srv.addr.sin_family = AF_INET;
	srv.addr.sin_port = htons(port);
	srv.addr.sin_addr.s_addr = htonl(2130706433);
	status = bind(srv.fd, (struct sockaddr *)&srv.addr, sizeof(srv.addr));
	ft_error(status);
	status = listen(srv.fd, 127);
	ft_error(status);
	FD_ZERO(&srv.read);
	FD_SET(srv.fd, &srv.read);
	srv.fd_max = srv.fd + 1;
	while (1)
	{
		FD_ZERO(&srv.tmp_read);
		srv.tmp_read = srv.read;
		status = select(srv.fd_max, &srv.tmp_read, 0, 0, 0);
		if (status <= 0)
		{
			free(clt);
			ft_error(status);
		}
		ft_handle_connection();	
	}
	return (0);
}
