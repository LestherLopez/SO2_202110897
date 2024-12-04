#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#define current_time 548  
#define reboot_time 549
int main() {
    long result;
    long result2;
    result = syscall(current_time);
    result2 = syscall(reboot_time);
    

    
    printf("Hora actual en segundos desde epoch: %ld\n", result);


    printf("Segundos desde el ultimo reinicio: %ld\n", result2);
    return 0;
}
