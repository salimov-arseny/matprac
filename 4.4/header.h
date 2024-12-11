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

class Args
{
    public:
    int p = 0;
    int k = 0;
    pthread_t tid = -1;

    double* arr = nullptr;
    int n = 0;
    int changed = 0;
    int len = 0;

    bool has_2left = false;
    bool has_1left = false;
    bool has_1right = false;
    bool has_2right = false;
    double prelast_in_prev = 0;
    double last_in_prev = 0;
    double first = 0;
    double second = 0;
    double prelast = 0;
    double last = 0;
    double first_in_next = 0;
    double second_in_next = 0;

    double cpu_time = 0;
    double full_time = 0;
    double variance = 0;

    io_status error_type = io_status::undef;
    int error_flag = 0;

    void print() const
    {
        printf(
            "k: %d, changed: %d, len: %d, "
            "has_2left: %d, has_1left: %d, has_1right: %d, has_2right: %d, "
            "prelast_in_prev: %.2lf, last_in_prev: %.2lf, first: %.2lf, second: %.2lf, "
            "prelast: %.2lf, last: %.2lf, first_in_next: %.2f, second_in_next: %.2lf, "
            "cpu_time = %.8lf, full_time = %.8lf\n\n",
            k, changed, len,
            has_2left, has_1left, has_1right, has_2right,
            prelast_in_prev, last_in_prev, first, second,
            prelast, last, first_in_next, second_in_next,
            cpu_time, full_time
        );
    }
    void print_array() const
    {
        for(int i = 0; i < n; i++)
            printf("%lf ", arr[i]);
        printf("\n");
    }
};

void* thread_func(void* arg);
int process_args(Args* a);
void get_variance(Args* a);
double get_full_time(void);
double get_cpu_time(void);

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