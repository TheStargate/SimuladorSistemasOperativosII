#include "simulacion.h"
#include <sys/wait.h>
#include <signal.h>

int acabados = 0;

void reaper()
{
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        acabados++;
    }
}

int main(int argc, char *argv[])
{
}
