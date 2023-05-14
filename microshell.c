#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void	ft_dup2(int fd1, int fd2)
{
	if (dup2(fd1, fd2) == -1)
	{
		write(2, "error: fatal\n", 13);
		exit(1);
	}
}

void	ft_pipe(int fd[2])
{
	if (pipe(fd) == -1)
	{
		write(2, "error: fatal\n", 13);
		exit(1);
	}
}

void	ft_close(int fd)
{
	if (close(fd) == -1)
	{
		write(2, "error: fatal\n", 13);
		exit(1);
	}
}

int	ft_dup(int fd)
{
	int	fd_dup;

	fd_dup = dup(fd);
	if (fd_dup == -1)
	{
		write(2, "error: fatal\n", 13);
		exit(1);
	}
	return (fd_dup);
}

size_t	ft_strlen(char *str)
{
	size_t	len;

	len = 0;
	while (str[len])
		len++;
	return (len);
}

int	ft_fork(void)
{
	int	id;

	id = fork();
	if (id == -1)
		exit(1);
	return (id);
}

void	print_error(char *cmd)
{
	write(2, "error: cannot execute ", 22);
	write(2, cmd, ft_strlen(cmd));
	write(2, "\n", 1);
	exit(1);
}

int	count_pipes(char **av)
{
	int	pipe_nbr;
	int	i;

	pipe_nbr = 0;
	i = 0;
	while (av[i] && strcmp(av[i], ";"))
	{
		if (strcmp(av[i], "|") == 0)
			++pipe_nbr;
		i++;
	}
	return (pipe_nbr);
}

int	set_arguments(char **av)
{
	int	ret_val;
	int	i;

	i = 0;
	while (av[i] && strcmp(av[i], "|") && strcmp(av[i], ";"))
		i++;
	if (av[i] == NULL)
		return (i);
	if (strcmp(av[i], "|") == 0)
		ret_val = i + 1;
	else
		ret_val = i;
	av[i] = NULL;
	return (ret_val);
}

void	execute_cd_cmd(char **args)
{
	if (args[1] == NULL || args[2])
	{
		write(2, "error: cd: bad arguments\n", 25);
		return;
	}
	if (chdir(args[1]) == -1)
	{
		write(2, "error: cd: cannot change directory to ", 38);
		write(2, args[1], ft_strlen(args[1]));
		write(2, "\n", 1);
		return;
	}
}

void	execute_one_cmd(char **args, char **env)
{
	if (fork() == 0)
	{
		execve(args[0], args, env);
		print_error(args[0]);
	}
}

void	close_pipe(int fd[2])
{
	ft_close(fd[0]);
	ft_close(fd[1]);
}

void	redirect_io(int fd[2], int pipe_nbr)
{
	if (pipe_nbr == 0)
		return ;
	ft_dup2(fd[1], 1);
	close_pipe(fd);
}

int	execute_single_cmd(char **av, char **env)
{
	int	args_nbr;

	args_nbr = set_arguments(av);
	if (args_nbr == 0)
		return (0);
	if (strcmp(av[0], "cd") == 0)
		execute_cd_cmd(av);
	else
	{
		execute_one_cmd(av, env);
		wait(NULL);
	}
	return (args_nbr);
}

int	main(int ac, char **av, char **env)
{
	int		stdin_tmp;
	int		pipe_nbr;
	int		args_nbr;
	int		fd[2];
	pid_t	id;
	int		i;

	if (ac < 2)
		exit(1);
	++av;
	stdin_tmp = ft_dup(0);
	i = -1;
	while (++i < ac)
	{
		pipe_nbr = count_pipes(av + i);
		if (pipe_nbr == 0)
		{
			i += execute_single_cmd(av + i, env);
			continue ;
		}
		id = 0;
		while (i < ac && av[i] && strcmp(av[i], ";"))
		{
			ft_pipe(fd);
			args_nbr = set_arguments(av + i);
			if (strcmp(av[i], "cd") == 0)
			{
				execute_cd_cmd(av);
				i += args_nbr;
				ft_dup2(fd[0], 0);
				close_pipe(fd);
				continue;
			}
			id = ft_fork();
			if (id == 0)
			{
				redirect_io(fd, pipe_nbr);
				execve(av[i], av + i, env);
				print_error(av[i]);
			}
			ft_dup2(fd[0], 0);
			close_pipe(fd);
			--pipe_nbr;
			i += args_nbr;
		}
		ft_dup2(stdin_tmp, 0);
		while (waitpid(-1, NULL, 0) != -1)
			;
	}
	ft_close(stdin_tmp);
	return (0);
}
