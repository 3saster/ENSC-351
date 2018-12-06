#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#define PTR_SIZE 8
static char getptr_freespace[1024];

//Helper function to convert a char to its appropriate hex value (a/A -> 10, b/B -> 11, etc.)
static int hex_value(char c)
{
    if      ( 'a' <= c && c <= 'f' ) return c-'a'+10;
    else if ( 'A' <= c && c <= 'F' ) return c-'A'+10;
    else if ( '0' <= c && c <= '9' ) return c-'0';
    else                             return -1;
}

static ssize_t getptr_read(struct file *file, char *data, size_t length, loff_t *offset_in_file)
{
    printk(KERN_INFO "Entered getptr_read.\n");
    if( length == PTR_SIZE )
    {
        printk(KERN_INFO "1024 free bytes at location: 0x%p\n", getptr_freespace);

        //Store pointer as a string of hex values
        char pointerValue[2*PTR_SIZE];
        snprintf(pointerValue, 2*PTR_SIZE+1, "%p", getptr_freespace);

        //Convert pair of hex-chars to one byte
        //i.e. ff -> 255, 05 -> 5, 50 -> 80, etc.
        char kernel_data[PTR_SIZE];
        int i;
        for(i=0; i<PTR_SIZE; i++)
        {
            kernel_data[i] = 16*hex_value(pointerValue[2*i]) + hex_value(pointerValue[2*i + 1]);
        }

        unsigned long copied = copy_to_user(data, kernel_data, PTR_SIZE); //copy_to_user returns number of non-copied Bytes (thus zero on success, non-zero on failure)
        if(copied == 0)
        {
            return 0;
        }
        else
        {
            printk(KERN_ERR "Unable to copy address to user-space.\n");
            return -ENOMEM;
        }
    }
    else
    {
        printk(KERN_ERR "Error in getptr: you must read exactly %i bytes.\n", PTR_SIZE);
        return -EFAULT;
    }
}

static int getptr_open(struct inode* inode_pointer, struct file* file_pointer)
{
    printk(KERN_INFO "Entered getptr_open.\n");
    return 0;
}

static int getptr_release(struct inode* inode_pointer, struct file* file_pointer)
{
    printk(KERN_INFO "Entered getptr_release.\n");
    return 0;
}

static ssize_t getptr_write(struct file *file, const char *data, size_t length, loff_t *offset_in_file)
{
    printk(KERN_NOTICE "Attempted to write to getptr driver (not permitted).\n");
    return -EPERM;
}

static int major_num;
static struct device* device_data;
static struct class* class_stuff;

static struct file_operations file_ops =
{
    .open    = getptr_open,
    .release = getptr_release,
    .read    = getptr_read,
    .write   = getptr_write,
};

static int __init getptr_init(void)
{
    printk(KERN_INFO "Starting getptr.\n");

    major_num = register_chrdev(0, "getptr", &file_ops);
    class_stuff = class_create(THIS_MODULE, "getptr class");
    device_data = device_create(class_stuff, NULL, MKDEV(major_num, 0), NULL, "getptrdev");

    return 0;
}

static void __exit getptr_exit(void)
{
    device_destroy(class_stuff, MKDEV(major_num, 0) );
    class_destroy(class_stuff);
    unregister_chrdev(major_num, "getptr");

    printk(KERN_INFO "Exiting getptr.\n");
}

module_init(getptr_init);
module_exit(getptr_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Paul Bologea & Juan Rivera");
MODULE_DESCRIPTION("getptr driver for final project");
