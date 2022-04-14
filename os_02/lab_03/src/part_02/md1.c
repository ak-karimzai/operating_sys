#include <linux/init.h>
#include <linux/module.h>

#include "md.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ahmad Khalid Karimzai");

char *md1_data = "Msg from md1!";

static int __init md_init(void) {
    printk("Module md1 Loaded\n");
    return 0;
}

static void __exit md_exit(void) { 
    printk("Module md1 unloaded!\n");
}

extern char *md1_proc(void) { return md1_data; }

extern char *md1_noexport(void) { return md1_data; }


EXPORT_SYMBOL(md1_data);
EXPORT_SYMBOL(md1_proc);

module_init(md_init);
module_exit(md_exit);