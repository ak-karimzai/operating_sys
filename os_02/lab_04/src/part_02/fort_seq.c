#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ahmad Khalid Karimzai");
MODULE_DESCRIPTION("LKM fortune cookies using seq_file :)");

#define MAX_COOKIE_LEN PAGE_SIZE

static struct proc_dir_entry *proc_entry;

static char *cookie_pot;
static int cookie_index;
static int next_fortune;

static char tmp[MAX_COOKIE_LEN];

static int seq_show(struct seq_file *seqf, void *v) {
    int len;
    if (next_fortune == 0) return 0;

    len = snprintf(tmp, MAX_COOKIE_LEN, "seq: %s", &cookie_pot[cookie_index]);
    seq_printf(seqf, "%s", tmp);
    cookie_index += len;

    return len;    
}

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

static ssize_t fortune_read(struct file *filp, char __user *buf, size_t count, loff_t *offp) {
    ssize_t len;
    
    if (*offp > 0)
        return 0;

    if (next_fortune >= cookie_index) next_fortune = 0;
    len = sprintf(tmp, "%s\n", &cookie_pot[next_fortune]);

    if (copy_to_user(buf, tmp, len)) {
        printk(KERN_INFO "sequence: copy_to_user error\n");
        return -EFAULT;
    }

    next_fortune += len;
    *offp += len;
    return len;
}

static int fortune_open(struct inode *sp_inode, struct file *sp_file){
    return single_open(sp_file, seq_show, NULL);
}

static int fortune_release(struct inode *sp_inode, struct file *sp_file){
    return single_release(sp_inode, sp_file);
}

static struct proc_ops fops = {
    .proc_read = fortune_read,
    .proc_write = fortune_write,
    .proc_open = fortune_open,
    .proc_release = fortune_release,
};

static int __init init_fortune_module(void) {
    int ret = 0;
    cookie_pot = (char *) vmalloc(MAX_COOKIE_LEN);

    if (cookie_pot == NULL) {
        ret = -ENOMEM;
    } else {
        memset(cookie_pot, 0, MAX_COOKIE_LEN);
        proc_entry = proc_create("sequence", 0644, NULL, &fops);
        if (proc_entry == NULL) {
            ret = -ENOMEM;
            vfree(cookie_pot);
            printk(KERN_INFO "sequence: coludn't create proc entry\n");
        } else {
            cookie_index = next_fortune = 0;
            printk(KERN_INFO "sequence: module loaded.\n");
        }
    }
    return ret;
}

static void __exit cleanup_fortune_module(void) {
    remove_proc_entry("sequence", NULL);
    vfree(cookie_pot);
    printk(KERN_INFO "sequence: module unloaded.\n");
}

module_init(init_fortune_module);
module_exit(cleanup_fortune_module);