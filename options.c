#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h> 
#include "options.h"

/**
 * Method to run command that is found by parsing line
 **/
int runcmd(char * line, int rc) {
     if (rc == 0) { 
        printf("Komut: %s\n", line);

        char* token = strtok(line, " ");

        char **argv = malloc(sizeof(char*) * 50);
        int token_count = 0;

        while (token != NULL) {
            argv[token_count++] = token;
            token = strtok(NULL, " "); 
        }

        argv[token_count] = NULL;

        argv = realloc(argv, sizeof(char*) * token_count);

        if(strcmp(argv[0], "cd") == 0) {
            chdir(argv[1]);
            char s[100];
            printf("Current dir: %s\n", getcwd(s,100));
        } else {
            execvp(argv[0], argv);
            printf("Error executing command!\n");
            _exit(1);
        }
        _exit(1);
    } else if(rc > 0) { 
        int status;
        waitpid(rc, &status, 0);

    } else { // fail
        printf("Error forking!\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * Option for sequential running of commands
 **/
void option1(char *file) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char **history = malloc(sizeof(char*) * 1000);
    int history_size = 0;

    fp = fopen(file, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        if(strstr(line, "\n") != NULL) {
            line[strlen(line) - 1] = '\0';
        }

        if (strcmp(line, "History") == 0) {
            printf("Komut: %s\n", line);
            for(int m = 0; m < history_size; m++) {
                printf("---%s\n", history[m]);
            }
            history[history_size] = malloc(sizeof(char)* (strlen(line) + 1));
            strcpy(history[history_size++], line);
        } else {
            history[history_size] = malloc(sizeof(char)* (strlen(line) + 1));
            strcpy(history[history_size++], line);
            runcmd(line, fork());
        }

    }
        if (line)
            free(line);
    fclose(fp);
    

    printf("Komutlar bitti\n");

   
}

/**
 * Option for parallel running of commands
 **/
void option2(char *file) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(file, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        if(strstr(line, "\n") != NULL) {
            line[strlen(line) - 1] = '\0';
        }

        char* token = strtok(line, ";");
        int token_count = 0;

        char **commands = malloc(sizeof(char*) * 100);

        while (token != NULL) {
            commands[token_count++] = token;
            token = strtok(NULL, ";"); 
        }

        pid_t pid[token_count];

        for (int i = 0; i < token_count; i++) {
            if ((pid[i]=fork()) == 0) {
                printf("Komut: %s\n", commands[i]);

                char* token = strtok(commands[i], " ");

                char **argv = malloc(sizeof(char*) * 50);
                int token_count = 0;

                while (token != NULL) {
                    argv[token_count++] = token;
                    token = strtok(NULL, " "); 
                }

                argv[token_count] = NULL;

                argv = realloc(argv, sizeof(char*) * token_count);
                execvp(argv[0], argv);
                _exit(1);
            }
            
            if (pid[i] < 0) {
                perror("Fork error");
            }
        }

        for (int j = 0; j < token_count; j++) { 
            if (pid[j] > 0) {
                int status;
                waitpid(pid[j], &status, 0);
            }     
        }

    }

    fclose(fp);
    if (line)
        free(line);
}

/**
 * Method to run commands in commands_parsed with pipes
 **/
void runpip(char **commands_parsed[])
{
    pid_t pid;
	
    int fd[2];
	int fdd = 0;			

	while (*commands_parsed != NULL) {
		pipe(fd);	

		if ((pid = fork()) == 0) {
			dup2(fdd, 0);

			if (*(commands_parsed + 1) != NULL) {
				dup2(fd[1], 1);
			}

			close(fd[0]);
			execvp(*commands_parsed[0], *commands_parsed);
			_exit(1);
		}
		else {
			int status;
            waitpid(pid, &status, 0);
			close(fd[1]);
			fdd = fd[0];
			commands_parsed++;
		}
	}
}

/**
 * Option for piped running of commands
 **/
void option3(char *file) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(file, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        printf("Komut: %s\n", line);
        if(strstr(line, "\n") != NULL) {
            line[strlen(line) - 1] = '\0';
        }

        char* token = strtok(line, "|");
        int token_count = 0;

        char **commands = malloc(sizeof(char*) * 100);

        while (token != NULL) {
            commands[token_count++] = token;
            token = strtok(NULL, "|"); 
        }

        char ***commands_parsed = malloc(sizeof(char**) * 100);

        for (int i = 0; i < token_count; i++){
            char* token_sub = strtok(commands[i], " ");

            char **argv = malloc(sizeof(char*) * 50);
            int token_count_sub = 0;

            while (token_sub != NULL) {
                argv[token_count_sub++] = token_sub;
                token_sub = strtok(NULL, " "); 
            }

            argv[token_count_sub] = NULL;
            commands_parsed[i] = argv;
        }

        commands_parsed[token_count] = NULL;

        runpip(commands_parsed);
    }

    fclose(fp);
    if (line)
        free(line);
}

/**
 * Option for running of commands according to given input file
 **/
void option4(char *file) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(file, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1) {
        if(strstr(line, ";") != NULL) {
            option2(file);
            _exit(0);
        } else if(strstr(line, "|") != NULL) {
            option3(file);
            _exit(0);
        } else {
            option1(file);
            _exit(0);
        }
    }

    fclose(fp);
    if (line)
        free(line);
}
