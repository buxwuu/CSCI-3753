//
//  multi-lookup.c
//  
//
//  Created by Kevin Kirk on 10/27/18.
//

#include "multi-lookup.h"
#define NUM_THREADS 1
#define USAGE "<inputFilePath> <outputFilePath>"
#define MINARGS 3
#define SBUFSIZE 1025
#define INPUTFS "%1024s"

queue request;
pthread_mutex_t lock;

void *readURL(void *file){
    FILE* inputfp = NULL;
    int fail;
    char* hostname = malloc(SBUFSIZE*sizeof(char));
    if (hostname == NULL){
        printf("Memory allocation failed.\n");
    }
    inputfp = fopen((char* )file, "r");
    
    if(!inputfp) {
        fprintf(stderr,"Error Opening Input File: %s\n", (char *) file);
        //Clean up after the failed file
        free(hostname);
        return NULL;
    }
    
    // free(hostname);
    while (fscanf(inputfp, INPUTFS, hostname)>0){
        char *name = strdup(hostname);
        pthread_mutex_lock(&lock);
        fail = queue_push(&request, (void*) name);
        pthread_mutex_unlock(&lock);
        
        
        while(fail == QUEUE_FAILURE){
            pthread_mutex_lock(&lock);
            queue_push(&request, (void*) name);
            pthread_mutex_unlock(&lock);
            if (fail == QUEUE_FAILURE) {
                usleep(rand()%100);
            }
        }
    }
    free(hostname);
    fclose(inputfp);
    return NULL;
    
}

void* resolve(void* outputFile) {
    char firstipstr[INET6_ADDRSTRLEN];
    
    /* Open Output File */
    FILE* outputfp = NULL;
    outputfp = fopen((char*) outputFile, "a");
    if(!outputfp) {
        perror("Error Opening Output File\n");
        return NULL;
    }
    while(1){
    	char* hostname;
        pthread_mutex_lock(&lock);
        hostname = (char*)queue_pop(&request);
        printf("%s\n", hostname);
        pthread_mutex_unlock(&lock);
        
        if (hostname != NULL) {
            if(dnslookup(hostname, firstipstr, sizeof(firstipstr))
               == UTIL_FAILURE) {
                fprintf(stderr, "dnslookup error: %s\n", hostname);
                strncpy(firstipstr, "", sizeof(firstipstr));
            }
            pthread_mutex_lock(&lock);
            printf("%s\n", hostname);
            fprintf(outputfp, "%s,%s\n", hostname, firstipstr);
            pthread_mutex_unlock(&lock);
            //free(hostname);
        }
        
    }
    fclose(outputfp);
    return NULL;
}

int main(int argc, char * argv[]){
    int numberofinputfiles = argc - 2; //got this command from stack overflow//
    //FILE* file;
    
    queue_init(&request, -1);
    //got these 2 commands from github, sets one thread per file
    pthread_t requester_threads[numberofinputfiles];
    
    pthread_t resolver_threads[1];
    void* status;//stackoverflow
    for (long i = 0; i < numberofinputfiles; i++){
        pthread_create(&requester_threads[i], NULL, readURL, (void *)argv[i+1]);
    }//moving on to the next file
    for (long i = 0; i < 1; i++){
        pthread_create(&resolver_threads[i], NULL, resolve, (void *)argv[argc-1]);
        
    }
    for (long i = 0; i < numberofinputfiles; i ++){
        pthread_join(requester_threads[i], &status);
        int* st = (int*)status;
        if ((*st) == 2){
            printf("Failed to join");
        }
    }
    for (long i = 0; i < 1; i ++){
        pthread_join(resolver_threads[i], &status);
        int* st = (int*)status;
        if ((*st) == 2){
            printf("It failed to join");
        }
    }
    
    pthread_mutex_destroy(&lock);
    queue_cleanup(&request);
    
    printf("Main: program completed.\n");
    return 0;
}
