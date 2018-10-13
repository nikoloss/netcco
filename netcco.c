#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

static char buf[1024];
static struct proc_dir_entry *pde;
static struct file_operations proc_ops;

static ssize_t proc_op_write(struct file *filp,
                             const char __user *buffer,
                             size_t len,
                             loff_t* offset)
{
    if(copy_from_user(buf, buffer, len)){
        return -ENOMEM;
    }
    buf[len]='\0'; //set EOF
    printk(KERN_INFO "[netcco]got:%s\n", buf);
    return len;
}

static int __init  hello_init(void){
    proc_ops.owner = THIS_MODULE;
    proc_ops.write = proc_op_write;
    pde = proc_create("netcco", 0666, NULL, &proc_ops);
    if (!pde) {
        printk(KERN_ERR "proc create failed!");
        proc_remove(pde);
        return ENOMEM;
    }
    return 0;
}

static void __exit hello_exit(void){
    if(pde){
        proc_remove(pde);
    }
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("this is a helloworld test");
MODULE_AUTHOR("Rowland");

module_init(hello_init);
module_exit(hello_exit);
