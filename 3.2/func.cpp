#include "header.h"

bool is_maximum(double c1, double c2, double c3)
{
    bool p1 = c1 < c2 && c2 > c3;
    bool p2 = c1 < c2 && std::fabs(c2 - c3) < eps;
    bool p3 = std::fabs(c2 - c3) < eps && c2 > c3;
    bool p4 = std::fabs(c1 - c2) < eps && std::fabs(c2 - c3) < eps;
    
    return p1 || p2 || p3 || p4;
}

int process_args(Args* a)
{
    if(a[0].error_flag == 0)
        for(int k = 1; k < a[0].p; k++)
        {
            if(a[k].error_flag == 0)
                a[0].res += a[k].res;
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

io_status process_subsequence(FILE* f, double* first, double* second, double* prelast, double* last, double* sum, int* num_extr, int* n)
{
    double c1, c2, c3;
    double s = 0, prelst, lst;

    int code1, code2;
    int cnt = 0, cnt1 = 0;

    code1 = fscanf(f, "%lf", &c1);
    code2 = fscanf(f, "%lf", &c2);

    if(code1 == -1)
    {
        return io_status::success;
    }
    else if(code1 == 0)
    {
        return io_status::error_read;
    }

    *first = c1;
    cnt++;

    if(code2 == -1)
    {
        *first = c1;
        *last = c1;
        *n = 1;
        return io_status::success;
    }
    else if(code2 == 0)
    {
        return io_status::error_read;
    }

    *second = c2;
    cnt++;

    prelst = c1;
    lst = c2;

    while(fscanf(f, "%lf", &c3) == 1)
    {
        cnt++;
        if(is_maximum(c1, c2, c3))
        {
            s += c2;
            cnt1++;
        }
        prelst = c2;
        lst = c3;

        c1 = c2;
        c2 = c3;
    }

    if(fscanf(f, "%lf", &c3) != EOF)
        return io_status::error_read;

    *prelast = prelst;
    *last = lst;
    *sum = s;
    *num_extr = cnt1;
    *n = cnt;

    return io_status::success;
}

io_status final(FILE* f, int* ans, double* mean)
{
    int cnt = 0;
    double tmp;
    double m = *mean;

    while(fscanf(f, "%lf", &tmp) == 1)
    {
        //printf("%lf < %lf : %d\n", tmp, *min, cnt);
        if(tmp > m)
            cnt++;
    }

    if(fscanf(f, "%lf", &tmp) != EOF)
        return io_status::error_read;
        
    //printf("im ok\n");
    *ans = cnt;

    return io_status::success;
}


void* thread_func(void* arg)
{
    Args* a = (Args*)arg;

    int i;

    double c1, c2, c3;

    FILE* f = fopen(a->name, "r");
    if(!f)
    {
        a->error_type = io_status::error_open;
        a->error_flag = 1;
    }
    else
        a->error_flag = 0;
    
    reduce_sum(a->p, &a->error_flag, 1);

    if(a->error_flag > 0)
    {
        if(f)
            fclose(f);
        return nullptr;
    }

    a->error_type = process_subsequence(f, &a->first, &a->second, &a->prelast, &a->last, &a->sum_extr, &a->num_of_extr, &a->count);
    fclose(f);
    f = nullptr;

    if(a->error_type != io_status::success)
        a->error_flag = 1;
    else
        a->error_flag = 0;

    reduce_sum(a->p, &a->error_flag, 1);

    if(a->error_flag > 0)
    {
        if(a->error_type != io_status::success)
            a->error_flag = 1;
        else
            a->error_flag = 0;
        if(f)
            fclose(f);
        return nullptr;
    }

    pthread_barrier_wait(&barrier);

    if(a->count > 0)
    {
        if(a->k > 0)
        {
            for(i = 1; i < a->k + 1; i++)
                if((a-i)->count != 0)
                {
                    a->has_left = true;
                    break;
                }
            a->last_in_prev = (a-i)->last;
        }

        if(a->k < a->p - 1)
        {
            for(i = 1; i < a->p - a->k; i++)
                if((a+i)->count != 0)
                {
                    a->has_right = true;
                    break;
                }
            a->first_in_next = (a+i)->first;
        }
    }  
    //printf("k = %d, has_left = %d, has_right = %d, count = %d, %lf %lf %lf %lf %lf %lf\n", a->k, a->has_left, a->has_right, 
    //a->count, a->last_in_prev, a->first, a->second, a->prelast, a->last, a->first_in_next);

    pthread_barrier_wait(&barrier);

    if(a->count == 1 && a->has_left && a->has_right)
    {
        c1 = a->last_in_prev;
        c2 = a->first;
        c3 = a->first_in_next;
        if(is_maximum(c1, c2, c3))
        {
            a->sum_extr += c2;
            a->num_of_extr++;
        }
    }

    if(a->count > 1)
    {
        if(a->has_left)
        {
            c1 = a->last_in_prev;
            c2 = a->first;
            c3 = a->second;
            if(is_maximum(c1, c2, c3))
            {
                a->sum_extr += c2;
                a->num_of_extr++;
            }
            //printf("has left: %lf %lf %lf\n", c1, c2, c3);
        }
        
        if(a->has_right)
        {
            c1 = a->prelast;
            c2 = a->last;
            c3 = a->first_in_next;
            if(is_maximum(c1, c2, c3))
            {
                a->sum_extr += c2;
                a->num_of_extr++;
            }
            //printf("%d has right: %lf %lf %lf\n", a->k, c1, c2, c3);
        }
    }
    
    //printf("k = %d, sum = %lf, count = %d, num_extr = %d\n", a->k, a->sum_extr, a->count, a->num_of_extr);

    reduce_sum(a->p, &a->sum_extr, 1);
    reduce_sum(a->p, &a->num_of_extr, 1);
    
    if(a->num_of_extr == 0)
    {
        a->error_type = io_status::success;
        return nullptr;
    }
    
    f = fopen(a->name, "r");

    if(!f)
    {
        a->error_type = io_status::error_open;
        a->error_flag = 1;
    }
    else
        a->error_flag = 0;
    
    reduce_sum(a->p, &a->error_flag, 1);

    if(a->error_flag > 0)
    {
        if(f)
            fclose(f);
        return nullptr;
    }

    a->mean = a->sum_extr/a->num_of_extr;

    a->error_type = final(f, &a->res, &a->mean);

    if(a->error_type != io_status::success)
        a->error_flag = 1;
    else
        a->error_flag = 0;

    reduce_sum(a->p, &a->error_flag, 1);

    if(a->error_flag > 0)
    {
        if(a->error_type != io_status::success)
            a->error_flag = 1;
        else
            a->error_flag = 0;
        if(f)
            fclose(f);
        return nullptr;
    }

    fclose(f);
    a->error_type = io_status::success;
    return nullptr;
}


