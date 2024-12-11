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


void process_sequence(Args* a, int num, int dob)
{
    double* arr = a->arr;

    double d, c1, c2, mean;

    int n = a->n;
    int k = a->k;
    int p = a->p;

    int i, j, cnt = 0, cnt1 = 0, ind;

    bool flag = false;

    if(num == 0 && k < n)
    {
        a->len = 1;
        a->first = a->last = arr[k];
        if(k > 0)
        {
            a->has_left = true;
            a->last_in_prev = arr[k - 1];
        }
        if(k < n - 1)
        {
            a->has_right = true;
            a->first_in_next = arr[k + 1];
        }
    }
    else if(num > 0)
    {
        if(k < p - 1)
            a->len = num;
        else
            a->len = num + dob;
        
        if(a->len == 1)
            a->first = a->last = arr[k * num];
        else
        {
            a->first = arr[k * num];
            if(k != p - 1)
                a->last = arr[(k + 1) * num - 1];
            else
                a->last = arr[n - 1];
        }

        if(k * num > 0)
        {
            a->has_left = true;
            a->last_in_prev = arr[k * num - 1];
        }

        if(k * num + a->len < n)
        {
            a->has_right = true;
            a->first_in_next = arr[k * num + a->len];
        }
    }


    if(a->len > 1)
    {
        d = arr[k * num + 1] - arr[k * num];
        ind = 0;
        c1 = arr[k * num];
        cnt = 2;
        cnt1++;
        flag = true;
        (void)mean;
        (void)j;
        (void)ind;  

        for(i = 1; i < a->len - 1; i++)
        {
            if(std::abs(arr[k * num + i + 1] - arr[k * num + i] - d) < eps)
            {
                flag = true;
                cnt++;
            }
            else
            {
                flag = false;
                if(cnt1 == 1)
                {
                    a->progression_left = true;
                    c2 = arr[k * num + i];
                    a->leftsum = cnt * (c1 + c2) / 2;
                    a->leftlen = cnt;

                    a->dleft = d;
                    a->leftlast = c2;

                    /*if(!a->has_left)
                    {
                        c2 = arr[k * num + i];
                        mean = (c1 + c2) / 2;

                        for(j = 0; j < cnt; j++)
                            arr[k * num + ind + j] = mean;
                    }*/
                }
                if (i < a->len - 2 && cnt1 > 1)
                {
                    c2 = arr[k * num + i];
                    mean = (c1 + c2) / 2;

                    //for(j = 0; j < cnt; j++)
                      //  arr[k * num + ind + j] = mean;
                }

                if(i + 2 < a->len - 1)
                {
                    ind = i + 1;
                    c1 = arr[k * num + i + 1];
                    d = arr[k * num + i + 2] - arr[k * num + i + 1];
                    cnt = 2;
                    i++;
                }
                else if(i + 2 == a->len - 1)
                {
                    /*if(cnt1 > 1)
                    {
                        c2 = arr[k * num + i];
                        mean = (c1 + c2) / 2;

                        for(j = 0; j < cnt; j++)
                            arr[k * num + ind + j] = mean;
                    }*/
                    a->progression_right = true;
                    a->rightlen = 2;
                    a->rightsum = arr[k * num + i + 1] + arr[k * num + i + 2];
                    a->rightfirst = arr[k * num + i + 1];
                    a->dright = arr[k * num + i + 2] - arr[k * num + i + 1];

                    /*if(!a->has_right)
                    {
                        c1 = arr[k * num + i + 1];
                        c2 = arr[k * num + i + 2];
                        mean = (c1 + c2) / 2;
                        arr[k * num + i + 1] = arr[k * num + i + 2] = mean;
                    }*/
                    break;
                }
                else if(i + 2 > a->len - 1)
                {
                    if(cnt1 > 1)
                    {
                        c2 = arr[k * num + i];
                        mean = (c1 + c2) / 2;

                        //for(j = 0; j < cnt; j++)
                          //  arr[k * num + ind + j] = mean;
                    }
                    a->progression_right = true;
                    a->rightlen = 1;
                    a->rightsum = arr[k * num + i + 1];
                    a->rightfirst = arr[k * num + i + 1];
                    break;
                }

                cnt1++;
            }
        }

        if(flag && cnt1 > 1)
        {
            a->progression_right = true;
            c2 = arr[k * num + a->len - 1];
            a->rightsum = cnt * (c1 + c2) / 2;
            a->rightlen = cnt;
            a->rightfirst = c1;
            a->dright = d;
            if(!a->has_right)
            {
                mean = (c1 + c2) / 2;

                //for(j = 0; j < cnt; j++)
                  //  arr[k * num + ind + j] = mean;
            }
        }
        if(flag && cnt1 == 1)
        {
            a->progression_full = true;
            c2 = arr[k * num + a->len - 1];
            a->sum = cnt * (c1 + c2) / 2;
            if(!a->has_right && !a->has_left)
            {
                mean = (c1 + c2) / 2;

                //for(j = 0; j < cnt; j++)
                  //  arr[k * num + ind + j] = mean;
            }
        }
    }
    else if(a->len == 1)
    {
        a->progression_full = true;
        a->sum = arr[k];
    }
}


void* thread_func(void* arg)
{
    Args* a = (Args*)arg;

    double* arr = a->arr;

    int n = a->n;
    int k = a->k;
    int p = a->p;
    int num = n / p;
    int dob = n % p;

    int i, j;

    a->full_time = get_full_time();
    a->cpu_time = get_cpu_time();

    process_sequence(a, num, dob);

    //a->print();

    pthread_barrier_wait(a->barrier);

    //if(k == 0)
      //  a->print_array();

    if(a->progression_left)
    {
        for(i = 1; i < k; i++)
        {
            if(!(a-i)->progression_full || !(std::abs((a-i)->dfull - a->dleft) < eps))
                break;
        }
        a->leftlen = 0;
        if((a-i)->progression_full)
        {
            a->to_change_left = ((a - i + 1)->first + a->leftlast) / 2;
        }
        else
        {
            if((a-i)->progression_right)
            {
                if(std::abs((a-i)->dright - a->dleft) < eps)
                    a->to_change_left = ((a-i)->rightfirst + a->leftlast) / 2;
                else 
                    a->to_change_left = ((a - i + 1)->first + a->leftlast) / 2;
            }
            else
            {
                if(std::abs((a-i+1)->first - (a-i)->last - a->dleft) < eps)
                {
                    a->to_change_left = ((a - i)->last + a->leftlast) / 2;
                }
                else
                {
                    a->to_change_left = ((a-i+1)->first + a->leftlast) / 2;
                }
            }
        }
    }
    if(a->progression_right)
    {
        for(i = 1; i < p - k; i++)
        {
            if(!(a+i)->progression_full || !(std::abs((a+i)->dfull - a->dright) < eps))
                break;
        }

        a->rightlen = 0;
        if((a+i)->progression_full)
        {
            a->to_change_right = ((a+i-1)->last + a->rightfirst) / 2;
        }
        else
        {
            if((a+i)->progression_left)
            {
                if(std::abs((a+i)->dleft - a->dright) < eps)
                    a->to_change_right = ((a+i)->leftlast + a->rightfirst) / 2;
                else 
                    a->to_change_right = ((a + i - 1)->last + a->rightfirst) / 2;
            }
            else
            {
                if(std::abs(-(a+i-1)->last + (a+i)->first - a->dright) < eps)
                {
                    a->to_change_right = ((a + i)->first + a->rightfirst) / 2;
                }
                else
                {
                    a->to_change_left = ((a+i-1)->last + a->rightfirst) / 2;
                }
            }
        }
    }
    if(a->progression_full)
    {
        for(i = 1; i < k; i++)
        {
            if(!(a-i)->progression_full || !(std::abs((a-i)->dfull - a->dfull) < eps))
                break;
        }

        for(j = 1; j < p - k; j++)
        {
            if(!(a+j)->progression_full || !(std::abs((a+j)->dfull - a->dfull) < eps))
                break;
        }

        a->len = 0;
        if((a-i)->progression_full && (a+j)->progression_full)
        {
            a->to_change_full = ((a-i+1)->first + (a+j-1)->last) / 2;
        }
        else if((a-i)->progression_full)
        {
            if((a+j)->progression_left)
            {
                if(std::abs((a+j)->dleft - a->dfull) < eps)
                    a->to_change_full = ((a+j)->leftlast + (a-i+1)->first) / 2;
                else 
                    a->to_change_full = ((a + j - 1)->last + (a-i+1)->first) / 2;
            }
            else
            {
                if(std::abs(-(a+j-1)->last + (a+j)->first - a->dfull) < eps)
                {
                    a->to_change_full = ((a + j)->first + (a-i+1)->first) / 2;
                }
                else
                {
                    a->to_change_full = ((a+j-1)->last + (a-i+1)->first) / 2;
                }
            }
        }
        else if((a+j)->progression_full)
        {
            if((a-i)->progression_right)
            {
                if(std::abs((a-i)->dright - a->dfull) < eps)
                    a->to_change_full = ((a-i)->rightfirst + (a+j-1)->last) / 2;
                else 
                    a->to_change_full = ((a - i + 1)->first + (a+j-1)->last) / 2;
            }
            else
            {
                if(std::abs((a-i+1)->first - (a-i)->last - a->dfull) < eps)
                {
                    a->to_change_full = ((a - i)->last + (a+j-1)->last) / 2;
                }
                else
                {
                    a->to_change_full = ((a-i+1)->first + (a+j-1)->last) / 2;
                }
            }
        }
        else
        {
            if((a-i)->progression_right && (a+j)->progression_left)
            {
                if(std::abs((a-i)->dright - a->dfull) < eps && std::abs((a+j)->dleft - a->dfull) < eps)
                {
                    a->to_change_full = ((a-i)->rightfirst + (a+j)->leftlast) / 2;
                }
                else if(std::abs((a-i)->dright - a->dfull) < eps)
                {
                    a->to_change_full = ((a-i)->rightfirst + (a+j-1)->last) / 2;
                }
                else if(std::abs((a+j)->dleft - a->dfull) < eps)
                {
                    a->to_change_full = ((a-i + 1)->first + (a+j)->leftlast) / 2;
                }
                else
                {
                    a->to_change_full = ((a-i+1)->first + (a+j-1)->last) / 2;
                }
            }
            else if((a-i)->progression_right)
            {
                if(std::abs((a-i)->dright - a->dfull) < eps && std::abs((a+j)->first - (a+j-1)->last - a->dfull) < eps)
                {
                    a->to_change_full = ((a-i)->rightfirst + (a+j)->first) / 2;
                }
                else if(std::abs((a-i)->dright - a->dfull) < eps)
                {
                    a->to_change_full = ((a-i)->rightfirst + (a+j-1)->last) / 2;
                }
                else if(std::abs((a+j)->first - (a+j-1)->last - a->dfull) < eps)
                {
                    a->to_change_full = ((a-i+1)->first + (a+j)->first) / 2;
                }
                else
                {
                    a->to_change_full = ((a-i+1)->first + (a+j-1)->last) / 2;
                }
            }
            else if((a+j)->progression_left)
            {
                if(std::abs((a-i+1)->first - (a-i)->last - a->dfull) < eps && std::abs((a+j)->dleft - a->dfull) < eps)
                {
                    a->to_change_full = ((a-i)->last + (a+j)->leftlast) / 2;
                }
                else if(std::abs((a-i+1)->first - (a-i)->last - a->dfull) < eps)
                {
                    a->to_change_full = ((a-i)->last + (a+j-1)->last) / 2;
                }
                else if(std::abs((a+j)->dleft - a->dfull) < eps)
                {
                    a->to_change_full = ((a-i+1)->first + (a+j)->leftlast) / 2;
                }
                else
                {
                    a->to_change_full = ((a-i+1)->first + (a+j-1)->last) / 2;
                }
            }
            else
            {
                if(std::abs((a-i+1)->first - (a-i)->last - a->dfull) < eps && std::abs((a+j)->first - (a+j-1)->last - a->dfull) < eps)
                {
                    a->to_change_full = ((a-i)->last + (a+j)->first) / 2;
                }
                else if(std::abs((a-i+1)->first - (a-i)->last - a->dfull) < eps)
                {
                    a->to_change_full = ((a-i)->last + (a+j-1)->last) / 2;
                }
                else if(std::abs((a+j)->first - (a+j-1)->last - a->dfull) < eps)
                {
                    a->to_change_full = ((a-i+1)->first + (a+j)->first) / 2;
                }
                else
                {
                    a->to_change_full = ((a-i+1)->first + (a+j-1)->last) / 2;
                }
            }
        }
    }


    pthread_berrier_wait(a);

    if(a->progression_left)
    {
        for(i = 0; i < a->leftlen; i++)
            arr[k*num + i] = a->to_change_left;
    }
    if(a->progression_right)
    {
        for(i = 0; i < a->rightlen; i++)
        {
            if(k < p - 1)
                arr[(k+1) * num - 1 - i] = a->to_change_right;
            else
                arr[(k+1) *num + dob - 1 - i] = a->to_change_right;
        }
    }
    if(a->progression_full)
    {
        for(i = 0; i < a->len; i++)
            arr[k * num + i] = a->to_change_full;
    }


    reduce_sum(p, &a->changed, 1);

    //a->print();


    a->full_time = get_full_time() - a->full_time;
    a->cpu_time = get_cpu_time() - a->cpu_time;

    return nullptr;
}

