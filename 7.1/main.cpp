#include "header.h"
//#include <fenv.h>

int main(int argc, char* argv[])
{
    //feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW);
    Args* a;
    unsigned long long int n, p, chunk = 1000, k, i;
    pthread_mutex_t mutex;

    if(argc != 3)
    {
        printf("Usage %s p n\n", argv[0]);
        return -1;
    }

    try
    {
        p = std::stoi(argv[1]);
        n = std::stoi(argv[2]);
    }
    catch (const std::invalid_argument& e) 
    {
        printf("Wrong arguments\n");
        return -2;
    }

    if(n <= 0 || p <= 0)
    {
        printf("Wrong arguments\n");
        return -2;
    }


    a = new Args[p];
    if(a == nullptr)
    {
        printf("Can not allocate memory for Args array\n");
        return -4;
    }

    pthread_mutex_init(&mutex, nullptr);
    
    for(k = 0; k < p; k++)
    {
        a[k].n = n;
        a[k].k = k;
        a[k].p = p;
        a[k].chunk = chunk;
        a[k].mutex = &mutex;
    }
    
    double t = get_full_time();

    for(k = 1; k < p; k++)
    {
        if(pthread_create(&a[k].tid, nullptr, thread_func, a + k))
        {
            printf("Can not create thread %llu\n", k);
            pthread_mutex_destroy(&mutex);
            delete[] a;
            return -3;
        }
    }

    a[0].tid = pthread_self();

    thread_func(a + 0);
    

    for(k = 1; k < p; k++)
        pthread_join(a[k].tid, nullptr);
        
    t = get_full_time() - t;
        
    printf("Result = %llu\n", a[0].res);
        
    get_variance(a);

    for(i = 0; i < p; i++)
        printf("\nThread %2llu time = %.12lf", i, a[i].cpu_time);
    printf("\ncpu_time variance = %.20lf\nTotal time = %.12lf\n", a[0].variance, t);
        

    delete[] a;
    pthread_mutex_destroy(&mutex);


    return 0;
}
