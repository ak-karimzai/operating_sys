/* 
 * procfs4.c -  create a "file" in /proc
 * This program uses the seq_file library to manage the /proc file.
 */ 
 
#include <linux/kernel.h> /* We are doing kernel work */ 
#include <linux/module.h> /* Specifically, a module */ 
#include <linux/proc_fs.h> /* Necessary because we use proc fs */ 
#include <linux/seq_file.h> /* for seq_file */ 
#include <linux/vmalloc.h> /* for seq_file */ 
#include <linux/version.h> 
 
#define MAX_COOKIE_LEN PAGE_SIZE
 
#define PROC_NAME "" 
 
//  ////////////////
 static char *cookie_pot;
static int cookie_index;
static int next_fortune;

static char tmp[MAX_COOKIE_LEN];


static ssize_t fortune_write(struct file *filp, const char __user *buff, unsigned long len, loff_t *data) {
    int space_avle = MAX_COOKIE_LEN - cookie_index + 1;
    
    if (space_avle < len) {
        printk(KERN_INFO "sequence: cookie pot is full!\n");
        return -ENOSPC;
    }

    if (copy_from_user(&cookie_pot[cookie_index], buff, len)) {
        printk(KERN_INFO "copy_from_user error!\n");
        return -EFAULT;
    }

    cookie_index += len;
    cookie_pot[cookie_index - 1] = '\0';
    return len;
}
/////////////////////////////////////////

/* This function is called at the beginning of a sequence. 
 * ie, when: 
 *   - the /proc file is read (first time) 
 *   - after the function stop (end of sequence) 
 */ 
static void *my_seq_start(struct seq_file *s, loff_t *pos) 
{ 
    static unsigned long counter = 0; 
 
    /* beginning a new sequence? */ 
    if (*pos == 0) { 
        /* yes => return a non null value to begin the sequence */ 
        return &counter; 
    } 
 
    /* no => it is the end of the sequence, return end to stop reading */ 
    *pos = 0; 
    return NULL; 
} 
 
/* This function is called after the beginning of a sequence. 
 * It is called untill the return is NULL (this ends the sequence). 
 */ 
static void *my_seq_next(struct seq_file *s, void *v, loff_t *pos) 
{ 
    unsigned long *tmp_v = (unsigned long *)v; 
    (*tmp_v)++; 
    (*pos)++; 
    return NULL; 
} 
 
/* This function is called at the end of a sequence. */ 
static void my_seq_stop(struct seq_file *s, void *v) 
{ 
    /* nothing to do, we use a static value in start() */ 
} 
 
/* This function is called for each "step" of a sequence. */ 
static int my_seq_show(struct seq_file *s, void *v) 
{ 
 
    seq_printf(s, "seq: %s\n", ((char*) v) ); 
    return 0; 
} 
 
/* This structure gather "function" to manage the sequence */ 
static struct seq_operations my_seq_ops = { 
    .start = my_seq_start, 
    .next = my_seq_next, 
    .stop = my_seq_stop, 
    .show = my_seq_show, 
}; 
 
/* This function is called when the /proc file is open. */ 
static int my_open(struct inode *inode, struct file *file) 
{ 
    return seq_open(file, &my_seq_ops); 
}; 
 
/* This structure gather "function" that manage the /proc file */ 
static const struct proc_ops my_file_ops = { 
    .proc_open = my_open, 
    .proc_write = fortune_write, 
    .proc_read = seq_read, 
    .proc_lseek = seq_lseek, 
    .proc_release = seq_release, 
}; 
 
static int __init procfs4_init(void) 
{ 
    struct proc_dir_entry *entry; 
 
    entry = proc_create(PROC_NAME, 0666, NULL, &my_file_ops); 
    if (entry == NULL) {
        cookie_pot = (char *) vmalloc(MAX_COOKIE_LEN);
        remove_proc_entry(PROC_NAME, NULL); 
        pr_debug("Error: Could not initialize /proc/%s\n", PROC_NAME); 
        return -ENOMEM; 
    } 

    next_fortune = cookie_index = 0;
    return 0; 
} 
 
static void __exit procfs4_exit(void) 
{ 
    remove_proc_entry(PROC_NAME, NULL); 
    // vfree(cookie_pot);
    pr_debug("/proc/%s removed\n", PROC_NAME); 
} 
 
module_init(procfs4_init); 
module_exit(procfs4_exit); 
 
MODULE_LICENSE("GPL");