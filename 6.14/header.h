#ifndef HEADER_H
#define HEADER_H

#include <iostream>
#include <cmath>
#include <sys/time.h>
#include <sys/resource.h>
#include <threads.h>

const double eps = 1e-15;

enum class io_status
{
    undef,
    error_open,
    error_read,
    success
};
// 0 1 2 3 5 6 10 11 12 13
// 0 1 2 4 6 
// 0 1 4 5 7 8

class Args
{
    public:
    int p = 0;
    int k = 0;
    pthread_t tid = -1;
    pthread_barrier_t* barrier = nullptr;

    double* A = nullptr;
    double* arr1 = nullptr;
    double* arr2 = nullptr;
    double* arr3 = nullptr;
    double* arr4 = nullptr;
    int n1 = 0;
    int n2 = 0;
    int h = 0;
    int changed = 0;

    int count = 0;
    double sum = 0;
    double mean = 0;

    bool has_top = false;
    bool has_down = false;

    double cpu_time = 0;
    double full_time = 0;
    double variance = 0;

    io_status error_type = io_status::undef;
    int error_flag = 0;

    void print() const 
    {
        printf("p: %d, k: %d, n1: %d, n2: %d, h: %d\n changed: %d, count: %d, sum: %.2lf, mean: %.2lf, has_top: %d, has_down: %d\n cpu_time: %.2lf, full_time: %.2lf, variance: %.2lf\n\n", 
               p, k, n1, n2, h, changed, count, sum, mean, has_top, has_down, cpu_time, full_time, variance);
    }

    int init(double* A_, int n1_, int n2_, int k_, int p_, pthread_barrier_t* barrier_)
    {
        n1 = n1_;
        n2 = n2_;
        k = k_;
        p = p_;
        A = A_;
        barrier = barrier_;

        arr1 = new double[n2];
        arr2 = new double[n2];
        arr3 = new double[n2];
        arr4 = new double[n2];

        if(!arr1 || !arr2)
        {
            delete[] arr1;
            delete[] arr2;
            delete[] arr3;
            delete[] arr4;
            printf("Can't allocate memory in thread %d\n", k); 
            return 1;
        }

        return 0;
    }
    
    void print_matrix() const
    {
        for(int i = 0; i < n1; i++)
        {
            for(int j = 0; j < n2; j++)
                printf("%lf ", A[i * n2 + j]);
            printf("\n");
        }
    }

    ~Args()
    {
        delete[] arr1;
        delete[] arr2; 
        delete[] arr3; 
        delete[] arr4; 
    }
};

void* thread_func(void* arg);
void process_sequence(Args* a);
void change_inner(Args* a);
void get_variance(Args* a);
double get_full_time(void);
double get_cpu_time(void);
bool compare(double c1, double c2, double c3, double c4, double c5, double c6);

template <typename T>
int reduce_sum(int p, T* a = nullptr, int n = 0)
{
    static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    static pthread_cond_t c_in = PTHREAD_COND_INITIALIZER;
    static pthread_cond_t c_out = PTHREAD_COND_INITIALIZER;
    static int t_in = 0;
    static int t_out = 0;
    static T* r = nullptr;
    int i;

    if(p <= 1)  return 0;

    pthread_mutex_lock(&m);
    if(r == nullptr) 
        r = a;
    else
        for(i = 0; i < n; i++)
            r[i] += a[i];
    
    t_in++;

    if(t_in >= p)
    {
        t_out = 0;
        pthread_cond_broadcast(&c_in);
    }
    else    
        while(t_in < p)
            pthread_cond_wait(&c_in, &m);
    
    if(r != a)
        for(i = 0; i < n; i++)
            a[i] = r[i];
    
    t_out++;

    if(t_out >= p)
    {
        t_in = 0;
        r = nullptr;
        pthread_cond_broadcast(&c_out);
    }
    else
        while(t_out < p)
            pthread_cond_wait(&c_out, &m);
    
    pthread_mutex_unlock(&m);
    return 0;
}

#endif