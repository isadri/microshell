#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define CD_NUM "error: cd: bad arguments\n"
#define CD_FAIL "error: cd: cannot change directory to "
#define SYS_CALL "error: fatal\n"
#define EXECVE_ERR "error: cannot execute "

void	print_error(char *cmd)
{
	write(2, EXECVE_ERR, 22);
	write(2, cmd, 10000);
	write(2, "\n", 1);
	exit(1);
}

int count_pipes(char **av)
{
    int pipe_count;
    int i;

    pipe_count = 0;
    i = 0;
    while (av[i] && strcmp(av[i], ";"))
    {
        if (strcmp(av[i], "|") == 0)
            pipe_count++;
        i++;
    }
    return (pipe_count);
}

int    argument_to_execute(char **argv, char **av)
{
    int i;

    i = 0;
    while (av[i] && strcmp(av[i], "|") && strcmp(av[i], ";"))
    {
        argv[i] = av[i];
        i++;
    }
    argv[i] = NULL;
    return (av[i] && !strcmp(av[i], "|") ? i + 1 : i);
}

void    execute_one_cmd(char **argv, char **env)
{

	if (strcmp(argv[0], "cd") == 0)
	{
		if (argv[1] == NULL || argv[2])
		{
			write(2, CD_NUM, 26);
			return;
		}
		if (chdir(argv[1]))
		{
			write(2, CD_FAIL, 26);
			write(2, argv[1], 4096);
			write(2, "\n", 1);
			return;
		}
		return;
	}
	if (fork() == 0)
    {
        execve(argv[0], argv, env);
        print_error(argv[0]);
    }
}

void	ft_dup2(int fd1, int fd2)
{
	if (dup2(fd1, fd2) == -1)
	{
		write(2, SYS_CALL, 14);
		exit(1);
	}
}

void	ft_close(int fd)
{
	if (close(fd) == -1)
	{
		write(2, SYS_CALL, 14);
		exit(1);
	}
}

void	redirect_io(int *fd, int pipe_count, int k)
{
	if (pipe_count == 0)
		return ;
	if (k == 0)
	{
		ft_dup2(fd[1], 1);
		ft_close(fd[1]);
		ft_close(fd[0]);
	}
	else if (k < pipe_count)
	{
		// ft_close(0);
		ft_dup2(fd[1], 1);
		ft_dup2(fd[0], 0);
		ft_close(fd[1]);
		ft_close(fd[0]);
	}
	else
	{
		// ft_close(0);
		ft_dup2(fd[0], 0);
		ft_close(fd[0]);
		ft_close(fd[1]);
	}
}

int main(int ac, char **av, char **env)
{
	(void)ac;
	int fd[2];
	int pipe_count;
	int i;
	char *argv[1000000];
	pid_t id;
	int j;
	int k;

	if (ac < 2)
		exit(1);
	i = -1;
	j = 0;
	pipe_count = 0;
	for (int f = 0; f < 1000000; f++)
		argv[f] = NULL;
	++av;
	while (++i < ac)
	{
		pipe_count = count_pipes(av + i);
		if (pipe_count == 0)
		{
			j = argument_to_execute(argv, av + i);
			if (j)
				execute_one_cmd(argv, env);
			i += j;
		}
		k = 0;
		if (pipe_count)
			if (pipe(fd) == -1)
			{
				write(2, SYS_CALL, 14);
				exit(1);
			}
		while (av[i] && strcmp(av[i], ";"))
		{
			j = i;
			j = argument_to_execute(argv, av + j);
			if (strcmp(argv[0], "cd") == 0)
			{
				if (argv[1] == NULL || argv[2])
				{
					write(2, CD_NUM, 26);
					i += j;
					continue;
				}
				if (chdir(argv[1]))
				{
					write(2, CD_FAIL, 26);
					write(2, argv[1], 4096);
					write(2, "\n", 1);
					i += j;
					continue;
				}
			}
			id = fork();
			if (id == -1)
			{
				write(2, SYS_CALL, 14);
				exit(1);
			}
			if (id == 0)
			{
				redirect_io(fd, pipe_count, k);
				execve(argv[0], argv, env);
				print_error(argv[0]);
			}
			i += j;
			k++;
		}
		if (pipe_count)
		{
			ft_close(fd[1]);
			ft_close(fd[0]);
		}
    	waitpid(-1, NULL, 0);
	}
	exit(0);
}
