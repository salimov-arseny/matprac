#ifndef HEADER_H
#define HEADER_H

#include <iostream>
#include <cmath>
#include <threads.h>

const double eps = 1e-15;

extern pthread_barrier_t barrier;

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
    const char *name = nullptr;
    pthread_t tid = -1;
    int count = 0;
    int num_of_extr = 0;
    int res = 0;
    bool has_left = false;
    bool has_right = false;

    double last_in_prev = 0;
    double first = 0;
    double second = 0;
    double prelast = 0;
    double last = 0;
    double first_in_next = 0;

    double mean = 0;
    double sum_extr = 0;
    io_status error_type = io_status::undef;
    int error_flag = 0;
};

void* thread_func(void* arg);
bool is_maximum(double c1, double c2, double c3);
io_status process_subsequence(FILE* f, double* first, double* second, double* prelast, double* last, double* sum, int* num_extr, int* n);
io_status final(FILE* f, int* ans, double* mean);
int process_args(Args* a);

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