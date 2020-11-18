// Copyright [2020] <Puchkov Kyryll>
#include <linux/init.h>                                         // Macros used to mark up functions e.g., __init __exit
#include <linux/module.h>                                       // Core header for loading LKMs into the kernel
#include <linux/kernel.h>                                       // Contains types, macros, functions for the kernel

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Puchkov Kyryll");
MODULE_DESCRIPTION("A simple example Linux module.");
MODULE_VERSION("0.1");

// Загрузка
// __init — макрос
static int __init lkm_example_init(void) {
    printk(KERN_INFO "Hi! It's my first kernel module.!\n");
    return 0;
}

// Выгрузка
static void __exit lkm_example_exit(void) {
    printk(KERN_INFO "Goodbye!\n");
}

// Макрос загрузки модуля
module_init(lkm_example_init);
// Макрос выгрузки модуля
module_exit(lkm_example_exit);
