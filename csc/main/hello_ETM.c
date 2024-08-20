#include <stdio.h>
#include <unistd.h>

int main() {

    printf("Hello, ETM! pid: %d\n", getpid());
    
    // write through a buffer to genreate some traffic
    // int size = 1024 * 1024 * 1024; // this is the demo size at the beginning. trying to get counter be meaningful
    int size = 1024;
    int sum = 0;
    char buffer[size];
    for (int i = 0; i < size; i++) {
        buffer[i] = i;
        sum += buffer[i];
    }

    printf("Bye, ETM!\n");
    
    return 0;
}