#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/mm.h>
#include <linux/sysinfo.h>
#include <linux/uaccess.h> 
#include <linux/syscalls_usac.h>



SYSCALL_DEFINE4(get_memory_info, 
    long __user *, totalram, 
    long __user *, freeram, 
    long __user *, usedram, 
    long __user *, cachedram)
{
    struct sysinfo i;


    si_meminfo(&i);

    long total = i.totalram * i.mem_unit / 1024;   
    long used = (i.totalram - i.freeram) * i.mem_unit / 1024;    
    long free = i.freeram * i.mem_unit / 1024;      
    long cached = i.bufferram * i.mem_unit / 1024;   


    if (copy_to_user(totalram, &total, sizeof(long)) ||
        copy_to_user(usedram, &used, sizeof(long)) ||
        copy_to_user(freeram, &free, sizeof(long)) ||
        copy_to_user(cachedram, &cached, sizeof(long))) {
        return -EFAULT;  
    }

    return 0; 
}

