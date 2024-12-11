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
    double target = 0;
    io_status error_type = io_status::undef;
    int error_flag = 0;
};

void* thread_func(void* arg);
int reduce_sum(int p, int* a = nullptr, int n = 0);
int reduce_min(int p, double* a = nullptr, int* l = nullptr, int n = 0);
io_status task(FILE* f, double* targ, int* n);
io_status final(FILE* f, int* ans, double* min);
int process_args(Args* a);