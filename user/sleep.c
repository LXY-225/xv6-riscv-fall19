#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]){
    if(argc != 2){
        fprintf(2, "Usage: sleep <ticks>\n");
        exit();
    }
    int x = atoi(argv[1]);
    fprintf(1, "Sleep %d\n", x);
    sleep(x);
    exit();
}