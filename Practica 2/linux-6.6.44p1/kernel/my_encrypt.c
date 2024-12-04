#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/ctype.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/syscalls_usac.h>

typedef struct {
    char *input;
    const char *key;
    int start;
    int end;
} ThreadParams;

// Funcion para encriptar
static int xor_encrypt_decrypt_thread(void *args) {
    ThreadParams *thread_args = (ThreadParams *)args;
    int key_len = strlen(thread_args->key);

    for (int i = thread_args->start; i < thread_args->end; i++) {
        thread_args->input[i] ^= thread_args->key[i % key_len];
    }

    return 0;
}

// Función para eliminar caracteres innecesarios
static void clean_string(char *str) {
    int len = strlen(str);
    while (len > 0 && isspace(str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }
}

// Función principal de cifrado
SYSCALL_DEFINE4(my_encrypt, 
                const char __user *, input_file, 
                const char __user *, output_file, 
                int, num_threads, 
                const char __user *, key_file) {
   //------------
   
    struct file *input_fp, *key_fp, *output_fp;
    char *input_data, *key_data;
    struct task_struct **threads;
    int i, fragment_size;
    long input_size, key_size;
    ThreadParams *args;
    


    input_fp = filp_open(input_file, O_RDONLY, 0);
    if (IS_ERR(input_fp)) {
        pr_err("error: no fue posible abrir el archivo de entrada\n");
        return PTR_ERR(input_fp);
    }

 
    input_size = i_size_read(file_inode(input_fp));
    if (input_size <= 0) {
       pr_err("error: el tamaño del archivo de entrada es invalido, debe ser igual o mayor a 0\n");
        return -EIO;
    }


    input_data = kmalloc(input_size + 1, GFP_KERNEL);  
    if (!input_data) {
        filp_close(input_fp, NULL);
        pr_err("error: no fue posible reservar memoria para el archivo de entrada\n");
        return -ENOMEM;
    }

 
    loff_t input_pos = 0; 
    ssize_t bytes_read = kernel_read(input_fp, input_data, input_size, &input_pos);
    if (bytes_read != input_size) {
        pr_err( "error: no fue posible leer el archivo de entrada\n");
        vfree(input_data);
        filp_close(input_fp, NULL);
        return -EIO;
    }
    input_data[input_size] = '\0'; 
  
    clean_string(input_data);



    printk(KERN_INFO "contenido archivo de entrada: %s.\n", input_data);
  
    key_fp = filp_open(key_file, O_RDONLY, 0);
    if (IS_ERR(key_fp)) {
        vfree(input_data);
        pr_err("error: no fue posible abrir el archivo de clave\n");
        
        return PTR_ERR(key_fp);
    }

  
    key_size = i_size_read(file_inode(key_fp));
    if (key_size <= 0) {
       
        pr_err( "error: el tamaño del archivo de entrada es invalido, debe ser igual o mayor a 0\n");
        vfree(input_data);
        filp_close(key_fp, NULL);
        
        return -EIO;
    }

  
    key_data = vmalloc(key_size + 1); 
    if (!key_data) {
        vfree(input_data);
        filp_close(key_fp, NULL);
        pr_err("error: no fue posible reservar memoria para el key file\n");
       
        return -ENOMEM;
    }

    loff_t key_pos = 0;
    ssize_t key_bytes_read = kernel_read(key_fp, key_data, key_size, &key_pos);
    if (key_bytes_read != key_size) {
           pr_err("error: no fue posible leer el key file\n");
        vfree(input_data);
        vfree(key_data);
        filp_close(key_fp, NULL);
        
        return -EIO;
    }
    
    key_data[key_size] = '\0'; 


    clean_string(key_data);
    printk(KERN_INFO "contenido de keyfile: %s.\n", key_data);


    fragment_size = input_size / num_threads;


    threads = vmalloc(num_threads * sizeof(struct task_struct *));
    if (!threads) {
        vfree(input_data);
        vfree(key_data);
        pr_err("error: No es posible reservar memoria para cada hilos\n");
       
        return -ENOMEM;
    }

    args = vmalloc(num_threads * sizeof(ThreadParams));
    if (!args) {
        vfree(input_data);
        vfree(key_data);
        vfree(threads);
        pr_err("error: No es posible reservar memoria para cada argumento de los hilos\n");
       
        return -ENOMEM;
    }

    // Crear y ejecutar los hilos
    for (i = 0; i < num_threads; i++) {
        // Asignar los valores a la estructura de parámetros
        args[i].input = input_data;
        args[i].key = key_data;
        args[i].start = i * fragment_size;
        args[i].end = (i == num_threads - 1) ? input_size : (i + 1) * fragment_size;

        // Crear el hilo del kernel
        threads[i] = kthread_run(xor_encrypt_decrypt_thread, &args[i], "xor_thread_%d", i);
        if (IS_ERR(threads[i])) {
            pr_err("error: no fue posible crear el hilo No. %d\n", i);
           
            return PTR_ERR(threads[i]);
        }
    }


    // Esperar a que todos los hilos terminen
    for (i = 0; i < num_threads; i++) {
        kthread_stop(threads[i]);
    }

    // Abrir el archivo de salida
    output_fp = filp_open(output_file, O_WRONLY | O_CREAT, 0644);
    if (IS_ERR(output_fp)) {
        vfree(input_data);
        vfree(key_data);
        vfree(threads);
        vfree(args);
        pr_err("error: No fue posible abrir el archivo de salida\n");
       
        return PTR_ERR(output_fp);
    }

    // Escribir el contenido encriptado en el archivo de salida
    loff_t output_pos = 0;
    ssize_t bytes_written = kernel_write(output_fp, input_data, input_size, &output_pos);
    if (bytes_written != input_size) {
        pr_err("error: No se puede escribir el contenido en el archivo de salida\n");
        vfree(input_data);
        vfree(key_data);
        vfree(threads);
        vfree(args);
        filp_close(output_fp, NULL);
        
        return -EIO;
    }

    filp_close(output_fp, NULL);



     vfree(input_data);
    vfree(key_data);
    vfree(threads);
    vfree(args);

    pr_info("Se completó la encriptacion\n");
    
    return 0;
}

