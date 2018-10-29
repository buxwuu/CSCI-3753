//
//  multi-lookup.c
//  
//
//  Created by Kevin Kirk on 10/27/18.
//

#include "multi-lookup.h"
#define USAGE "<inputFilePath> <outputFilePath>"
#define MINARGS 3
#define SBUFSIZE 1025
#define INPUTFS "%1024s"

queue request;
int requesters_finished = 0;

//pthread_mutex_t finished_lock;

void *readURL(void *file){
    pthread_mutex_t lock;
    FILE* inputfp = NULL;
    int fail;
    char* hostname = malloc(SBUFSIZE*sizeof(char)); /*https://pebble.gitbooks.io/learning-c-with-pebble/content/chapter08.html*/
    /*if (hostname == NULL){
     printf("Memory allocation failed.\n");
     }*/
    inputfp = fopen((char*)file, "r");
    
   /* if(!inputfp) {
        fprintf(stderr,"Error Opening Input File: %s\n", (char *) file);
        //Clean up after the failed file, took this out
        free(hostname);
        return NULL;
    }*/
    printf("%s\n", "Safe");
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

    //free(hostname);
    fclose(inputfp);
    printf("%s\n", "End of readURL");

    return NULL;
    
}

void* resolve(void* outputFile) {
    pthread_mutex_t lock;
    char firstipstr[INET6_ADDRSTRLEN];

    //char* hostname = NULL;

    /* Open Output File */
    FILE* outputfp = NULL;
    outputfp = fopen((char*) outputFile, "a");
    if(!outputfp) {
        perror("Error Opening Output File\n");
        return NULL;
    }
   // pthread_mutex_lock(&lock);
    //pthread_mutex_lock(&finished_lock);
    while(1){

        while(!queue_is_empty(&request) || !requesters_finished){
            char* hostname;// = malloc(SBUFSIZE*sizeof(char));
            pthread_mutex_lock(&lock);
            hostname = (char*)queue_pop(&request);
           // pthread_mutex_unlock(&lock);

        if (hostname != NULL && !!strcmp(hostname, "\0")) {
            printf("%s\n", hostname);
            if(dnslookup(hostname, firstipstr, sizeof(firstipstr))
               == UTIL_FAILURE) {
                fprintf(stderr, "dnslookup error: %s\n", hostname);
                strncpy(firstipstr, "", sizeof(firstipstr));
                
            }
           // pthread_mutex_lock(&lock);
            printf("%s\n", hostname);
            fprintf(outputfp, "%s,%s\n", hostname, firstipstr);
        }
            pthread_mutex_unlock(&lock);
                free(hostname);//The free function causes the space pointed to by ptr to be deallocated, that is, made available for further allocation.
                //return NULL;
           
            hostname = NULL;
          //  char* hostname;

           /* if(queue_is_empty == 1){
                break;
            }*/
       /* else {
            break;
        }*/

    }

        fclose(outputfp);
        return NULL;
}
   
}

int main(int argc, char * argv[]){
    pthread_mutex_t lock;
    int numberofinputfiles = argc - 2; //got this command from stack overflow//
    int RESOLVER_THREADS = sysconf(_SC_NPROCESSORS_ONLN);
    //FILE* file;
    
    int maxSize2 = queue_init(&request, -1);
    if(maxSize2 != 50){
        printf("Error creating the queue\n");
    }
    //got these 2 commands from github, sets one thread per file
    pthread_t requester_threads[numberofinputfiles];
    pthread_t resolver_threads[RESOLVER_THREADS];
    
    void* status;//stackoverflow
    for (long i = 0; i < numberofinputfiles; i++){
        pthread_create(&requester_threads[i], NULL, readURL, (void *)argv[i+1]);
    }//moving on to the next file
    for (long i = 0; i < RESOLVER_THREADS; i++){
        pthread_create(&resolver_threads[i], NULL, resolve, (void *)argv[argc-1]);
        
    }
    printf("%s\n", "here");

    for (long i = 0; i < numberofinputfiles; i ++){
        pthread_join(requester_threads[i], &status);
        /*int* st = (int*)status;
         if (*st == 2){
         printf("Failed to join");
         }*/
    }
    
    pthread_mutex_lock(&lock);
    requesters_finished = 1;
    pthread_mutex_unlock(&lock);
    
    for (long i = 0; i < RESOLVER_THREADS; i ++){
        pthread_join(resolver_threads[i], &status);
        /*int* st = (int*)status;
         if (*st == 2){
         printf("It failed to join");
         }*/
    }
    
    pthread_mutex_destroy(&lock);
    queue_cleanup(&request);
    
    printf("Main: program completed.\n");
    return 0;
}

