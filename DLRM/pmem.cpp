#include <memkind.h>
#include <iostream>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


using namespace std; 

#define NUM_THREADS 10
#define NUM_ALLOCS  100
static char path[PATH_MAX] = "/tmp/";

static void print_err_message(int err)
{
    char error_message[MEMKIND_ERROR_MESSAGE_SIZE];
    memkind_error_message(err, error_message, MEMKIND_ERROR_MESSAGE_SIZE);
    fprintf(stderr, "%s\n", error_message);
}

struct arg_struct {
    int id;
    struct memkind *kind;
    double **ptr;
    int *rows;
    int arrSize;
};

void *thread_onekind(void *arg);

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int main(int argc, char *argv[])
{
    struct memkind *pmem_kind_unlimited = NULL;
    int err = 0;

    if (argc < 2){
        printf("Usage: %s <num1> <num2> .... <num-n>\n", argv[0]);
        exit(-1);
    }

    int num_args = argc -1;
    //cout<<" === "<<num_args << endl;
    int arr[num_args];

    for (int i = 1 ; i <= num_args ; i++){
    arr[0] = 0;
    arr[i] = atoi(argv[i]);

    }

    for (int i = 0 ; i <= num_args + 1; ++i)
        arr[i] = arr[i + 1]; 

  
    int size = sizeof(arr)/sizeof(arr[0]);
    //cout << "The size of the array is: " << size <<endl;
    //cout << "The number of arguments: " <<num_args << endl; 


    // for(int i = 0 ; i < num_args ; i++){

    //     cout<<arr[i]<<endl;

    // }

    fprintf(
        stdout,
        "Implementing multithreading ...."
        "\nPMEM kind directory: %s\n\n",
        path);

    
    // Create PMEM partition with unlimited size
    err = memkind_create_pmem(path, 0, &pmem_kind_unlimited);
    if (err) {
        print_err_message(err);
        return 1;
    }

    // Create a few threads which will access to our main pmem_kind
    pthread_t pmem_threads[NUM_THREADS];
    double *pmem_tint[NUM_THREADS][NUM_ALLOCS]; 
    int row[num_args];
    int t = 0, i = 0;

    struct arg_struct *args[NUM_THREADS];

    for (t = 0; t < NUM_THREADS; t++) {
        args[t] = (arg_struct *)malloc(sizeof(struct arg_struct));
        args[t]->id = t;
        args[t]->ptr = &pmem_tint[t][0];
        args[t]->kind = pmem_kind_unlimited;
        args[t]->rows = arr;
        args[t]->arrSize = size;

        if (pthread_create(&pmem_threads[t], NULL, thread_onekind,
                           (void *)args[t]) != 0) {
            fprintf(stderr, "Unable to create a thread.\n");
            return 1;
        }
    }

    sleep(1);
    if (pthread_cond_broadcast(&cond) != 0) {
        fprintf(stderr, "Unable to broadcast a condition.\n");
        return 1;
    }

    for (t = 0; t < NUM_THREADS; t++) {
        if (pthread_join(pmem_threads[t], NULL) != 0) {
            fprintf(stderr, "Thread join failed.\n");
            return 1;
        }
    }

    
    for (t = 0; t < NUM_THREADS; t++) {
        for (i = 0; i < NUM_ALLOCS; i++) {
         
            memkind_free(args[t]->kind, *(args[t]->ptr + i));
        }
        free(args[t]);
    }

    fprintf(stdout,
            "Threads successfully allocated memory in the PMEM kind.\n");

    return 0;
}

void *thread_onekind(void *arg)
{
    struct arg_struct *args = (struct arg_struct *)arg;
    int i;

    if (pthread_mutex_lock(&mutex) != 0) {
        fprintf(stderr, "Failed to acquire mutex.\n");
        return NULL;
    }
    if (pthread_cond_wait(&cond, &mutex) != 0) {
        fprintf(stderr, "Failed to block mutex on condition.\n");
        return NULL;
    }
    if (pthread_mutex_unlock(&mutex) != 0) {
        fprintf(stderr, "Failed to release mutex.\n");
        return NULL;
    }

    
    for (i = 0; i < NUM_ALLOCS; i++) {
        *(args->ptr + i) = (double *)memkind_malloc(args->kind, sizeof(double));
        if (*(args->ptr + i) == NULL) {
            fprintf(stderr, "Unable to allocate pmem int.\n");
            return NULL;
        }
        **(args->ptr + i) = (double)(random()) / (double)(RAND_MAX);
        
    }

    
        // for(int i = 0 ; i < NUM_THREADS ; i++){
        //     std::cout<<i<<**(args->ptr + i)<<std::endl;
        // }   
        // cout<<" =========== " << endl;

        
        for (int i = 0 ; i < args->arrSize ; i++){
            if (args->id == args->rows[i] ){
                cout<<"this is row --- "<<args->id <<endl; 
                for (int j = 0 ; j < NUM_THREADS ; j++){
                    cout<< "col " << j << " ---- " << **(args->ptr + j)<<endl;
                }
            }
        }

    return NULL;
}
