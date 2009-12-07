#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>

#include "err.h"
#include "ipc_msg.h"

int main(int argc, char **argv)
{
    //FILE *stdlog = stderr;
    char action;
    int i;
    long *vertices;
    long vertices_quantity;
    order_msgbuf message;
    respond_msgbuf result;
    int msgid;
    if(argc < 2)
        fatal("ERROR: Too few arguments.");
    action = argv[1][0];  /* First char of second arg. */
    switch(action) {
        case '+':
            if(argc != 5)
                fatal("Wrong number of arguments. `+` requires 3 additional "
                        "args: `from` vertex, `to` vertex and edge weight.");
            message.order = order_add_edge(atoi(argv[2]), atoi(argv[3]),
                    atoi(argv[4]));
            break;
        case '-':
            if(argc != 4)
                fatal("Wrong number of arguments. `-` requires 2 additional "
                        "args: `from` vertex and `to` vertex.");
            message.order = order_remove_edge(atoi(argv[2]), atoi(argv[3]));
            break;
        case 'H':
            if(argc < 3)
                fatal("Wrong number of arguments. `H` requires at least "
                        "1 vertex.");
            if(argc > 52)
                fatal("Wrong number of arguments. `H` works with max of 50 "
                        "vertices (but beware that H has cost higher than"
                        "exponential).");
            vertices_quantity = argc - 2;
            if((vertices = calloc((size_t) (vertices_quantity), sizeof(long)))
                    == NULL)
                syserr("calloc: When allocating space for vertices.");
            for(i = 0; i < vertices_quantity; i ++)
                vertices[i] = atoi(argv[i]);
            message.order = order_hamiltonian_cycle(vertices_quantity,
                    vertices);
            break;
        default:
            fatal("Invalid command. You ought to use `+`, `-` or `H` "
                    "as your first argument.");
    }
    message.msg_type = IPC_ORDERS_RESERVED;
    message.order.client_pid = (long) getpid();
    if((msgid = msgget(get_ipc_key(), 0666)) == -1)
        syserr("msgget: While receiving IPC id.");
    if(msgsnd(msgid, &message, sizeof(order_t), 0) != 0)
        syserr("msgsnd: While sending order.");
    if(msgrcv(msgid, &result, sizeof(respond_msgbuf) - sizeof(long),
                getpid(), 0) == -1)
        syserr("msgrcv: While receiving result.");
    switch(action) {
        case '+':
            return result.code;
        case '-':
            return result.code;
        case 'H':
            fprintf(stdout, "H %d\n", (int) result.code);
            if(result.code == 0)
                return 1;
            else
                return 0;
        default:
            fatal("Internal error.");
    }
    return 0;
}
