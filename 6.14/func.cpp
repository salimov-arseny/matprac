#include "header.h"

int process_args(Args* a)
{
    if(a[0].error_flag == 0)
        for(int k = 1; k < a[0].p; k++)
        {
            if(!(a[k].error_flag == 0))
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


bool compare(double c1, double c2, double c3, double c4, double c5, double c6)
{
    return (6 * c1 > c2 + c3 + c4 + c5 + c6) || std::abs(6 * c1 - c2 - c3 - c4 - c5 - c6) < eps;
}


void process_sequence(Args* a)
{
    double* A = a->A;
    int n1 = a->n1;
    int n2 = a->n2;
    int k = a->k;
    int h = a->h;

    int num = n1 / a->p;

    double c1, c2, c3, c4, c5, c6, s = 0;

    int i, j, cnt = 0;

    if(num > 0)
    {
        for(i = 0; i < h; i++)
        {
            for(j = 0; j < n2 - 1; j++)
            {
                if((k * num + i - 1) >= 0 && (k * num + i + 1) < n1)
                {
                    c1 = A[(k * num + i) * n2 + j];
                    c2 = A[(k * num + i + 1) * n2 + j];
                    c3 = A[(k * num + i - 1) * n2 + j];
                    c4 = A[(k * num + i + 1) * n2 + j + 1];
                    c5 = A[(k * num + i - 1) * n2 + j + 1];
                    c6 = A[(k * num + i) * n2 + j + 1];
                    
                    if(compare(c1, c2, c3, c4, c5, c6))
                    {
                        s += c1;
                        cnt++;
                    }
                }
            }
        }
    }
    else if(k < n1 - 1 && k > 0)
    {
        for(j = 0; j < n2 - 1; j++)
        {
            c1 = A[k * n2 + j];
            c2 = A[(k + 1) * n2 + j];
            c3 = A[(k - 1) * n2 + j];
            c4 = A[(k + 1) * n2 + j + 1];
            c5 = A[(k - 1) * n2 + j + 1];
            c6 = A[k * n2 + j + 1];
            
            if(compare(c1, c2, c3, c4, c5, c6))
            {
                s += c1;
                cnt++;
            }
        }
    }

    a->count = cnt;
    a->sum = s;
}

void change_inner(Args* a)
{
    double* A = a->A;
    int n1 = a->n1;
    int n2 = a->n2;
    int k = a->k;
    int h = a->h;

    int num = n1 / a->p;

    double c1, c2, c3, c4, c5, c6;

    int i, j, q;

    if(h == 1)
    {
        for(i = 0; i < n2; i++)
            a->arr1[i] = a->arr4[i] = A[k * n2 + i];
    }
    else if(h == 2)
    {
        for(i = 0; i < n2; i++)
        {
            a->arr1[i] = a->arr3[i] = A[(k * num) * n2 + i];
            a->arr4[i] = a->arr2[i] =  A[(k * num + 1) * n2 + i];
        }
    }
    else if(h == 3)
    {
        for(i = 0; i < n2; i++)
        {
            a->arr1[i] = A[(k * num + 0) * n2 + i];
            a->arr2[i] = a->arr3[i] = A[(k * num + 1) * n2 + i];
            a->arr4[i] = A[(k * num + 2) * n2 + i];
        }

        for(j = 0; j < n2 - 1; j++)
        {
            c1 = A[(k * num + 1) * n2 + j];
            c2 = A[(k * num + 2) * n2 + j];
            c3 = A[(k * num + 0) * n2 + j];
            c4 = A[(k * num + 2) * n2 + j + 1];
            c5 = A[(k * num + 0) * n2 + j + 1];
            c6 = A[(k * num + 1) * n2 + j + 1];
            
            if(compare(c1, c2, c3, c4, c5, c6))
            {
                A[(k * num + 1) * n2 + j] = a->mean;
                a->changed++;
            }
            
        }
    }
    else if(h > 3)
    {
        for(i = 0; i < n2; i++)
        {
            a->arr1[i] = A[(k * num + 0) * n2 + i];
            a->arr2[i] = A[(k * num + 1) * n2 + i];
            a->arr3[i] = A[(k * num + h - 2) * n2 + i];
            //a->arr4[i] = A[(k * num + h - 1) * n2 + i];
        }

        for(i = 1; i < h - 1; i++)
        {
            for(q = 0; q < n2; q++)
            {
                a->arr4[q] = A[(k * num + i) * n2 + q];
            }

            for(j = 0; j < n2 - 1; j++)
            {
                c1 = A[(k * num + i) * n2 + j];
                c2 = A[(k * num + i + 1) * n2 + j];
                c3 = a->arr1[j];
                c4 = A[(k * num + i + 1) * n2 + j + 1];
                c5 = a->arr1[j + 1];
                c6 = A[(k * num + i) * n2 + j + 1];
                
                if(compare(c1, c2, c3, c4, c5, c6))
                {
                    a->changed++;
                    A[(k * num + i) * n2 + j] = a->mean;
                }
            }

            for(q = 0; q < n2; q++)
            {
                a->arr1[q] = a->arr4[q];
            }
        }

        for(i = 0; i < n2; i++)
        {
            a->arr1[i] = A[(k * num + 0) * n2 + i];
            a->arr4[i] = A[(k * num + h - 1) * n2 + i];
        }
    }
}


void* thread_func(void* arg)
{
    Args* a = (Args*)arg;

    double* A = a->A;

    int n1 = a->n1;
    int n2 = a->n2;
    int k = a->k;
    int p = a->p;
    int num = n1 / p;
    int dob = n1 % p;

    int j;

    double c1, c2, c3, c4, c5, c6;

    a->full_time = get_full_time();
    a->cpu_time = get_cpu_time();

    if(num == 0 && k < n1)
    {
        a->h = 1;
        if(k > 0)
            a->has_top = true;

        if(k < n1 - 1)
            a->has_down = true;

    }
    else if(num > 0)
    {
        if(k < p - 1)
            a->h = num;
        else
            a->h = num + dob;
        
        if(k * num > 0)
            a->has_top = true;


        if(k * num + a->h < n1)
            a->has_down = true;
    }

    process_sequence(a);

    //a->print(); 

    reduce_sum(p, &a->sum, 1);
    reduce_sum(p, &a->count, 1);

    if(a->count > 0)
        a->mean = a->sum / a->count;
    else
    {
        a->full_time = get_full_time() - a->full_time;
        a->cpu_time = get_cpu_time() - a->cpu_time;
        return nullptr;
    }

    change_inner(a);

    //a->print(); 

    /*if(k == 0)
    {
        printf("\n\n");
        a->print_matrix();
        printf("\n\n");
    }*/


    pthread_barrier_wait(a->barrier);

    //if(k == 0)
      //  a->print_matrix();


    if(a->h == 1)
    {
        if(a->has_top && a->has_down)
        {
            for(j = 0; j < n2 - 1; j++)
            {
                c1 = A[k * n2 + j];
                c2 = (a+1)->arr1[j];
                c3 = (a-1)->arr4[j];
                c4 = (a+1)->arr1[j + 1];
                c5 = (a-1)->arr4[j + 1];
                c6 = A[k * n2 + j + 1];
                
                if(compare(c1, c2, c3, c4, c5, c6))
                {
                    A[k * n2 + j] = a->mean;
                    a->changed++;
                } 
            }
        }
    }
    else if(a->h > 1)
    {
        if(a->has_top)
        {
            for(j = 0; j < n2 - 1; j++)
            {
                c1 = A[(k * num) * n2 + j];
                c2 = a->arr2[j];
                c3 = (a-1)->arr4[j];
                c4 = a->arr2[j + 1];
                c5 = (a-1)->arr4[j +1];
                c6 = A[(k * num) * n2 + j + 1];
                
                if(compare(c1, c2, c3, c4, c5, c6))
                {
                    A[(k * num) * n2 + j] = a->mean;
                    a->changed++;
                }
            }
        }
        if(a->has_down)
        {
            for(j = 0; j < n2 - 1; j++)
            {
                c1 = A[(k * num + a->h - 1) * n2 + j];
                c2 = (a+1)->arr1[j];
                c3 = a->arr3[j];
                c4 = (a+1)->arr1[j + 1];
                c5 = a->arr3[j + 1];
                c6 = A[(k * num + a->h - 1) * n2 + j + 1];
                
                if(compare(c1, c2, c3, c4, c5, c6))
                {
                    A[(k * num + a->h - 1) * n2 + j] = a->mean;
                    a->changed++;
                }
            }
        }
    }

    reduce_sum(p, &a->changed, 1);


    a->full_time = get_full_time() - a->full_time;
    a->cpu_time = get_cpu_time() - a->cpu_time;

    return nullptr;
}

