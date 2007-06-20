#include <csignal>
#include <cstdlib>
#include <cstdio>
#include "mysignal.h"

using namespace std;

bool flag_end=false;

namespace {

void signal_handler(int n)
{
    printf("\n"
           "============================================\n"
           "=== The program has received signal: %3d ===\n"
           "============================================\n\n",n);
    if( flag_end )
    {
        printf("Forcing exit.\n\n");
        abort();
    }
    else
        flag_end = true;
}

}

void set_signal_handler(void)
{
    signal(0,       signal_handler);
    signal(SIGHUP,  signal_handler);
    signal(SIGINT,  signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGALRM, signal_handler);
    signal(SIGTERM, signal_handler);
}
