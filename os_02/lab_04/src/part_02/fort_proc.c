#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ahmad Khalid Karimzai");
MODULE_DESCRIPTION("LKM fortune cookies using proc_file :)");

#define MAX_COOKIE_LEN PAGE_SIZE

#define DIR_NAME "fproc"
#define DIR_FILE "fproc_file"
#define DIR_SYMLINK "fproc_symlink"
#define FILE_PATH DIR_NAME "/" DIR_FILE

#define PRINT_FUNC_INFO() {printk(KERN_INFO"%s: %s called\n", DIR_NAME, __func__);}

static struct proc_dir_entry *proc_dir = NULL;
static struct proc_dir_entry *proc_slink = NULL;
static struct proc_dir_entry *proc_file = NULL;

static char *cookie_pot = NULL;
static int cookie_index = 0;
static int next_fortune = 0;

static char tmp[MAX_COOKIE_LEN];

ssize_t write__proc(struct file *filep, const char __user *buf, size_t len, loff_t *offp) {
    PRINT_FUNC_INFO();

    int abl_spc = MAX_COOKIE_LEN - cookie_index + 1;

    if (abl_spc < len) {
        printk(KERN_ERR"%s: cookie_pot overflow\n", DIR_NAME);
        return -ENOSPC;
    }

    if (copy_from_user(&cookie_pot[next_fortune], buf, len)) {
        printk(KERN_ERR"%s: copy_from_user error\n", DIR_NAME);
        return -EFAULT;
    }

    next_fortune += len;
    cookie_pot[next_fortune - 1] = '\0';
    return len;
}

ssize_t read__proc(struct file *filp, char __user *buf, size_t count, loff_t *offp) {
    PRINT_FUNC_INFO();
    ssize_t len;
    
    if (*offp > 0 || next_fortune == 0)
        return 0;

    if (next_fortune <= cookie_index) cookie_index = 0;
    
    len = sprintf(tmp, "%s\n", &cookie_pot[cookie_index]);
    if (copy_to_user(buf, tmp, len)) {
        printk(KERN_INFO"fortune: copy_to_user error\n");
        return -EFAULT;
    }

    cookie_index += len;
    *offp += len;
    return len;
}

static int open__proc(struct inode *sq_node, struct file *sq_file) {
    PRINT_FUNC_INFO();
    return 0;
}

static int release__proc(struct inode *sq_node, struct file *sq_file) {
    PRINT_FUNC_INFO();
    return 0;
}

static struct proc_ops fops = {
    .proc_write = write__proc,
    .proc_read = read__proc,
    .proc_open = open__proc,
    .proc_release = release__proc,
};

static void cleanup_proc_module(void);

static int __init init_proc_module(void) {
    PRINT_FUNC_INFO();
    cookie_pot = (char *)vmalloc(MAX_COOKIE_LEN);
    if (cookie_pot == NULL) {
        printk(KERN_ERR"%s: vmalloc error\n", DIR_NAME);
        return -ENOMEM;
    }
    proc_dir = proc_mkdir(DIR_NAME, NULL);
    if (proc_dir == NULL) {
        printk(KERN_INFO"%s: proc_mkdir error!\n", DIR_NAME);
        cleanup_proc_module();
        return -ENOMEM;
    }

    proc_file = proc_create(DIR_FILE, 0666, proc_dir, &fops);
    if (proc_file == NULL) {
        printk(KERN_INFO"%s: proc_create error!\n", DIR_NAME);
        cleanup_proc_module();
        return -ENOMEM;
    }

    proc_slink = proc_symlink(DIR_SYMLINK, NULL, FILE_PATH);
    if (proc_slink == NULL) {
        printk(KERN_INFO"%s: proc_symlink error!\n", DIR_NAME);
        cleanup_proc_module();
        return -ENOMEM;
    }

    cookie_index = 0;
    next_fortune = 0;
    memset(cookie_pot, 0, MAX_COOKIE_LEN);
    
    return 0;
}

static void cleanup_proc_module(void) {
    PRINT_FUNC_INFO();
    if (cookie_pot) vfree(cookie_pot);
    if (proc_slink) remove_proc_entry(DIR_SYMLINK, NULL);
    if (proc_file) remove_proc_entry(DIR_FILE, proc_dir);
    if (proc_dir) remove_proc_entry(DIR_NAME, NULL);
}

static void __exit exit_proc_module(void) {
    PRINT_FUNC_INFO();
    cleanup_proc_module();
}

module_init(init_proc_module);
module_exit(exit_proc_module);