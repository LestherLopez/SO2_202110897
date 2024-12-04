#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/timekeeping.h>

SYSCALL_DEFINE0(reboot_time)
{
   
     return ktime_get_boottime_seconds();
}
