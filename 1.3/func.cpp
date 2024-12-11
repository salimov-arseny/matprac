#include "header.h"

io_status task(FILE* f, int* ans, int* n)
{
    int cnt = 0;
    double c1, c2, c3;
    int code1, code2, code3;

    code1 = fscanf(f, "%lf", &c1);
    code2 = fscanf(f, "%lf", &c2);
    code3 = fscanf(f, "%lf", &c3);

    if(code1 == -1)
    {
        *n = 0;
        *ans = 0;
        return io_status::success;
    }
    else if(code1 == 0)
    {
        return io_status::error_read;
    }

    if(code2 == -1)
    {
        *n = 1;
        *ans = 0;
        return io_status::success;
    }
    else if(code2 == 0)
    {
        return io_status::error_read;
    }

    if(code3 == -1)
    {
        *n = 2;
        *ans = 0;
        return io_status::success;
    }
    else if(code3 == 0)
    {
        return io_status::error_read;
    }

    *n = 3;

    if(c1 * c3 > 0)
        if(c2 < sqrt(c1 * c3))
            cnt++;

    c1 = c2;
    c2 = c3;

    while(fscanf(f, "%lf", &c3) == 1)
    {
        (*n)++;
        if(c1 * c3 > 0)
            if(c2 < sqrt(c1 * c3))
                cnt++;
        c1 = c2;
        c2 = c3;
    }

    if(fscanf(f, "%lf", &c3) != EOF)
        return io_status::error_read;

    *ans = cnt;

    return io_status::success;
}

void* thread_func(void* arg)
{
    Args* a = (Args*)arg;

    FILE* f = fopen(a->name, "r");
    if(!f)
    {
        a->error_type = io_status::error_open;
        a->error_flag = 1;
    }
    else
        a->error_flag = 0;

    if(a->error_flag > 0)
    {
        if(f)
            fclose(f);
        return nullptr;
    }

    a->error_type = task(f, &a->res, &a->count);
    
    if(a->error_type != io_status::success)
        a->error_flag = 1;
    else
        a->error_flag = 0;


    if(a->error_flag > 0)
    {
        if(f)
            fclose(f);
        return nullptr;
    }

    fclose(f);
    a->error_type = io_status::success;
    return nullptr;
}


