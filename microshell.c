#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define CD_NUM "error: cd: bad arguments\n"
#define CD_FAIL "error: cd: cannot change directory to %s\n"
#define SYS_CALL "error: fatal\n"
#define EXECVE_ERR "error: cannot execute %s\n"

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
		if (argv[1] && argv[2])
		{
			write(2, CD_NUM, 26);
			return;
		}
		if (chdir(argv[1]))
		{
			write(2, CD_FAIL, 42);
			write(2, argv[1], 4096);
			return;
		}
		return;
	}
	if (fork() == 0)
    {
        execve(argv[0], argv, env);
        perror(argv[0]);
        exit (1);
    }
}

int main(int ac, char **av, char **env)
{
    (void)ac;
    int     fd[2];
    int     pipe_count;
    int        i;
    char    *argv[1000000];
    pid_t   id;
    int     j;
    int     k;

    i = -1;
    j = 0;
    ++av;
    // while (av[++i])
    // {
    //     printf("argv : %s\n", av[i]);
    //     getchar();
    // }
    // return (1);
    while (++i < ac)
    {
        //printf("%d\n", i);
        //getchar();
        pipe_count = count_pipes(av + i);
        if (pipe_count == 0)
        {
            j = argument_to_execute(argv, av + i);
            execute_one_cmd(argv, env);
            i += j;
        }
        while (av[i] && strcmp(av[i], ";"))
        {
            if (pipe_count)
                pipe(fd);
            k = 0;
            j = i;
            j = argument_to_execute(argv, av + j);
			if (strcmp(argv[0], "cd") == 0)
            {
                if (argv[1] && argv[2])
                {
                    write(2, CD_NUM, 26);
                    i += j;
                    continue;
                }
                if (chdir(argv[1]))
                {
                    write(2, CD_FAIL, 42);
                    write(2, argv[1], 4096);
                    i += j;
                    continue;
                }
            }
            id = fork();
            if (id == 0)
            {
                if (k == 0)
                {
                    dup2(fd[1], 1);
                    close(fd[1]);
                }
                else if (k < pipe_count - 1)
                {
                    dup2(fd[1], 1);
                    dup2(fd[0], 0);
                    close(fd[1]);
                    close(fd[0]);
                }
                else
                {
                    dup2(fd[0], 0);
                    close (fd[0]);
                }
                execve(argv[0], argv, env);
                perror(argv[0]);
                return (1);
            }
            i += j;
            k++;
        }
            if (pipe_count)
            {
                close(fd[1]);
                close(fd[0]);
            }
    }
    waitpid(-1, NULL, 0);
}

//int main(int ac, char **av, char **env)
//{
//    (void)ac;
//    int     fd[2];
//    int     pipe_count;
//    int        i;
//    char    *argv[10000000];
//    pid_t   id;
//    int     j;
//    int     k;

//    i = -1;
//    j = 0;
//    ++av;
//    while (++i < ac)
//    {
//        pipe_count = count_pipes(av + i);
//        if (pipe_count == 0)
//        {
//            j = argument_to_execute(argv, av + i);
//            execute_one_cmd(argv, env);
//            i += j;
//        }
//        while (av[i] && strcmp(av[i], ";"))
//        {
//            if (pipe_count)
//                pipe(fd);
//            k = 0;
//            j = i;
//            j = argument_to_execute(argv, av + j);
//            //if (strcmp(argv[0], "cd") == 0)
//            //{
//            //    if (argv[1] && argv[2])
//            //    {
//            //        write(2, CD_NUM, 26);
//            //        i += j;
//            //        continue;
//            //    }
//            //    if (chdir(argv[1]))
//            //    {
//            //        write(2, CD_FAIL, 42);
//            //        write(2, argv[1], 4096);
//            //        i += j;
//            //        continue;
//            //    }
//            //}
//            id = fork();
//            if (id == 0)
//            {
//                if (k == 0)
//                {
//                    dup2(fd[1], 1);
//                    close(fd[1]);
//                }
//                else if (k < pipe_count - 1)
//                {
//                    dup2(fd[1], 1);
//                    dup2(fd[0], 0);
//                    close(fd[1]);
//                    close(fd[0]);
//                }
//                else
//                {
//                    dup2(fd[0], 0);
//                    close (fd[0]);
//                }
//                execve(argv[0], argv, env);
//                perror(argv[0]);
//                return (1);
//            }
//            i += j;
//            k++;
//		}
//    }
//        if (pipe_count)
//        {
//            close(fd[1]);
//            close(fd[0]);
//        }
//    waitpid(-1, NULL, 0);
//    return (0);
//}