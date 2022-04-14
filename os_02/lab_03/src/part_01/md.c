#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/init.h> /* __init __exit */
#include <linux/sched.h> /* struct task_struct */
#include <linux/init_task.h> /* struct init_task */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ahmad Khalid Karimzai");

static int __init md_init(void) {
	
	printk(KERN_INFO "Module MD loaded.\n");

	struct task_struct *task = &init_task;
	do {
		printk(KERN_INFO "---%s-%d, parent %s-%d", 
            task->comm, task->pid, 
            task->parent->comm, task->parent->pid);
	} while ((task = next_task(task)) != &init_task);

	printk(KERN_INFO "---%s-%d, parent %s-%d", 
        current->comm, current->pid, 
        current->parent->comm, current->parent->pid);
	return 0;
}

static void __exit md_exit(void) {
	printk(KERN_INFO " Module MD unloaded.\n");
}

module_init(md_init);
module_exit(md_exit);