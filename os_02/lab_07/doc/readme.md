**SuperBlock :- The superblock contains only metainformation and is used to write and read metadata from the disk (inodes, directory entries). A superblock (and implicitly the `struct super_block` structure) will contain information about the block device used, the list of inodes, a pointer to the inode of the file system root directory, and a pointer to the superblock operations.**

```
struct super_block {
        //...
        dev_t                   s_dev;              /* identifier */
        unsigned char           s_blocksize_bits;   /* block size in bits */
        unsigned long           s_blocksize;        /* block size in bytes */
        unsigned char           s_dirt;             /* dirty flag */
        loff_t                  s_maxbytes;         /* max file size */
        struct file_system_type *s_type;            /* filesystem type */
        struct super_operations *s_op;              /* superblock methods */
        //...
        unsigned long           s_flags;            /* mount flags */
        unsigned long           s_magic;            /* filesystem’s magic number */
        struct dentry           *s_root;            /* directory mount point */
        //...
        char                    s_id[32];           /* informational name */
        void                    *s_fs_info;         /* filesystem private info */
};
```
