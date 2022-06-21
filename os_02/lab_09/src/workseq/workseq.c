#include <asm/io.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/smp.h>
#include <linux/workqueue.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ahmad Khalid Karimzai");
MODULE_DESCRIPTION("workqueue");

#define IRQ 1 /* keyboard interrupt */
#define SCANCODE_RELEASED_MASK	0x80

int status;
char scancode;
static int dev_id;
struct workqueue_struct *work_queue;

static int get_ascii(unsigned int scancode)
{
	static char *row1 = "1234567890";
	static char *row2 = "qwertyuiop";
	static char *row3 = "asdfghjkl";
	static char *row4 = "zxcvbnm";

	scancode &= ~SCANCODE_RELEASED_MASK;
	if (scancode >= 0x02 && scancode <= 0x0b)
		return *(row1 + scancode - 0x02);
	if (scancode >= 0x10 && scancode <= 0x19)
		return *(row2 + scancode - 0x10);
	if (scancode >= 0x1e && scancode <= 0x26)
		return *(row3 + scancode - 0x1e);
	if (scancode >= 0x2c && scancode <= 0x32)
		return *(row4 + scancode - 0x2c);
	if (scancode == 0x39)
		return ' ';
	if (scancode == 0x1c)
		return '\n';
	return '?';
}

static inline u8 i8042_read_data(void)
{
	u8 val;
	val = inb(0x60);
	return val;
}

void queue_function_fir(struct work_struct *work) {
    printk(KERN_INFO "workseq %s: scancode: %d, inputed char: %c\n", __func__, scancode, get_ascii(scancode));
    // msleep(100);
    // printk(KERN_INFO "workseq %s: after blocking scancode: %d, inputed char: %c\n", __func__, scancode, get_ascii(scancode));
}

void queue_function_sec(struct work_struct *work) {
    printk(KERN_INFO "workseq %s: scancode: %d, inputed char: %c\n", __func__, scancode, get_ascii(scancode));
}

struct work_struct fir_work;
struct work_struct sec_work;

irqreturn_t handler(int irq, void *dev) {
    printk(KERN_INFO "workseq: move work to queue...\n");
    if (irq == IRQ) {
        scancode = i8042_read_data();
        queue_work(work_queue, &fir_work);
        queue_work(work_queue, &sec_work);
        return IRQ_HANDLED;
    }
    return IRQ_NONE;
}

static struct proc_dir_entry *file = NULL;

int procShow(struct seq_file *filep, void *v) {
    printk(KERN_INFO "workseq: called show\n");
    seq_printf(filep, "Data of a 1 work: %d", fir_work.data);
    seq_printf(filep, "Data of a 2 work: %d", sec_work.data);
    return 0;
}

int _proc_open(struct inode *inode, struct file *file_inner) {
    printk(KERN_INFO "workseq: called open\n");
    return single_open(file_inner, procShow, NULL);
}

int _proc_release(struct inode *inode, struct file *file_inner) {
    printk(KERN_INFO "workseq: called release\n");
    return single_release(inode, file_inner);
}

static struct proc_ops fops = {
    proc_read : seq_read,
    proc_open : _proc_open,
    proc_release : _proc_release
};

static int __init work_queue_init(void) {
    int ret = request_irq(IRQ, handler, IRQF_SHARED, "handler", &dev_id);
    if (ret) {
        printk(KERN_ERR "workseq: handler wasn't registered\n");
        return ret;
    }

    if (!(work_queue = create_singlethread_workqueue("work_queue"))) {
        free_irq(IRQ, &dev_id);
        printk(KERN_INFO "workseq: work_queue wasn't created");
        return -ENOMEM;
    }

    INIT_WORK(&fir_work, queue_function_fir);
    INIT_WORK(&sec_work, queue_function_sec);

    if (!proc_create("work_queue", 0666, file, &fops)) {
        printk(KERN_INFO "workseq: cannot proc_create!\n");
        return -1;
    }

    printk(KERN_INFO "workseq: module loaded\n");
    return 0;
}

static void __exit work_queue_exit(void) {
    flush_workqueue(work_queue);
    destroy_workqueue(work_queue);
    free_irq(IRQ, &dev_id);
    remove_proc_entry("work_queue", NULL);
    printk(KERN_INFO "workseq: module unloaded\n");
}

module_init(work_queue_init);
module_exit(work_queue_exit);