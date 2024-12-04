#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/mm.h>       // Para acceder a la administración de memoria
#include <linux/swap.h>     // Para las estadísticas de intercambio (swap)
#include <linux/sched.h>    // Para acceder a las tareas
#include <linux/syscalls_usac.h>

// Definir la syscall
SYSCALL_DEFINE2(get_activeinactive, long __user *, active_pages,long __user *, inactive_pages)
{
    long paginas_activas;
    long paginas_inactivas
    paginas_activas = global_node_page_state(NR_ACTIVE_FILE) + global_node_page_state(NR_ACTIVE_ANON);
    paginas_inactivas = global_node_page_state(NR_INACTIVE_FILE) + global_node_page_state(NR_INACTIVE_ANON);

   
    if (copy_to_user(active_pages, &paginas_activas, sizeof(paginas_activas)) ||
        copy_to_user(inactive_pages, &paginas_inactivas, sizeof(paginas_inactivas))) {
        return -EFAULT; 

    return 0;  
}
