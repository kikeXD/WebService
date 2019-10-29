#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <stdlib.h>
#include "../include/client.h"

#define BUFFER 8096


void get_directory_path(char* request, char* path)
{
    char* path_init = strchr(request, ' ');
    // printf("%s", path_init);

    int pos = 0;
    int i = 1;
    while(path_init[i] != ' ')
    {
        if(path_init[i] == '%')
        {
            if(path_init[i + 1] == '2')
            {
                if(path_init[i + 2] == '0')
                {
                    path[pos] = ' ';
                    i += 2;
                }
            }
        }
        else
        {
            path[pos] = path_init[i];
        }
        pos++;
        i++;
    }
    path[i - 1] = '\000';
    printf("------------+-->%s\n", path);
}

void comprobate_path(char *new_path, char *origin_path)
{
    if(strlen(new_path) < strlen(origin_path))
    {
        strcpy(new_path, origin_path);
        return;
    }
    for(int i = 0; i < strlen(origin_path); i++)
    {
        if(new_path[i] != origin_path[i])
        {
            strcpy(new_path, origin_path);
            return;
        }
    }
}

int server(int argc, char **argv)
{
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    char serving_directory[BUFFER];

    fd_set read_set;
    fd_set write_set;

    //CHECK COMMAND LINE ARGS
    if(argc != 3)
    {
        fprintf(stderr, "usage %s <port> at %s \n", "argv[1", argv[2]);
        exit(1);
    }
    int portno = atoi(argv[1]);
    printf("Listening in port %d\n", portno);
    strcpy (serving_directory, argv[2]);
    printf("Serving directory \"%s\"\n", serving_directory);


    int listefd = socket(AF_INET, SOCK_STREAM, 0);
    if(listefd < 0)
    {
        printf("ERROR opening socket");
    }


    int optval = 1;
    setsockopt(listefd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));


    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((unsigned short)portno);


    if(bind(listefd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("ERROR on binding");
    }

    if(listen(listefd, 5) < 0)
    {
        printf("ERROR on listen");
    }

    int client_len = sizeof(client_addr);

    char path[BUFFER];
    char serving_directory_temp[BUFFER];

    int connfd = 0;
    while(1)
    {
        printf("--> Waiting for conection...\n");
        connfd = accept(listefd, (struct sockaddr *)&client_addr, &client_len);
        write(STDOUT_FILENO, "accept\n", 7);
        if(connfd < 0)
        {
            printf("ERROR on accept");
            return -1;
        }

        printf("--> Connection established with: %s.\n", inet_ntoa(client_addr.sin_addr));

        char buffer[BUFFER];
        int readcount = read(connfd, buffer, BUFFER);
        printf("--> recibing request\n");

        get_directory_path(buffer, path);
        strcpy(serving_directory_temp, path);
        if(strcmp(serving_directory_temp, "/favicon.ico\0") == 0)
        {
            // printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
            close(connfd);
            continue;
        }

        comprobate_path(serving_directory_temp, argv[2]);

        if(serving_directory_temp[strlen(serving_directory_temp) - 1] == '/')
        {
            strcpy(serving_directory, serving_directory_temp);
            client_dir(serving_directory, connfd, strcmp(serving_directory, argv[2]));
        }
        else
        {
            client_file(serving_directory_temp, connfd);
        }
        
         
        close(connfd);
    }
    close(connfd);
    printf("--> Socket closed.\n");
}