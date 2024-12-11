#include "header.h"
#include <fenv.h>

int main(int argc, char* argv[])
{
    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW);
    Args* a;
    double* arr;
    int k, n, p;
    FILE* f;
    pthread_barrier_t barrier;

    if(argc != 4)
    {
        printf("Usage %s p n filename\n", argv[0]);
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

    f = fopen(argv[3], "r");
    if(!f)
    {
        printf("Can't open file\n");
        return -3;
    }

    a = new Args[p];
    if(a == nullptr)
    {
        printf("Can not allocate memory for Args array\n");
        return -4;
    }

    arr = new double[n];
    if(arr == nullptr)
    {
        printf("Can not allocate memory for array\n");
        return -5;
    }

    for(int i = 0; i < n; i++)
        if(fscanf(f, "%lf", &arr[i]) != 1)
        {
            printf("Trash in file %s\n", argv[3]);
            delete[] arr;
            delete[] a;
            fclose(f);
            return -6;
        }
    
    fclose(f);

    pthread_barrier_init(&barrier, 0, p);
    
    for(k = 0; k < p; k++)
    {
        a[k].arr = arr;
        a[k].n = n;
        a[k].k = k;
        a[k].p = p;
        a[k].barrier = &barrier;
    }
    
    double t = get_full_time();

    for(k = 1; k < p; k++)
    {
        if(pthread_create(&a[k].tid, nullptr, thread_func, a + k))
        {
            printf("Can not create thread %d\n", k);
            pthread_barrier_destroy(&barrier);
            delete[] arr;
            delete[] a;
            return -3;
        }
    }

    a[0].tid = pthread_self();

    thread_func(a + 0);
    

    for(k = 1; k < p; k++)
        pthread_join(a[k].tid, nullptr);
        
    t = get_full_time() - t;
        
    if(process_args(a) == 0)
        printf("RESULT %2d: ", a[0].p);
    
    for(int i = 0; i < n; i++)
        printf(" %lf ", arr[i]);
        
    get_variance(a);

    for(int i = 0; i < p; i++)
        printf("\nThread %2d time = %.12lf", i, a[i].cpu_time);
    printf("\ncpu_time variance = %.20lf\nTotal time = %.12lf\nTotal changed elements = %d\n", a[0].variance, t, a[0].changed);
        
    delete []a;
    delete []arr;
    pthread_barrier_destroy(&barrier);

    return 0;
}
