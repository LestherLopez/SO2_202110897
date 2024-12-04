#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/syscalls_usac.h>

SYSCALL_DEFINE2(get_fault, long __user *, minor_faults, long __user *, major_faults)
{
    long fallos_menores;
    long fallos_mayores;


    fallos_menores = global_node_page_state(NR_FILE_PAGES) + global_node_page_state(NR_ANON_MAPPED);
    fallos_mayores = global_node_page_state(NR_FILE_PMDMAPPED) + global_node_page_state(PGMAJFAULT);

    if (copy_to_user(minor_faults, &fallos_menores, sizeof(fallos_menores)) || 
        copy_to_user(major_faults, &fallos_mayores, sizeof(fallos_mayores))) {
        return -EFAULT;
    }

    return 0; 
}

