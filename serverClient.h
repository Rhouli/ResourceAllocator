/*
 *Ryan Houlihan 
 *Lab: Resource Allocator 
 *CIS 3207 
 *
 * Header File
 */

#ifndef serverClient
#define serverClient

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <sys/msg.h>
#include <sys/time.h>

#define ALLOCATE 2
#define RETURN 3
#define DONE 4
#define CLIENTNUM 4
#define BUFSIZE 9
#define FILENAME "./serverClientLog.txt"
#define RESOURCES 10
#define TURNS 20

typedef struct mymsgbuf { long mtype; long mfrom; long mto; int message[BUFSIZE];} MESSAGE;

FILE *file;

void *server(void*);
void *findResource(int*, int*, int);
void *returnResource(int, int, long);
void *client(void*);
#endif
