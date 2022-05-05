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

#define DIR_NAME "seq"
#define DIR_FILE "seq_file"
#define DIR_SYMLINK "seq_symlink"
#define FILE_PATH DIR_NAME "/" DIR_FILE

#define PRINT_FUNC_INFO() {printk(KERN_INFO"%s: %s called\n", DIR_NAME, __func__);}

static struct proc_dir_entry *proc_dir = NULL;
static struct proc_dir_entry *proc_slink = NULL;
static struct proc_dir_entry *proc_file = NULL;

static char *cookie_pot = NULL;
static int cookie_index = 0;
static int next_fortune = 0;

static char tmp[MAX_COOKIE_LEN];

static int show__seq(struct seq_file *seqf, void *v) {
    PRINT_FUNC_INFO();
    if (cookie_index == 0) return cookie_index;
    if (next_fortune >= cookie_index) next_fortune = 0;

    int len = snprintf(tmp, MAX_COOKIE_LEN, "seq: %s\n", &cookie_pot[next_fortune]);
    seq_printf(seqf, "%s", tmp);
    next_fortune += len;
    return 0;    
}

static void *start__seq(struct seq_file *m, loff_t *pos) {
    PRINT_FUNC_INFO();
    static unsigned long count = 0;

    if (*pos == 0) return &count;

    *pos = 0;
    return NULL;
}

static void *next__seq(struct seq_file *m, void *v, loff_t *pos) {
    PRINT_FUNC_INFO();
    unsigned long *tmp = (unsigned long *) v;
    (*tmp)++;
    (*pos)++;
    return NULL;
}

static void stop__seq(struct seq_file *m, void *v) {
    PRINT_FUNC_INFO();
}

static struct seq_operations ops__seq = {
    .start = start__seq,
    .next = next__seq,
    .stop = stop__seq,
    .show = show__seq,
};

static int open__seq(struct inode *sq_node, struct file *sq_file) {
    PRINT_FUNC_INFO();
    return seq_open(sq_file, &ops__seq);
}

static int release__seq(struct inode *sq_node, struct file *sq_file) {
    PRINT_FUNC_INFO();
    return 0;
}

ssize_t write__seq(struct file *filep, const char __user *buf, size_t len, loff_t *offp) {
    PRINT_FUNC_INFO();

    int abl_spc = MAX_COOKIE_LEN - cookie_index + 1;

    if (abl_spc < len) {
        printk(KERN_ERR"%s: cookie_pot overflow\n", DIR_NAME);
        return -ENOSPC;
    }

    if (copy_from_user(&cookie_pot[cookie_index], buf, len)) {
        printk(KERN_ERR"%s: copy_from_user error\n", DIR_NAME);
        return -EFAULT;
    }

    cookie_index += len;
    cookie_pot[cookie_index - 1] = '\0';
    return len;
}

static struct proc_ops fops = {
    .proc_open = open__seq,
    .proc_release = release__seq,
    .proc_write = write__seq,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = seq_release,
};

static void cleanup_seq_module(void);

static int __init init_seq_module(void) {
    PRINT_FUNC_INFO();
    cookie_pot = (char *)vmalloc(MAX_COOKIE_LEN);
    if (cookie_pot == NULL) {
        printk(KERN_ERR"%s: vmalloc error\n", DIR_NAME);
        return -ENOMEM;
    }
    memset(cookie_pot, 0, MAX_COOKIE_LEN);
    proc_dir = proc_mkdir(DIR_NAME, NULL);
    if (proc_dir == NULL) {
        printk(KERN_INFO"%s: proc_mkdir error!\n", DIR_NAME);
        cleanup_seq_module();
        return -ENOMEM;
    }

    proc_file = proc_create(DIR_FILE, 0666, proc_dir, &fops);
    if (proc_file == NULL) {
        printk(KERN_INFO"%s: proc_create error!\n", DIR_NAME);
        cleanup_seq_module();
        return -ENOMEM;
    }

    proc_slink = proc_symlink(DIR_SYMLINK, NULL, FILE_PATH);
    if (proc_slink == NULL) {
        printk(KERN_INFO"%s: proc_symlink error!\n", DIR_NAME);
        cleanup_seq_module();
        return -ENOMEM;
    }

    cookie_index = 0;
    next_fortune = 0;
    return 0;
}

static void cleanup_seq_module(void) {
    PRINT_FUNC_INFO();
    if (cookie_pot) vfree(cookie_pot);
    if (proc_slink) remove_proc_entry(DIR_SYMLINK, NULL);
    if (proc_file) remove_proc_entry(DIR_FILE, proc_dir);
    if (proc_dir) remove_proc_entry(DIR_NAME, NULL);
}

static void __exit exit_seq_module(void) {
    PRINT_FUNC_INFO();
    cleanup_seq_module();
}

module_init(init_seq_module);
module_exit(exit_seq_module);