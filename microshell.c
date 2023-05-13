#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define CD_NUM "error: cd: bad arguments\n"
#define CD_FAIL "error: cd: cannot change directory to "
#define SYS_CALL "error: fatal\n"
#define EXECVE_ERR "error: cannot execute "

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
	{
		// write(2, SYS_CALL, 14);
		exit(1);
	}
	return (id);
}

void	print_error(char *cmd)
{
	write(2, EXECVE_ERR, 23);
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

int	set_arguments(char **args, char **av)
{
	int	i;

	i = 0;
	while (av[i] && strcmp(av[i], "|") && strcmp(av[i], ";"))
	{
		args[i] = av[i];
		i++;
	}
	args[i] = NULL;
	return (av[i] && !strcmp(av[i], "|") ? i + 1 : i);
}

void	execute_one_cmd(char **args, char **env)
{
	if (strcmp(args[0], "cd") == 0)
	{
		if (args[1] == NULL || args[2])
		{
			write(2, CD_NUM, 26);
			return ;
		}
		if (chdir(args[1]) == -1)
		{
			write(2, CD_FAIL, 26);
			write(2, args[1], ft_strlen(args[1]));
			write(2, "\n", 1);
			return ;
		}
	}
	else if (ft_fork() == 0)
	{
		execve(args[0], args, env);
		print_error(args[0]);
	}
}

void	ft_pipe(int *fd)
{
	if (pipe(fd) == -1)
	{
		// write(2, SYS_CALL, 14);
		exit(1);
	}
}

void	ft_dup2(int fd1, int fd2)
{
	if (dup2(fd1, fd2) == -1)
	{
		// write(2, SYS_CALL, 14);
		exit(1);
	}
}

void	redirect_io(int fd[2], int pipe_nbr, int cmd_nbr)
{
	if (pipe_nbr == 0)
		return ;
	if (cmd_nbr < pipe_nbr)
		ft_dup2(fd[1], 1);
	close(fd[0]);
	close(fd[1]);
}

int	count_all(char **av)
{
	int 	a = 0;
	int		i = 0;

	while (av[i])
	{
		if (strcmp(av[i], "|") == 0)
			++a;
		i++;
	}
	return (a);
}

int	main(int ac, char **av, char **env)
{
	char	*args[10000];
	int		stdin_tmp;
	int		pipe_nbr;
	int		args_nbr;
	int		cmd_nbr;
	int		fd[2];
	pid_t	id;
	int		i;

	if (ac < 2)
		exit(1);
	for (int f = 0; f < 10000; f++)
		args[f] = NULL;
	++av;
	i = -1;
	while (++i < ac)
	{
		stdin_tmp = dup(0);
		if (stdin_tmp == -1)
		{
			write(2, SYS_CALL, 14);
			exit(1);
		}
		pipe_nbr = count_pipes(av + i);
		if (pipe_nbr == 0)
		{
			args_nbr = set_arguments(args, av + i);
			if (args_nbr != 0)
				execute_one_cmd(args, env);
			i += args_nbr;
			continue ;
		}
		cmd_nbr = 0;
		id = 0;
		while (i < ac && av[i] && strcmp(av[i], ";"))
		{
			ft_pipe(fd);
			args_nbr = set_arguments(args, av + i);
			if (strcmp(args[0], "cd") == 0)
			{
				if (args[1] == NULL || args[2])
					write(2, CD_NUM, 26);
				else if (chdir(args[1]) == -1)
				{
					write(2, CD_FAIL, 26);
					write(2, args[1], 4096);
					write(2, "\n", 1);
				}
				i += args_nbr;
				if (pipe_nbr)
				{
					close(fd[0]);
					close(fd[1]);
				}
				continue;
			}
			id = ft_fork();
			if (id == 0)
			{
				redirect_io(fd, pipe_nbr, cmd_nbr);
				execve(args[0], args, env);
				print_error(args[0]);
			}
			if (pipe_nbr)
			{
				ft_dup2(fd[0], 0);
				close(fd[0]);
				close(fd[1]);
				wait(NULL);
			}
			cmd_nbr++;
			i += args_nbr;
		}
		ft_dup2(stdin_tmp, 0);
		close(stdin_tmp);
		// if (id != 0)
		// 	waitpid(id, NULL, 0);
		// wait(NULL);
	// while (pipe_nbr--)
	// 	wait(NULL);
	}
	int count = count_all(av);
	while (count--)
		wait(NULL);
	return (0);
}
