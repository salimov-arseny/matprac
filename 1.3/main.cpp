#include "header.h"

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
            delete []a;
            return -3;
        }
    }

    a[0].tid = pthread_self();

    thread_func(a + 0);
    
    for(k = 1; k < p; k++)
        pthread_join(a[k].tid, nullptr);
    
    if(a[0].error_flag == 0)
        for(k = 1; k < p; k++)
        {
            if(a[k].error_flag == 0)
                a[0].res += a[k].res;
            else
            {
                printf("Error in thread %d: %d\n", k, int(a[k].error_type));
                delete []a;
                return -4;
            }
        }
    else
    {
        printf("Error in thread %d: %d\n", 0, int(a[0].error_type));
        delete []a;
                return -4;
    }

    printf("Result = %d\n", a[0].res);
    delete []a;
    return 0;
}
