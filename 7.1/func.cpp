#include "header.h"

static unsigned long long int global_cnt = 0;

static unsigned long long global_start = 1;

void get_variance(Args* a)
{
    double s1 = 0, s2 = 0;
    int p = a[0].p;
    
    for(int i = 0; i < p; i++)
    {
        s1 += a[i].cpu_time * a[i].cpu_time;
        s2 += a[i].cpu_time;
    }
    
    a[0].variance = s1/p - s2*s2/(p*p);
}



double get_full_time() 
{
    struct timeval buf;
    gettimeofday(&buf, 0);
    return buf.tv_sec + buf.tv_usec/1e6;
}
double get_cpu_time() 
{
    struct rusage buf;
    getrusage(RUSAGE_THREAD, &buf);
    return buf.ru_utime.tv_sec + buf.ru_utime.tv_usec/1e6;
}


unsigned long long int count_primes(unsigned long long int start, unsigned long long int end)
{
    unsigned long long int i;

    unsigned long long int cnt = 0;

    if(2 > start)
    {
        cnt++;
        start = 3;
    }

    if(start % 2 == 0)
        start++;

    for(i = start; i < end; i += 2)
        if(is_prime(i))
            cnt++;

    return cnt;
}


inline bool is_prime(unsigned long long int k)
{
    unsigned long long int i;

    if(k < 2)
        return false;
    if(k == 2 || k == 3)
        return true;
    if(k % 2 == 0 || k % 3 == 0)
        return false;
    for(i = 5; i * i <= k; i += 6)
        if(k % i == 0 || k % (i + 2) == 0)
            return false;
    
    return true;
}


void* thread_func(void* arg)
{
    Args* a = (Args*)arg;

    unsigned long long int n = a->n;
    unsigned long long int p = a->p;
    unsigned long long int k = a->k;
    unsigned long long int chunk = a->chunk;

    unsigned long long int cnt = 0;

    unsigned long long int i;

    unsigned long long int j;

    a->full_time = get_full_time();
    a->full_time1 = get_full_time();
    a->cpu_time = get_cpu_time();
    
    while(global_cnt < n)
    {
        a->step += 1;
        pthread_mutex_lock(a->mutex);

        a->start = global_start % 2 == 0 ? global_start + 1 : global_start;

        a->end = global_start + chunk;

        global_start += chunk;

        pthread_mutex_unlock(a->mutex);

        if(2 > a->start)
            cnt++;

        for(i = a->start; i < a->end; i += 2)
        {
            if(is_prime(i))
                cnt++;
        }

        a->loc_cnt = cnt;

        pthread_mutex_lock(a->mutex);

        if(a->flag)
        {
            global_cnt += cnt;
            a->flag = false;
            pthread_mutex_unlock(a->mutex);
            break;
        }

        global_cnt += cnt;

        if(global_cnt >= n)
        {
            //printf("ended up on thread %llu\n", k);
            
            for(j = 1; j <= k; j++)
                (a-j)->flag = true;
            
            for(j = 1; j < p - k; j++)
                (a+j)->flag = true;
            
            pthread_mutex_unlock(a->mutex);
            break;
        }   

        pthread_mutex_unlock(a->mutex);

        cnt = 0;
    }

    a->full_time1 = get_full_time() - a->full_time1;

    reduce_min(p, &a->start, 1);
    reduce_max(p, &a->end, 1);
    

    //if(k==0)
      //  printf("start = %llu, end = %llu\n", a->start, a->end);

    //a->print();

    //reduce_sum(p);

    if(k == 0)
    {
        global_cnt -= count_primes(a->start, a->end);

        //printf("global_cnt = %llu\n", global_cnt);

        for(i = a->start; i < a->end; i++)
        {
            if(is_prime(i))
                global_cnt++;

            if(global_cnt == n)
            {
                a->res = i;
                break;
            }
        }
    }
    
    /*if(n == 500)
        a->res = 3571;
    else if(n == 10000000)
        a->res = 179424673;*/

    a->full_time = get_full_time() - a->full_time;
    a->cpu_time = get_cpu_time() - a->cpu_time;

    return nullptr;
}
