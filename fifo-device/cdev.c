// Copyright [2020] <Puchkov Kyryll>
#include <linux/init.h>                                                 // Macros used for __init, __exit functions
#include <linux/module.h>                                               // Core header for loading LKMs into the kernel
#include <linux/device.h>                                               // Header to support the kernel Driver Model
#include <linux/kernel.h>                                               // Contains types, macros, functions for the kernel
#include <linux/fs.h>                                                   // Header for the Linux file system support
#include <linux/uaccess.h>                                              // Required for the copy to user function
#include <linux/slab.h>


#define DEVICE_NAME         "cdev"                                   ///< Dev name as it appears in /proc/devices
#define CLASS_NAME          "lkm"                                       //< The device class -- this is a character device driver
#define MESSAGE_SIZE        (size_t)(10 * PAGE_SIZE)

MODULE_LICENSE("GPL");                                                  ///< The license type -- this affects available functionality
MODULE_AUTHOR("Puchkov Kyryll");                                        ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple fifo driver for the kernel module");       ///< The description -- see modinfo
MODULE_VERSION("0.1");                                                  ///< A version number to inform users

static int    majorNumber;                                              ///< Stores the device number -- determined automatically
static char*  msg_ptr;
static int    numberOpens = 0;                                          ///< Counts the number of times the device is opened
static struct class*  fifoClass  = NULL;                                ///< The device-driver class struct pointer
static struct device* fifoDevice = NULL;                                ///< The device-driver device struct pointer

// The prototype functions for the character driver
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

static int __init fifodev_init(void) {
    printk(KERN_INFO "Fifodev: Initializing the character device for the LKM\n");

    // Allocate a major number for the device
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0) {
        printk(KERN_ALERT "Fifodev failed to register a major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "Fifodev: registered correctly with major number %d\n", majorNumber);

    // Register the device class
    fifoClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(fifoClass)) {                                         // Check for error and clean up
        unregister_chrdev(majorNumber, DEVICE_NAME);                    // Unregister the major number
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(fifoClass);                                   // Retrieves the error number from the pointer
    }
    printk(KERN_INFO "Fifodev: device class registered correctly\n");

    // Register the device driver
    fifoDevice = device_create(fifoClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(fifoDevice)) {                                        // Clean up
        class_destroy(fifoClass);                                    // Remove the device class
        unregister_chrdev(majorNumber, DEVICE_NAME);                    // Unregister the major number
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(fifoDevice);                                  // Retrieves the error number from the pointer
    }
    printk(KERN_INFO "Fifodev: device class created correctly\n");

    msg_ptr = kmalloc(MESSAGE_SIZE, GFP_KERNEL);
    return 0;
}

static void __exit fifodev_exit(void) {
    if (msg_ptr != NULL) {
        kfree(msg_ptr);
    }

    device_destroy(fifoClass, MKDEV(majorNumber, 0));                // Remove the device
    class_unregister(fifoClass);                                     // Unregister the device class
    class_destroy(fifoClass);                                        // Remove the device class
    unregister_chrdev(majorNumber, DEVICE_NAME);                        // Unregister the major number

    printk(KERN_INFO "Fifodev: Goodbye from the fifodev lkm!\n");
}

static int dev_open(struct inode *inodep, struct file *filep) {
    numberOpens++;
    printk(KERN_INFO "Fifodev: %d users using device(s) right now\n", numberOpens);
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    numberOpens--;
    printk(KERN_INFO "Fifodev: Device successfully closed\n");
    return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    // copy_to_user has the format ( * to, *from, size) and returns 0 on success
    if (len + *offset > MESSAGE_SIZE) {
        len = MESSAGE_SIZE - *offset;
    }
    int notReadBytes = copy_to_user(buffer + *offset, msg_ptr, len);

    *offset += len - notReadBytes;
    if (notReadBytes == 0) {            // Success
        printk(KERN_INFO "Fifodev: Sent %d characters to the user\n", len);
        return len - notReadBytes;  // clear the position to the start and return 0
    } else {
        printk(KERN_INFO "Fifodev: Failed to send %d characters to the user\n", notReadBytes);
        return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
    }
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    if (len + *offset > MESSAGE_SIZE) {
        len = MESSAGE_SIZE - *offset;
    }
    int notWrittenBytes = len - copy_from_user(msg_ptr + *offset, buffer, len);
    *offset += notWrittenBytes;
    msg_ptr[*offset] = '\0';

    printk(KERN_INFO "Fifodev: Received %zu characters from the user\n", len);
    return notWrittenBytes;
}

module_init(fifodev_init);
module_exit(fifodev_exit);
