#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/slab.h>

MODULE_DESCRIPTION("Simple no-dev filesystem (VFS)");
MODULE_AUTHOR("Ahmad Khalid Karimzai");
MODULE_LICENSE("GPL");

#define MYFS_BLOCKSIZE		4096
#define MYFS_BLOCKSIZE_BITS	12
#define MYFS_MAX_CACHE 		12
#define MYFS_MAGIC			0xbeefcafe
#define LOG_LEVEL			KERN_ALERT

struct myfs_inode_info {
	struct inode *inode_ptr;
};

static struct kmem_cache *inode_cache = NULL;
static struct myfs_inode_info **inode_ptrs = NULL;
static size_t cached_count = 0;

static const struct super_operations myfs_ops = {
	.statfs		= simple_statfs,
	.drop_inode	= generic_drop_inode,
};

static const struct file_operations myfs_file_operations = {
	.read_iter      = generic_file_read_iter,
	.write_iter     = generic_file_write_iter,
	.mmap           = generic_file_mmap,
	.llseek         = generic_file_llseek,
};

static const struct inode_operations myfs_file_inode_operations = {
	.getattr        = simple_getattr,
};

static struct myfs_inode_info *cache_get_inode(void) 
{
    if (cached_count == MYFS_MAX_CACHE)
        return NULL;

    return inode_ptrs[cached_count++] = kmem_cache_alloc(inode_cache, GFP_KERNEL);
}

struct inode *myfs_get_inode(struct super_block *sb, const struct inode *dir,
		int mode)
{
	struct inode *inode = new_inode(sb);
	struct myfs_inode_info *own_cache = NULL;

	if (!inode)
		return NULL;

	inode_init_owner(&init_user_ns, inode, dir, mode);
	inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);
	inode->i_ino = get_next_ino();

	own_cache = cache_get_inode();
	if (own_cache) {
		own_cache->inode_ptr = inode;
	}
	inode->i_private = own_cache;

	if (S_ISDIR(mode)) {
		inode->i_op = &simple_dir_inode_operations;
		inode->i_fop = &simple_dir_operations;
		inc_nlink(inode);
	}

	if (S_ISREG(mode)) {
		inode->i_op = &myfs_file_inode_operations;
		inode->i_fop = &myfs_file_operations;
	}

	return inode;
}

static int myfs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct inode *root_inode;
	struct dentry *root_dentry;

	sb->s_maxbytes = MAX_LFS_FILESIZE;
	sb->s_blocksize = MYFS_BLOCKSIZE;
	sb->s_blocksize_bits = MYFS_BLOCKSIZE_BITS;
	sb->s_magic = MYFS_MAGIC;
	sb->s_op = &myfs_ops;

	root_inode = myfs_get_inode(sb, NULL,
			S_IFDIR | S_IRWXU | S_IRGRP |
			S_IXGRP | S_IROTH | S_IXOTH);

	printk(LOG_LEVEL "myfs: root inode has %d link(s)\n", root_inode->i_nlink);

	if (!root_inode)
		return -ENOMEM;

	root_dentry = d_make_root(root_inode);
	if (root_dentry)
	{
		sb->s_root = root_dentry;
		return 0;
	}

	iput(root_inode);
	return -ENOMEM;
}

static struct dentry *myfs_mount(struct file_system_type *fs_type,
		int flags, const char *dev_name, void *data)
{
	return mount_nodev(fs_type, flags, data, myfs_fill_super);
}

static void free_own_icache(void)
{
	size_t i = 0;
	for (; i < cached_count; i++)
		kmem_cache_free(inode_cache, inode_ptrs[i]);
}

static struct file_system_type myfs_fs_type = {
	.owner		= THIS_MODULE,
	.name		= "myfs",
	.mount		= myfs_mount,
	.kill_sb	= kill_litter_super,
};

static int __init myfs_init(void)
{
	int err;

	err = register_filesystem(&myfs_fs_type);
	if (err) {
		printk(LOG_LEVEL "myfs: register_filesystem failed\n");
		return err;
	}

	inode_ptrs = kmalloc(sizeof(struct myfs_inode_info *) * MYFS_MAX_CACHE, GFP_KERNEL);
	if (inode_ptrs == NULL)
	{
		printk(LOG_LEVEL "myfs: out of memory!\n");
		return -ENOMEM;
	}

	inode_cache = kmem_cache_create("myfs", sizeof(struct myfs_inode_info), 0, 0, NULL);
	if (inode_cache == NULL)
	{
		kfree(inode_ptrs);
		printk(LOG_LEVEL "myfs: out of memory!\n");
		return -ENOMEM;
	}

	printk(KERN_INFO "myfs: Loaded!\n");
	return 0;
}

static void __exit myfs_exit(void)
{
	int err;

	free_own_icache();
	kmem_cache_destroy(inode_cache);
	kfree(inode_ptrs);

	err = unregister_filesystem(&myfs_fs_type);
	if (err) {
		printk(LOG_LEVEL "myfs: unregister_filesystem failed\n");
	}
}

module_init(myfs_init);
module_exit(myfs_exit);