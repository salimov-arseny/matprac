#include <iostream>
#include <cmath>
#include <threads.h>

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
    int res = 0;
    int count = 0;
    io_status error_type = io_status::undef;
    int error_flag = 0;
};

void* thread_func(void* arg);
io_status task(FILE* f, int* ans, int* n);