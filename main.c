#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#define FSH_RL_BUFSIZE 1024
#define FSH_TOK_BUFSIZE 64
#define FSH_TOK_DELIM " \t\r\n\a"

int fsh_cd(char **);
int fsh_help(char **);
int fsh_exit(char **);
int fsh_num_builtins();

void fsh_loop(void);
char *fsh_read_line(void);
char **fsh_split_line(char *);
int fsh_launch(char **);
int fsh_execute(char **);

// List of builtin commands, followed by their corresponding functions
char *builtin_str[] =
{
    "cd",
    "help",
    "exit"
};

int (*builtin_func[])(char **) =
{
    &fsh_cd,
    &fsh_help,
    &fsh_exit
};

/**************************** BUILT-IN COMMANDS ********************************/

int fsh_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

int fsh_cd(char **args)
{
    if (args[1] == NULL)
        fprintf(stderr, "fsh: expected argument to \"cd\"\n");
    else
    {
        if (chdir(args[1]) != 0)
            perror("fsh");
    }

    return 1;
}

int fsh_help(char **args)
{
    int i;
    printf("FSH shell by Syed Faraz Abrar\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < fsh_num_builtins(); i++)
        printf("\t%s\n", builtin_str[i]);

    printf("Use the man command for information regarding other programs.\n");
    return 1;
}

int fsh_exit(char **args)
{
    return 0;
}

/*****************************************************************************/

/********************************** FSH SHELL ********************************/

void fsh_loop(void)
{
    char *line;
    char **args;
    int status;

    do
    {
        printf("> ");
        line = fsh_read_line();
        args = fsh_split_line(line);
        status = fsh_execute(args);

        free(line);
        free(args);
    } while (status);
}

char *fsh_read_line(void)
{
    int bufsize = FSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer)
    {
        fprintf(stderr, "fsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // Read a character
        c = getchar();

        // If EOF, replace it with a null character and return
        if (c == EOF || c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position] = c;
        }
        position++;

        // If we exceed the buffer, reallocate
        if (position >= bufsize)
        {
            bufsize += FSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize * sizeof(char));

            if (!buffer)
            {
                fprintf(stderr, "fsh: reallocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char **fsh_split_line(char *line)
{
    int bufsize = FSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (!tokens)
    {
        fprintf(stderr, "fsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, FSH_TOK_DELIM);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= bufsize)
        {
            bufsize += FSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));

            if (!tokens)
            {
                fprintf(stderr, "fsh: reallocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, FSH_TOK_DELIM);
    }

    tokens[position] = NULL;
    return tokens;
}

int fsh_launch(char **args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        // Child process
        if (execvp(args[0], args) == -1)
            perror("fsh");
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        // Error in forking
        perror("fsh");
    }
    else
    {
        // Parent process
        do
        {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int fsh_execute(char **args)
{
    int i;

    if (args[0] == NULL)
        return 1;

    for (i = 0; i < fsh_num_builtins(); i++)
    {
        if (strcmp(args[0], builtin_str[i]) == 0)
            return (*builtin_func[i])(args);
    }

    return fsh_launch(args);
}

/**************************************************************************/

int main(int argc, char **argv)
{
    // Load config files here, if any

    // Run command loop
    fsh_loop();

    // Perform cleanup

    return EXIT_SUCCESS;
}
