#include "header.h"

int process_args(Args* a)
{
    if(a[0].error_flag == 0)
        for(int k = 1; k < a[0].p; k++)
        {
            if(a[k].error_flag == 0)
                a[0].changed += a[k].changed;
            else
            {
                printf("Error in thread %d: %d\n", k, int(a[k].error_type));
                //delete []a;
                return -4;
            }
        }
    else
    {
        printf("Error in thread %d: %d\n", 0, int(a[0].error_type));
        //delete []a;
        return -4;
    }
    return 0;
}

void get_variance(Args* a)
{
    double s1 = 0, s2 = 0;
    int p = a[0].p;
    
    for(int i = 0; i < p; i++)
    {
        s1 += a[i].cpu_time * a[i].cpu_time;
        s2 += a[i].cpu_time;
    }
    
    //printf("s1 = %.8lf, s2 = %.8lf\n", s1, s2);
    
    a[0].variance = s1/p - s2*s2/(p*p);
}

void* thread_func(void* arg)
{
    Args* a = (Args*)arg;

    int num = a->n / a->p;
    int dob = a->n % a->p;

    double c1, c2, c3;

    a->full_time = get_full_time();
    a->cpu_time = get_cpu_time();

    if(num == 0)
    {
        if(a->k < a->n)
        {
            a->len = 1;
            a->first = a->last = a->arr[a->k];

            if(a->k > 1)
            {
                a->has_2left = true;
                a->prelast_in_prev = a->arr[a->k - 2];
                a->last_in_prev = a->arr[a->k - 1];
            }
            else if(a->k == 1)
            {
                a->has_1left = true;
                a->last_in_prev = a->arr[a->k - 1];
            }

            if(a->k < a->n - 2)
            {
                a->has_2right = true;
                a->first_in_next = a->arr[a->k + 1];
                a->second_in_next = a->arr[a->k + 2];
            }
            else if(a->k == a->n - 2)
            {
                a->has_1right = true;
                a->first_in_next = a->arr[a->k + 1];
            }
        }
    }
    else
    {
        if(a->k < a->p - 1)
        {
            a->len = num;
        }
        else
            a->len = num + dob;
        
        if(a->len == 1)
        {
            a->first = a->last = a->arr[a->k * num];
        }
        else
        {
            a->first = a->arr[a->k * num];
            a->second = a->arr[a->k * num + 1];
            if(a->k != a->p - 1)
            {
                a->last = a->arr[(a->k + 1) * num - 1];
                a->prelast = a->arr[(a->k + 1) * num - 2];
            }
            else
            {
                a->last = a->arr[a->n - 1];
                a->prelast = a->arr[a->n - 2];
            }
        }

        if(a->k * num > 1)
        {
            a->has_2left = true;
            a->prelast_in_prev = a->arr[a->k * num - 2];
            a->last_in_prev = a->arr[a->k * num - 1];
        }
        else if(a->k * num == 1)
        {
            a->has_1left = true;
            a->last_in_prev = a->arr[a->k * num - 1];
        }

        if(a->k * num + a->len < a->n - 1)
        {
            a->has_2right = true;  
            a->first_in_next = a->arr[a->k * num + a->len];
            a->second_in_next = a->arr[a->k * num + a->len + 1];
        }
        else if(a->k * num + a->len == a->n - 1)
        {
            a->has_1right = true;
            a->first_in_next = a->arr[a->k * num + a->len];
        }
    }

    reduce_sum(a->p, &a->error_flag, 1);

    //a->print();

    if(num == 0)
        num = 1;
    
    if(a->has_2left && a->has_2right)
    {
        if(a->len == 1)
        {
            a->arr[a->k * num] = (a->prelast_in_prev + a->last_in_prev + a->first_in_next + a->second_in_next) / 4;
            a->changed++;
        }
        else if(a->len == 2)
        {
            c1 = a->last_in_prev;
            c2 = a->arr[a->k * num];

            a->arr[a->k * num] = (a->prelast_in_prev + a->last_in_prev + a->arr[a->k * num + 1] + a->first_in_next) / 4;
            a->arr[a->k * num + 1] = (a->last_in_prev + c2 + a->first_in_next + a->second_in_next) / 4;
            a->changed += 2;
        }
        else if(a->len >= 3)
        {
            c1 = a->prelast_in_prev;
            c2 = a->last_in_prev;
            c3 = a->arr[a->k * num];

            for(int i = 0; i < a->len - 2; i++)
            {
                a->arr[a->k * num + i] = (c1 + c2 + a->arr[a->k * num + i + 1] + a->arr[a->k * num + i + 2]) / 4;
                a->changed++;

                c1 = c2;
                c2 = c3;
                c3 = a->arr[a->k * num + i + 1];
            }

            a->arr[a->k * num + a->len - 2] = (c1 + c2 + a->last + a->first_in_next) / 4;

            a->arr[a->k * num + a->len - 1] = (c2 + c3 + a->first_in_next + a->second_in_next) / 4;

            a->changed += 2;
        }
    }
    if(a->has_2left && a->has_1right)
    {
        if(a->len == 2)
        {
            a->arr[a->k * num] = (a->prelast_in_prev + a->last_in_prev + a->last + a->first_in_next) / 4;
        }
        else if(a->len > 2)
        {
            c1 = a->prelast_in_prev;
            c2 = a->last_in_prev;
            c3 = a->arr[a->k * num];

            for(int i = 0; i < a->len - 2; i++)
            {
                a->arr[a->k * num + i] = (c1 + c2 + a->arr[a->k * num + i + 1] + a->arr[a->k * num + i + 2]) / 4;
                a->changed++;

                c1 = c2;
                c2 = c3;
                c3 = a->arr[a->k * num + i + 1];
            }

            a->arr[a->k * num + a->len - 2] = (c1 + c2 + a->last + a->first_in_next) / 4;
        }

    }
    if(a->has_2left && !a->has_1right && !a->has_2right)
    {
        if(a->len > 2)
        {
            c1 = a->prelast_in_prev;
            c2 = a->last_in_prev;
            c3 = a->arr[a->k * num];

            for(int i = 0; i < a->len - 2; i++)
            {
                a->arr[a->k * num + i] = (c1 + c2 + a->arr[a->k * num + i + 1] + a->arr[a->k * num + i + 2]) / 4;
                a->changed++;

                c1 = c2;
                c2 = c3;
                c3 = a->arr[a->k * num + i + 1];
            }
        }
    }
    if(a->has_1left && a->has_2right)
    {
        if(a->len == 2)
        {
            a->arr[a->k * num + a->len - 1] = (a->last_in_prev + a->arr[a->k * num] + a->first_in_next + a->second_in_next) / 4;
            a->changed++;
        }
        else if(a->len > 2)
        {
            c1 = a->last_in_prev;
            c2 = a->arr[a->k * num];
            c3 = a->arr[a->k * num + 1];

            for(int i = 1; i < a->len - 2; i++)
            {
                a->arr[a->k * num + i] = (c1 + c2 + a->arr[a->k * num + i + 1] + a->arr[a->k * num + i + 2]) / 4;
                a->changed++;

                c1 = c2;
                c2 = c3;
                c3 = a->arr[a->k * num + i + 1];
            }

            a->arr[a->k * num + a->len - 2] = (c1 + c2 + a->last + a->first_in_next) / 4;

            a->arr[a->k * num + a->len - 1] = (c2 + c3 + a->first_in_next + a->second_in_next) / 4;

            a->changed += 2;
        }
    }
    if(a->has_1left && a->has_1right)
    {
        if(a->len > 2)
        {
            c1 = a->last_in_prev;
            c2 = a->arr[a->k * num];
            c3 = a->arr[a->k * num + 1];

            for(int i = 1; i < a->len - 2; i++)
            {
                a->arr[a->k * num + i] = (c1 + c2 + a->arr[a->k * num + i + 1] + a->arr[a->k * num + i + 2]) / 4;
                a->changed++;

                c1 = c2;
                c2 = c3;
                c3 = a->arr[a->k * num + i + 1];
            }

            a->arr[a->k * num + a->len - 2] = (c1 + c2 + a->last + a->first_in_next) / 4;

            a->changed++;
        }
    }
    if(a->has_1left && !a->has_1right && !a->has_2right)
    {
        if(a->len > 3)
        {
            c1 = a->last_in_prev;
            c2 = a->arr[a->k * num];
            c3 = a->arr[a->k * num + 1];

            for(int i = 1; i < a->len - 2; i++)
            {
                a->arr[a->k * num + i] = (c1 + c2 + a->arr[a->k * num + i + 1] + a->arr[a->k * num + i + 2]) / 4;
                a->changed++;

                c1 = c2;
                c2 = c3;
                c3 = a->arr[a->k * num + i + 1];
            }
        }
    }
    if(!a->has_1left && !a->has_2left && a->has_2right)
    {
        if(a->len == 3)
        {
            a->arr[a->k * num + 2] = (a->arr[a->k * num] + a->arr[a->k * num + 1] + a->first_in_next + a->second_in_next) / 4;
            a->changed++;
        }
        if(a->len > 3)
        {
            c1 = a->arr[a->k * num];
            c2 = a->arr[a->k * num + 1];
            c3 = a->arr[a->k * num + 2];

            for(int i = 2; i < a->len - 2; i++)
            {
                a->arr[a->k * num + i] = (c1 + c2 + a->arr[a->k * num + i + 1] + a->arr[a->k * num + i + 2]) / 4;
                a->changed++;

                c1 = c2;
                c2 = c3;
                c3 = a->arr[a->k * num + i + 1];
            }

            a->arr[a->k * num + a->len - 2] = (c1 + c2 + a->last + a->first_in_next) / 4;

            a->arr[a->k * num + a->len - 1] = (c2 + c3 + a->first_in_next + a->second_in_next) / 4;

            a->changed += 2;
        }
    }
    if(!a->has_1left && !a->has_2left && a->has_1right)
    {
        if(a->len > 3)
        {
            c1 = a->arr[a->k * num];
            c2 = a->arr[a->k * num + 1];
            c3 = a->arr[a->k * num + 2];

            for(int i = 2; i < a->len - 2; i++)
            {
                a->arr[a->k * num + i] = (c1 + c2 + a->arr[a->k * num + i + 1] + a->arr[a->k * num + i + 2]) / 4;
                a->changed++;

                c1 = c2;
                c2 = c3;
                c3 = a->arr[a->k * num + i + 1];
            }

            a->arr[a->k * num + a->len - 2] = (c1 + c2 + a->last + a->first_in_next) / 4;

            a->changed++;
        }
    }
    if(!a->has_1left && !a->has_2left && !a->has_1right && !a->has_2right)
    {
        if(a->len > 4)
        {
            c1 = a->arr[a->k * num];
            c2 = a->arr[a->k * num + 1];
            c3 = a->arr[a->k * num + 2];

            for(int i = 2; i < a->len - 2; i++)
            {
                a->arr[a->k * num + i] = (c1 + c2 + a->arr[a->k * num + i + 1] + a->arr[a->k * num + i + 2]) / 4;
                a->changed++;

                c1 = c2;
                c2 = c3;
                c3 = a->arr[a->k * num + i + 1];
            }
        }
    }

    a->full_time = get_full_time() - a->full_time;
    a->cpu_time = get_cpu_time() - a->cpu_time;
    
    //a->print_array();
    //a->print();

    return nullptr;
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


