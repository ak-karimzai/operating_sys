#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <asm/io.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ahmad Khalid Karimzai");
MODULE_DESCRIPTION("tasklet");

#define IRQ 1 /* keyboard irq */
#define SCANCODE_RELEASED_MASK	0x80

static int dev_id;

char my_tasklet_data[] = "KEYBOARD INTERRUPT";
struct tasklet_struct my_tasklet;

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

void my_tasklet_function(unsigned long data)
{
	int scancode = inb(0x60);
	if (scancode < 103) {
		printk(KERN_INFO "my_tasklet: %s: scancode: %d, inputed char: %c\n", (char *)data, scancode, get_ascii(scancode));
	}
}

irqreturn_t my_irq_handler(int irq, void *dev)
{
	if (irq == IRQ) {
		tasklet_schedule(&my_tasklet);
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}

static int __init my_tasklet_init(void)
{
	int ret = request_irq(IRQ, my_irq_handler, IRQF_SHARED, "my_irq_handler", 
                        (void *)(my_irq_handler));
	if (ret) {
		printk(KERN_ERR "my_tasklet: my_irq_handler wasn't register\n");
	} else {
		printk(KERN_INFO "my_tasklet: module loaded\n");
        tasklet_init(&my_tasklet, my_tasklet_function, (void *)(my_irq_handler));
	}
	return ret;
}

static void __exit my_tasklet_exit(void)
{
    free_irq(IRQ, (void *)(my_irq_handler));
    tasklet_disable(&my_tasklet);
	tasklet_kill(&my_tasklet);
	free_irq(IRQ, &dev_id);
	printk(KERN_DEBUG "my_tasklet: module unloaded\n");
}

module_init(my_tasklet_init)
module_exit(my_tasklet_exit)