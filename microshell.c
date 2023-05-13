#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define CD_NUM "error: cd: bad arguments\n"
#define CD_FAIL "error: cd: cannot change directory to "
#define SYS_CALL "error: fatal\n"
#define EXECVE_ERR "error: cannot execute "

typedef struct s_ids
{
	pid_t			id;
	struct s_ids	*next;
}	t_ids;

t_ids	*last_id(t_ids *ids)
{
	while (ids && ids->next)
		ids = ids->next;
	return (ids);
}

void	add_id(t_ids **ids, pid_t id)
{
	t_ids	*tmp;

	if (*ids == NULL)
	{
		*ids = malloc(sizeof(t_ids));
		(*ids)->id = id;
		(*ids)->next = NULL;
		return ;
	}
	tmp = NULL;
	add_id(&tmp, id);
	last_id(*ids)->next = tmp;
}

void	clear(t_ids **ids)
{
	t_ids	*tmp;

	while (*ids)
	{
		tmp = *ids;
		*ids = (*ids)->next;
		free(tmp);
	}
	*ids = NULL;
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

void	execute_cd_cmd(char **args)
{
	if (args[1] == NULL || args[2])
	{
		write(2, CD_NUM, 26);
		return;
	}
	if (chdir(args[1]) == -1)
	{
		write(2, CD_FAIL, 26);
		write(2, args[1], ft_strlen(args[1]));
		write(2, "\n", 1);
		return;
	}
}

void	execute_one_cmd(char **args, char **env, int stdin_tmp)
{
	if (fork() == 0)
	{
		close(stdin_tmp);
		execve(args[0], args, env);
		print_error(args[0]);
	}
}

void	redirect_io(int fd[2], int pipe_nbr, int cmd_nbr)
{
	if (pipe_nbr == 0)
		return ;
	if (cmd_nbr < pipe_nbr)
		dup2(fd[1], 1);
	close(fd[0]);
	close(fd[1]);
}

int	main(int ac, char **av, char **env)
{
	char	*args[10000];
	t_ids	*ids;
	int		stdin_tmp;
	int		pipe_nbr;
	int		args_nbr;
	int		cmd_nbr;
	int		fd[2];
	pid_t	id;
	int		i;

	if (ac < 2)
		exit(0);
	for (int f = 0; f < 10000; f++)
		args[f] = NULL;
	++av;
	stdin_tmp = dup(0);
	i = -1;
	while (++i < ac)
	{
		pipe_nbr = count_pipes(av + i);
		if (pipe_nbr == 0)
		{
			args_nbr = set_arguments(args, av + i);
			if (args_nbr != 0)
			{
				if (strcmp(args[0], "cd") == 0)
					execute_cd_cmd(args);
				else
				{
					execute_one_cmd(args, env, stdin_tmp);
					wait(NULL);
				}
			}
			i += args_nbr;
			continue ;
		}
		ids = NULL;
		cmd_nbr = 0;
		id = 0;
		while (i < ac && av[i] && strcmp(av[i], ";"))
		{
			pipe(fd);
			args_nbr = set_arguments(args, av + i);
			if (strcmp(args[0], "cd") == 0)
			{
				execute_cd_cmd(args);
				i += args_nbr;
				dup2(fd[0], 0);
				close(fd[0]);
				close(fd[1]);
				continue;
			}
			id = ft_fork();
			if (id == 0)
			{
				clear(&ids);
				redirect_io(fd, pipe_nbr, cmd_nbr);
				close(stdin_tmp);
				execve(args[0], args, env);
				print_error(args[0]);
			}
			add_id(&ids, id);
			dup2(fd[0], 0);
			close(fd[0]);
			close(fd[1]);
			cmd_nbr++;
			i += args_nbr;
		}
		dup2(stdin_tmp, 0);
		stdin_tmp = -1;
		for (t_ids *tmp = ids; tmp; tmp = tmp->next)
			waitpid(tmp->id, NULL, 0);
		clear(&ids);
	}
	close(stdin_tmp);
	return (0);
}
