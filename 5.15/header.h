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

    double* arr = nullptr;
    int n = 0;
    int changed = 0;

    int leftlen = 0;
    int len = 0;
    int rightlen = 0;

    double to_change_left = 0;
    double to_change_full = 0;
    double to_change_right = 0;
    double leftlast = 0;
    double rightfirst = 0;
    double dleft = 0;
    double dfull = 0;
    double dright = 0;
    double leftsum = 0;
    double rightsum = 0;
    double sum = 0;
    bool progression_left = false;
    bool progression_right = false;
    bool progression_full = false;

    bool has_left = false;
    bool has_right = false;
    double last_in_prev = 0;
    double first = 0;
    double last = 0;
    double first_in_next = 0;

    double cpu_time = 0;
    double full_time = 0;
    double variance = 0;

    io_status error_type = io_status::undef;
    int error_flag = 0;

    void print() const
    {
        printf(
            "k: %d, p: %d, tid: %ld, n: %d, changed: %d,\n"
            "len: %d, leftlen: %d, rightlen: %d,\n"
            "to_change_left: %.2lf, to_change_full: %.2lf, to_change_right: %.2lf,\n"
            "leftlast: %.2lf, rightfirst: %.2lf,\n"
            "dleft: %.2lf, dfull: %.2lf, dright: %.2lf,\n"
            "leftsum: %.2lf, rightsum: %.2lf, sum: %.2lf,\n"
            "progression_left: %d, progression_right: %d, progression_full: %d,\n"
            "has_left: %d, has_right: %d,\n"
            "last_in_prev: %.2lf, first: %.2lf, last: %.2lf, first_in_next: %.2lf,\n"
            "cpu_time: %.8lf, full_time: %.8lf, variance: %.8lf,\n\n",
            k, p, tid, n, changed,
            len, leftlen, rightlen,
            to_change_left, to_change_full, to_change_right,
            leftlast, rightfirst,
            dleft, dfull, dright,
            leftsum, rightsum, sum,
            progression_left, progression_right, progression_full,
            has_left, has_right,
            last_in_prev, first, last, first_in_next,
            cpu_time, full_time, variance
        );
    }
    
    void print_array() const
    {
        for(int i = 0; i < n; i++)
            printf("%lf ", arr[i]);
        printf("\n\n");
    }
};

void* thread_func(void* arg);
int process_args(Args* a);
void get_variance(Args* a);
void pthread_berrier_wait(Args* a);
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

inline void pthread_berrier_wait(Args* a)
{
    double c1, c2, d, mean;
    int i, j, cnt, cnt1 = 0, ind;
    bool flag = false;
    if(a->k == 0)
    {
        if(a->n > 1)
        {
            d = a->arr[1] - a->arr[0];
            ind = 0;
            c1 = a->arr[0];
            cnt = 2;
            flag = true;
        
            for(i = 1; i < a->n - 1; i++)
            {
                if(std::abs(a->arr[i + 1] - a->arr[i] - d) < eps)
                {
                    flag = true;
                    cnt++;
                }
                else
                {
                    cnt1++;
                    flag = false;
                    c2 = a->arr[i];
                    mean = (c1 + c2) / 2;

                    for(j = 0; j < cnt; j++)
                    {
                        a->arr[ind + j] = mean;
                        a->changed++;
                    }
                        
                    if(i + 2 < a->n - 1)
                    {
                        d = a->arr[i + 2] - a->arr[i + 1];
                        c1 = a->arr[i + 1];
                        cnt = 2;
                        ind = i + 1;
                        i++;
                    }
                    else if(i+2 == a->n-1)
                    {
                        mean = (a->arr[a->n-1] + a->arr[a->n-2]) / 2;
                        a->arr[a->n-1] = a->arr[a->n-2] = mean;
                        a->changed += 2;
                        break;
                    }
                }
                
            }
            
            if(flag)
            {
                c2 = a->arr[a->n-1];
                mean = (c1+c2)/2;
                
                for(i = ind; i < a->n; i++)
                {
                    a->arr[i] = mean;
                    a->changed++;
                }
            }
        }
    }

    pthread_barrier_wait(a->barrier);
}

#endif