#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#define PTR_SIZE 8
#define DATA_SIZE 9
static char* poke_writeptr = NULL;

static ssize_t poke_read(struct file *file, char *data, size_t length, loff_t *offset_in_file)
{
    printk(KERN_NOTICE "Attempted to read to getptr driver (not permitted).\n");
    return -EPERM;
}

static ssize_t poke_write(struct file *file, const char *data, size_t length, loff_t *offset_in_file)
{
    printk(KERN_INFO "Entered poke_write.\n");
    if( length == DATA_SIZE )
    {
        //Copy data to kernel-space
        char kernel_data[DATA_SIZE+1];
        kernel_data[DATA_SIZE] = '\0';
        unsigned long copied = copy_from_user(kernel_data, data, DATA_SIZE);
        if(copied != 0)
        {
            printk(KERN_ERR "Error in poke_write: could not read data given.\n");
            return -EFAULT;
        }

        //Convert address to an int
        long int write_address = 0;
        int i;
        for(i=0; i<PTR_SIZE; i++)
        {
            write_address <<= 8;
            write_address += kernel_data[i];
        }
        poke_writeptr = (char*) write_address;
        printk(KERN_INFO "poke attempting to set address 0x%.*x to %i\n", PTR_SIZE*2,poke_writeptr, kernel_data[PTR_SIZE]);
        //This is intentionally a = sign
        if( (*poke_writeptr = kernel_data[PTR_SIZE]) )
        {
            printk(KERN_INFO "poke set address 0x%.*x to %i.\n", PTR_SIZE*2, poke_writeptr, kernel_data[PTR_SIZE] );
            return 1;
        }
        else
        {
            printk(KERN_ERR "Error in poke_write: could not write data to specified address given.\n");
            return -EPERM;
        }
    }
    else
    {
        printk(KERN_ERR "Error in poke: you must write exactly %i bytes (%i bytes were given).\n", DATA_SIZE, length);
        return -EFAULT;
    }
}

static int poke_open(struct inode* inode_pointer, struct file* file_pointer)
{
    printk(KERN_INFO "Entered poke_open.\n");
    return 0;
}

static int poke_release(struct inode* inode_pointer, struct file* file_pointer)
{
    printk(KERN_INFO "Entered poke_release.\n");
    return 0;
}

static int major_num;
static struct device* device_data;
static struct class* class_stuff;

static struct file_operations file_ops =
{
    .open    = poke_open,
    .release = poke_release,
    .read    = poke_read,
    .write   = poke_write,
};

static int __init poke_init(void)
{
    printk(KERN_INFO "Starting poke.\n");

    major_num = register_chrdev(0, "poke", &file_ops);
    class_stuff = class_create(THIS_MODULE, "poke class");
    device_data = device_create(class_stuff, NULL, MKDEV(major_num, 0), NULL, "pokedev");

    return 0;
}

static void __exit poke_exit(void)
{
    device_destroy(class_stuff, MKDEV(major_num, 0) );
    class_destroy(class_stuff);
    unregister_chrdev(major_num, "poke");

    printk(KERN_INFO "Exiting poke.\n");
}

module_init(poke_init);
module_exit(poke_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Paul Bologea & Juan Rivera");
MODULE_DESCRIPTION("poke driver for final project");
