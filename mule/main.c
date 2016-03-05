#include <stdio.h>

#include "msg.h"
#include "shell.h"

#define MAIN_MSG_QUEUE_SIZE (4)
static msg_t main_msg_queue[MAIN_MSG_QUEUE_SIZE];

extern int mule_server(int argc, char **argv);

static const shell_command_t shell_commands[] = {
    { "mule_server", "send data over UDP and listen on UDP ports", mule_server },
    { NULL, NULL, NULL }
};

int main(void)
{
    msg_init_queue(main_msg_queue, MAIN_MSG_QUEUE_SIZE);
    puts("RIOT socket example application");

    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
