#include <memkind.h>

#include <iostream>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string>
#include <initializer_list>


using namespace std;

#define PMEM_MAX_SIZE (1024 * 1024 * 32)
static char path[PATH_MAX] = "/tmp/";

class Emb {
private:

public:

    struct memkind *pmem_kind;
    int err;
    const size_t stream_len = 64 ;
    const size_t col_len = 32 ; 
    size_t i, j;
    double **stream = NULL;

    Emb (){
    err = memkind_create_pmem(path, PMEM_MAX_SIZE, &pmem_kind);
    if (err) {
        print_err_message(err);
        
    }
    }

    void setEmb(){
        
        srandom(0);

        stream = (double **)memkind_malloc(pmem_kind, stream_len * sizeof(double *));
        for (i = 0 ; i < stream_len ; i++){
            *(stream + i) = (double *) memkind_malloc(pmem_kind , col_len * sizeof(double));
        }
        if (stream == NULL) {
            perror("<memkind>");
            fprintf(stderr, "Unable to allocate stream\n");
            
        }

        for (i = 0; i < stream_len; i++) {
            for (j = 0 ; j < col_len ; j++){
            stream[i][j] = (double)(random()) / (double)(RAND_MAX);
        

        }
        
    }

    }
    
    template <typename T>
    void getEmb(T t) 
    {
        for (i = 0; i < stream_len; i++) {
            if (t == i){
                for (j = 0; j < col_len ; j++)
                    cout<<"stream "<<i<<"---"<<stream[i][j]<<endl;
            }
    }
    }

    template<typename T, typename... Args>
    void getEmb(T t, Args... args) 
    {

    for (i = 0; i < stream_len; i++) {
            if (i == t){
                for (j = 0; j < col_len ; j++)
                    cout<<"stream "<<i<<"---"<<stream[i][j]<<endl;
            }
            if(i == t)
                cout<<endl;
    }



    getEmb(args...) ;
    }
    

    static void print_err_message(int err)
{
    char error_message[MEMKIND_ERROR_MESSAGE_SIZE];
    memkind_error_message(err, error_message, MEMKIND_ERROR_MESSAGE_SIZE);
    fprintf(stderr, "%s\n", error_message);

}

~Emb(){
    memkind_free(pmem_kind, stream);

    err = memkind_destroy_kind(pmem_kind);
    if (err) {
        print_err_message(err);
        
    }

    fprintf(stdout, "Memory was successfully allocated and released.\n");

}

};



int main(int argc, char **argv)
{

    Emb emb;
    emb.setEmb();
    emb.getEmb(0,1,2); 
    return 0;
}
