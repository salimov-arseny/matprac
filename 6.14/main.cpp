#include "header.h"

#include <fenv.h>

int main(int argc, char* argv[]) 
{
    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW);
    if (argc != 5) {
        printf("Usage: %s p n1 n2 [filename]\n", argv[0]);
        return 1;
    }

    int n1, n2, p, k, i, j;
    double* A = nullptr;
    Args* a;
    FILE* f;
    pthread_barrier_t barrier;

    try
    {
        p = std::stoi(argv[1]);
        n1 = std::stoi(argv[2]);
        n2 = std::stoi(argv[3]);
    }
    catch(const std::invalid_argument&)
    {
        printf("Wrong arguments\n"); 
        return 1;
    }

    if(n1 <= 0 || n2 <= 0 || p <= 0)
    {
        printf("Wrong arguments\n"); 
        return 1;
    }


    a = new Args[p];
    if(a == nullptr)
    {
        printf("Can not allocate memory for Args array\n");
        return 2;
    }

    A = new double[n1 * n2];
    if(A == nullptr)
    {
        printf("Can not allocate memory for matrix\n");
        return 3;
    }

    f = fopen(argv[4], "r");
    if(!f)
    {
        printf("Can't open file\n");
        delete[] A;
        delete[] a;
        return 4;
    }

    for(i = 0; i < n1; i++)
    {
        for(j = 0; j < n2; j++)
        {
            if(fscanf(f, "%lf", &A[i * n2 + j]) != 1)
            {
                printf("Trash in file %s\n", argv[4]);
                delete[] A;
                delete[] a;
                fclose(f);
                return 5;
            }
        }
    }
    
    fclose(f);

    pthread_barrier_init(&barrier, 0, p);

    for(i = 0; i < p; i++)
    {
        if(a[i].init(A, n1, n2, i, p, &barrier) != 0)
        {
            delete[] A;
            delete[] a;
            pthread_barrier_destroy(&barrier);
            return 6;
        }
    }

    double t = get_full_time();

    for(i = 1; i < p; i++)
    {
        if(pthread_create(&a[i].tid, nullptr, thread_func, a + i))
        {
            printf("Can not create thread %d\n", i);
            pthread_barrier_destroy(&barrier);
            delete[] A;
            delete[] a;
            return -3;
        }
    }

    a[0].tid = pthread_self();

    thread_func(a + 0);
    

    for(k = 1; k < p; k++)
        pthread_join(a[k].tid, nullptr);
        
    t = get_full_time() - t;

    
    a[0].print_matrix();
        
    get_variance(a);

    for(int i = 0; i < p; i++)
        printf("\nThread %2d time = %.12lf", i, a[i].cpu_time);
    printf("\ncpu_time variance = %.20lf\nTotal time = %.12lf\nTotal changed elements = %d\n", a[0].variance, t, a[0].changed);

    delete[] A;
    delete[] a;
    pthread_barrier_destroy(&barrier);
    return 0;
}