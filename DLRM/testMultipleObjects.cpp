#include <memkind.h>
#include <iostream>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>


using namespace std; 

#define NUM_THREADS 10
#define NUM_ALLOCS  100
static char path[PATH_MAX] = "/tmp/";

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;



class Emb{
    
    public:

    struct arg_struct {
    int id;
    struct memkind *kind;
    double **ptr;
    int *rows;
    int arrSize;
};
    
    static void print_err_message(int err)
{
    char error_message[MEMKIND_ERROR_MESSAGE_SIZE];
    memkind_error_message(err, error_message, MEMKIND_ERROR_MESSAGE_SIZE);
    fprintf(stderr, "%s\n", error_message);
}

static void getEmb(vector<int> vect){
    struct memkind *pmem_kind_unlimited = NULL;
    int err = 0;

    int arr[vect.size()];
    for(int i{}; i < vect.size(); i++){
        arr[i] = vect[i];
    }

    int size = vect.size();


    fprintf(
        stdout,
        "Implementing multithreading ...."
        "\nPMEM kind directory: %s\n\n",
        path);

    
    // Create PMEM partition with unlimited size
    err = memkind_create_pmem(path, 0, &pmem_kind_unlimited);
    if (err) {
        print_err_message(err);
        
    }

    // Create a few threads which will access to our main pmem_kind
    pthread_t pmem_threads[NUM_THREADS];
    double *pmem_tint[NUM_THREADS][NUM_ALLOCS]; 
    
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
            
        }
    }

    sleep(1);
    if (pthread_cond_broadcast(&cond) != 0) {
        fprintf(stderr, "Unable to broadcast a condition.\n");
        
    }

    for (t = 0; t < NUM_THREADS; t++) {
        if (pthread_join(pmem_threads[t], NULL) != 0) {
            fprintf(stderr, "Thread join failed.\n");
            
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

};

static void *thread_onekind(void *arg)
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


};



int main(int argc, char *argv[])
{
    Emb emb ; 
    Emb emb2 ;

    emb.getEmb(vector<int>{1,2});
    emb2.getEmb(vector<int>{1,2});
    return 0;
}

