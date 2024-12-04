#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/swap.h>
#include <linux/vmstat.h>
#include <linux/syscalls_usac.h>

SYSCALL_DEFINE2(get_swap, long __user *, used_swap_pages, long __user *, free_swap_pages)
{
    struct sysinfo si;

    si_swapinfo(&si);

    long used_swap = si.totalswap - si.freeswap;
    long free_swap = si.freeswap;


    if (copy_to_user(used_swap_pages, &used_swap, sizeof(used_swap)) || 
        copy_to_user(free_swap_pages, &free_swap, sizeof(free_swap))) {
        return -EFAULT;
    }

    return 0;
}
