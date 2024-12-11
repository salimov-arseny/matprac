#include "header.h"

pthread_barrier_t barrier;

int main(int argc, char* argv[])
{
    Args* a;
    int k;

    if(argc == 1)
    {
        printf("Usage %s files\n", argv[0]);
        return -1;
    }

    int p = argc - 1;
    a = new Args[p];
    if(a == nullptr)
    {
        printf("Can not allocate memory\n");
        return -2;
    }

    pthread_barrier_init(&barrier, NULL, p);

    for(k = 0; k < p; k++)
    {
        a[k].k = k;
        a[k].p = p;
        a[k].name = argv[k + 1];
    }

    for(k = 1; k < p; k++)
    {
        if(pthread_create(&a[k].tid, nullptr, thread_func, a + k))
        {
            printf("Can not create thread %d\n", k);
            delete[] a;
            return -3;
        }
    }

    a[0].tid = pthread_self();

    thread_func(a + 0);
    

    for(k = 1; k < p; k++)
        pthread_join(a[k].tid, nullptr);
    
    if(process_args(a) == 0)
        printf("Result = %d\n", a[0].res);

    delete []a;
    pthread_barrier_destroy(&barrier);
    
    return 0;
}
