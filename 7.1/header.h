#ifndef HEADER_H
#define HEADER_H

#include <iostream>
#include <cmath>
#include <sys/time.h>
#include <sys/resource.h>
#include <threads.h>

class Args
{
    public:
    unsigned long long int p = 0;
    unsigned long long int k = 0;
    pthread_t tid = -1;
    pthread_mutex_t* mutex = nullptr;

    unsigned long long int n = 0;
    unsigned long long int loc_cnt = 0;
    unsigned long long int chunk = 0;

    unsigned long long int step = 0;

    unsigned long long int res = 0;

    unsigned long long int start = 0;
    unsigned long long int end = 0;

    bool flag = false;

    double cpu_time = 0;
    double full_time = 0;
    double full_time1 = 0;
    double variance = 0;

    void print() const 
    {
        printf(
            "p = %llu, k = %llu, n = %llu, chunk = %llu, step = %llu,\n start = %llu, end = %llu,"
            " loc_cnt = %llu, res = %llu, time1 = %lf\n\n",
            p, k, n, chunk, step, start, end, loc_cnt, res, full_time1
        );
    }

};

void* thread_func(void* arg);

void get_variance(Args* a);
double get_full_time(void);
double get_cpu_time(void);
bool is_prime(unsigned long long int k);
unsigned long long int count_primes(unsigned long long int start, unsigned long long int end);


template <typename T>
int reduce_min(int p, T* a = nullptr, int n = 0)
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
            r[i] = std::min(r[i], a[i]);  
    
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

template <typename T>
int reduce_max(int p, T* a = nullptr, int n = 0)
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
            r[i] = std::max(r[i], a[i]);  
    
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
