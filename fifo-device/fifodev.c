// Copyright [2020] <Puchkov Kyryll>
#include <linux/init.h>                                                 // Macros used for __init, __exit functions
#include <linux/module.h>                                               // Core header for loading LKMs into the kernel
#include <linux/device.h>                                               // Header to support the kernel Driver Model
#include <linux/kernel.h>                                               // Contains types, macros, functions for the kernel
#include <linux/fs.h>                                                   // Header for the Linux file system support
#include <linux/uaccess.h>                                              // Required for the copy to user function

#define DEVICE_NAME         "fifodev"                                   ///< Dev name as it appears in /proc/devices
#define CLASS_NAME          "lkm"                                       //< The device class -- this is a character device driver
#define MESSAGE_SIZE        10 * 1024

MODULE_LICENSE("GPL");                                                  ///< The license type -- this affects available functionality
MODULE_AUTHOR("Puchkov Kyryll");                                        ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple fifo driver for the kernel module");       ///< The description -- see modinfo
MODULE_VERSION("0.1");                                                  ///< A version number to inform users

static int    majorNumber;                                              ///< Stores the device number -- determined automatically
// static char   message[10*  1024] = {0};                                       ///< Memory for the string that is passed from userspace
static char*  msg_ptr;
static int    numberOpens = 0;                                          ///< Counts the number of times the device is opened
/*static int    numberReaders = 0;
static int    numberWriters = 0;
static int    readBytes = 0;
static int    writtenBytes = 0;*/
static struct class*  fifoClass  = NULL;                                ///< The device-driver class struct pointer
static struct device* fifoDevice = NULL;                                ///< The device-driver device struct pointer

// The prototype functions for the character driver
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

/*  Devices are represented as file structure in the kernel.
 *  The file_operations structure from /linux/fs.h lists the callback functions
 *  that you wish to associated with your file operations using a C99 syntax structure.
 *  Char devices implement open, read, write and release calls
 */
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

/*  The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file.
 *  The __init macro means that for a built-in driver (not a LKM) the function is only
 *  used at initialization time and that it can be discarded and its memory freed up 
 *  after that point.
 *  Returns 0 if successful
 */
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
    return 0;
}

/*  The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit fifodev_exit(void) {
    device_destroy(fifoClass, MKDEV(majorNumber, 0));                // Remove the device
    class_unregister(fifoClass);                                     // Unregister the device class
    class_destroy(fifoClass);                                        // Remove the device class
    unregister_chrdev(majorNumber, DEVICE_NAME);                        // Unregister the major number

    printk(KERN_INFO "Fifodev: Goodbye from the fifodev lkm!\n");
}

/*  The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  inodep — a pointer to an inode object (defined in linux/fs.h)
 *  filep — a pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep) {
    msg_ptr = kmalloc(MESSAGE_SIZE, GFP_KERNEL);
    /* If device is open, return busy */
    /*if (numberOpens) {
        return -EBUSY;
    }
    if (filep->f_mode & FMODE_READ) {
        numberReaders++;
    }
    if (filep->f_mode & FMODE_WRITE) {
        numberWriters++;
    }*/
    numberOpens++;
    printk(KERN_INFO "Fifodev: Device has been opened %d time(s)\n", numberOpens);
    return 0;
}

/*  This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  filep — a pointer to a file object (defined in linux/fs.h)
 *  buffer — the pointer to the buffer to which this function writes the data
 *  len — the length of the buffer
 *  offset — the offset if required
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    /*
    readBytes = 0;

    if (*msg_ptr == NULL) {
        msg_ptr = message;
    }
    while ((len > 0) && (*msg_ptr != NULL)) {
        // Read one symbol from kernel ram to user ram
        ssize_t error = put_user(*(msg_ptr++), buffer++);
        if (error == -EFAULT) {
            printk(KERN_INFO "Fifodev: failed to write a byte from kernel to user ram by put_user()");
        }
        len--;
        readBytes++;
    }

    return readBytes;*/

    int error_count = 0;
    // copy_to_user has the format ( * to, *from, size) and returns 0 on success
    error_count = copy_to_user(buffer, msg_ptr, MESSAGE_SIZE);

    if (error_count == 0) {            // Success
        printk(KERN_INFO "Fifodev: Sent %d characters to the user\n", size_of_message);
        return (size_of_message = 0);  // clear the position to the start and return 0
    } else {
        printk(KERN_INFO "Fifodev: Failed to send %d characters to the user\n", error_count);
        return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
    }
}

/*  This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the snprintf() function along with the length of the string.
 *  filep — a pointer to a file object
 *  buffer — the buffer to that contains the string to write to the device
 *  len — the length of the array of data that is being passed in the const char buffer
 *  offset — the offset if required
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    snprintf(message, sizeof(message), "%s(%zu chars)", buffer, len);   // appending received string with its length
    size_of_message = strlen(message);                                  // store the length of the stored message
    printk(KERN_INFO "Fifodev: Received %zu characters from the user\n", len);
    return len;
}

/*  The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  inodep — a pointer to an inode object (defined in linux/fs.h)
 *  filep — a pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep) {
    /*if (filep->f_mode & FMODE_READ) {
        numberReaders--;
    }
    if (filep->f_mode & FMODE_WRITE) {
        numberWriters--;
    }*/
    // numberOpens--;
    kfree(msg_ptr);

    printk(KERN_INFO "Fifodev: Device successfully closed\n");
    return 0;
}

/*  A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function
 */
module_init(fifodev_init);
module_exit(fifodev_exit);
