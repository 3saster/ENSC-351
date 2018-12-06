#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#define PTR_SIZE 8
static char* peek_readptr = NULL;

static ssize_t peek_read(struct file *file, char *data, size_t length, loff_t *offset_in_file)
{
    printk(KERN_INFO "Entered peek_read.\n");
    if( length == 1 )
    {
        if(peek_readptr == NULL)
        {
            printk(KERN_INFO "peek driver has nothing to read (try writing an address first).\n");
            return 0;
        }
        else
        {
            unsigned long copied = copy_to_user(data, peek_readptr, 1);  //copy_to_user returns number of non-copied Bytes (thus zero on success, non-zero on failure)
            printk(KERN_INFO "peek at 0x%.*x\n", PTR_SIZE*2, peek_readptr);
            if(copied == 0)
            {
                printk(KERN_INFO "peek at address 0x%.*x: %i\n", PTR_SIZE*2,peek_readptr, *peek_readptr);
                return 0;
            }
            else
            {
                printk(KERN_ERR "Unable to copy peeked value to user-space.\n");
                return -ENOMEM;
            }
        }
    }
    else
    {
        printk(KERN_ERR "Error in peek: you must read exactly 1 byte.\n");
        return -1;
    }
}

static ssize_t peek_write(struct file *file, const char *data, size_t length, loff_t *offset_in_file)
{
    printk(KERN_INFO "Entered peek_write.\n");
    if( length == PTR_SIZE )
    {
        //Copy data to kernel-space
        char kernel_data[PTR_SIZE+1];
        kernel_data[PTR_SIZE] = '\0';
        unsigned long copied = copy_from_user(kernel_data, data, PTR_SIZE);
        if(copied != 0)
        {
            printk(KERN_ERR "Error in peek_write: could not read data given.\n");
            return -EFAULT;
        }

        //Convert address to an int
        long int read_address = 0;
        int i;
        for(i=0; i<PTR_SIZE; i++)
        {
            read_address <<= 8;
            read_address += kernel_data[i];
        }
        peek_readptr = (char*) read_address;
        printk(KERN_INFO "peek address set to 0x%.*x\n", PTR_SIZE*2,peek_readptr);
        return PTR_SIZE-copied; //0 causes infinite loops
    }
    else
    {
        printk(KERN_ERR "Error in peek: you must write exactly %i bytes (%i bytes were given).\n", PTR_SIZE, length);
        return -EFAULT;
    }
}

static int peek_open(struct inode* inode_pointer, struct file* file_pointer)
{
    printk(KERN_INFO "Entered peek_open.\n");
    return 0;
}

static int peek_release(struct inode* inode_pointer, struct file* file_pointer)
{
    printk(KERN_INFO "Entered peek_release.\n");
    return 0;
}

static int major_num;
static struct device* device_data;
static struct class* class_stuff;

static struct file_operations file_ops =
{
    .open    = peek_open,
    .release = peek_release,
    .read    = peek_read,
    .write   = peek_write,
};

static int __init peek_init(void)
{
    printk(KERN_INFO "Starting peek.\n");

    major_num = register_chrdev(0, "peek", &file_ops);
    class_stuff = class_create(THIS_MODULE, "peek class");
    device_data = device_create(class_stuff, NULL, MKDEV(major_num, 0), NULL, "peekdev");

    return 0;
}

static void __exit peek_exit(void)
{
    device_destroy(class_stuff, MKDEV(major_num, 0) );
    class_destroy(class_stuff);
    unregister_chrdev(major_num, "peek");

    printk(KERN_INFO "Exiting peek.\n");
}

module_init(peek_init);
module_exit(peek_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Paul Bologea & Juan Rivera");
MODULE_DESCRIPTION("peek driver for final project");
