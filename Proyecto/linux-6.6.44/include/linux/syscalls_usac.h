#ifndef _SYSCALLS_USAC_H
#define _SYSCALLS_USAC_H
#include <linux/syscalls.h>
#include <linux/kernel.h>


asmlinkage long sys_my_encrypt(const char __user *input_file,
const char __user *output_file,
int num_threads,
const char __user *key_file);
asmlinkage long sys_my_decrypt(const char __user *input_file,
const char __user *output_file,
int num_threads,
const char __user *key_file);
asmlinkage long sys_get_memory_info(long __user *totalram,
				    long __user *freeram,
				    long __user *usedram,
				    long __user *cachedram);

asmlinkage long sys_get_swap(long __user *used_swap_pages,
                             long __user *free_swap_pages);
asmlinkage long sys_get_fault(long __user *minor_faults,
                             long __user *major_faults);
asmlinkage long sys_get_activeinactive(long __user *active_pages,
                             long __user *inactive_pages);


#endif
