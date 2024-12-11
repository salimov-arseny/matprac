#include "header.h"

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

io_status task(FILE* f, double* targ, int* n)
{
    double c1, c2;
    int code1;
    double curmin, target;
    int curlen, maxlen;
    int cnt = 0;
    curlen = maxlen = 1;

    code1 = fscanf(f, "%lf", &c1);
    
    if(code1 == -1)
    {
        *n = 0;
        *targ = 0;
        return io_status::success;
    }
    else if(code1 == 0)
    {
        return io_status::error_read;
    }
    
    //curmin = c1;
    target = curmin = 0;

    while(fscanf(f, "%lf", &c2) == 1) 
    {
        
        if (c2 < c1) 
        {
            curlen++;
            curmin = c2;
        } 
        else 
        {
            if (curlen >= maxlen && curlen > 1) 
            {
                
                printf("curlen = %d maxlen = %d\n", curlen, maxlen);
                
                if(curlen > maxlen)
                    target = curmin;
                else
                {
                    if(cnt > 0)
                    {
                        if(target > curmin)
                            target = curmin;
                    }
                    else
                        target = curmin;
                    printf("curmin = %lf target = %lf cnt = %d\n", curmin, target, cnt);
                    
                }
                cnt++;
                maxlen = curlen;
            }
            
            curlen = 1;
            //curmin = c2;
        }
        c1 = c2;
    }
    
    if(fscanf(f, "%lf", &c2) != EOF)
        return io_status::error_read;
    
    if(curlen > maxlen)
    {
        maxlen = curlen;
        target = curmin;
    }

    if (curlen >= maxlen && maxlen > 1) 
    {
        printf("curlen = %d maxlen = %d\n", curlen, maxlen);
        maxlen = curlen;
        if(target > curmin)
            target = curmin;
        printf("curmin = %lf target = %lf\n", curmin, target);
    }
    
    printf("%lf\n", target);
    printf("cur = %d max = %d\n",curlen, maxlen);
    
    *n = maxlen;
    *targ = target;
    return io_status::success;
}

io_status final(FILE* f, int* ans, double* min)
{
    int cnt = 0;
    fseek(f, 0, SEEK_SET);
    double tmp;

    while(fscanf(f, "%lf", &tmp) == 1)
    {
        //printf("%lf < %lf : %d\n", tmp, *min, cnt);
        if(tmp < *min)
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
    
    int c;

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
    a->error_type = task(f, &a->target, &a->count);
    
    //printf("thread %d target = %lf, len = %d\n", a->k, a->target, a->count);
    
    c = a->count;
    
    if(a->error_type != io_status::success)
        a->error_flag = 1;
    else
        a->error_flag = 0;
    //printf("thread %d, errflag = %d\n", a->k, a->error_flag);

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
    
    reduce_sum(a->p, &a->count, 1);
    
    if(a->count == a->p)
    {
        a->res = 0;
        return nullptr;
    }
    
    a->count = c;
    
    reduce_min(a->p, &a->target, &a->count, 1);
    
    //printf("thread %d after reduce target = %lf, len = %d\n",a->k, a->target, a->count);
    
    a->error_type = final(f, &a->res, &a->target);
    
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

int reduce_sum(int p, int* a, int n)
{
    static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    static pthread_cond_t c_in = PTHREAD_COND_INITIALIZER;
    static pthread_cond_t c_out = PTHREAD_COND_INITIALIZER;
    static int t_in = 0;
    static int t_out = 0;
    static int *r = nullptr;
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

int reduce_min(int p, double* a, int* l, int n)
{
    static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    static pthread_cond_t c_in = PTHREAD_COND_INITIALIZER;
    static pthread_cond_t c_proc = PTHREAD_COND_INITIALIZER;
    static pthread_cond_t c_out = PTHREAD_COND_INITIALIZER;
    static int t_in = 0;
    static int t_proc = 0;
    static int t_out = 0;
    static int len = 1;
    static double *r = nullptr;
    int i;
    
    //printf("len = %d, %d\n", len, l[0]);

    if(p <= 1)  return 0;

    pthread_mutex_lock(&m);
    if(l[0] > 1)
    {
        for(i = 0; i < n; i++)
        {
            //printf("%d > %d\n", l[i], len);
            if(l[i] > len)
                len = l[i];
        }
    }
    t_in++;
    
    if(t_in >= p)
    {
        t_proc = 0;
        pthread_cond_broadcast(&c_in);
    }
    else    
        while(t_in < p)
            pthread_cond_wait(&c_in, &m);
            
    //printf("LEN = %d\n", len);
    
    if(l[0] == len)
    {
        if(r == nullptr)
            r = a;
        else if(r[0] > a[0])
            r[0] = a[0];
    }   
    t_proc++;
    

    if(t_proc >= p)
    {
        t_out = 0;
        pthread_cond_broadcast(&c_proc);
    }
    else    
        while(t_proc < p)
            pthread_cond_wait(&c_proc, &m);
            
    
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


