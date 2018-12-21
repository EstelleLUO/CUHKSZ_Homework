#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/fs.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include "ioc_hw5.h"

MODULE_LICENSE("GPL");
#define PREFIX_TITLE "OS_AS5"

#define IRQ_NUM  1

//DMA
#define DMA_BUFSIZE 64
#define DMASTUIDADDR 0x0        // Student ID
#define DMARWOKADDR 0x4         // RW function complete
#define DMAIOCOKADDR 0x8        // ioctl function complete
#define DMAIRQOKADDR 0xc        // ISR function complete
#define DMACOUNTADDR 0x10       // interrupt count function complete
#define DMAANSADDR 0x14         // Computation answer
#define DMAREADABLEADDR 0x18    // READABLE variable for synchronize
#define DMABLOCKADDR 0x1c       // Blocking or non-blocking IO
#define DMAOPCODEADDR 0x20      // data.a opcode
#define DMAOPERANDBADDR 0x21    // data.b operand1
#define DMAOPERANDCADDR 0x25    // data.c operand2
void *dma_buf;

/* To obtain device major and minor number*/
static int dev_major;
static int dev_minor;
static struct cdev *dev_cdevp = NULL;

// Declaration for file operations
static ssize_t drv_read(struct file *filp, char __user *buffer, size_t ss, loff_t *lo);
static int drv_open(struct inode *ii, struct file *ff);
static ssize_t drv_write(struct file *filp, const char __user *buffer, size_t ss, loff_t *lo);
static int drv_release(struct inode *ii, struct file *ff);
static long drv_ioctl( struct file *filp, unsigned int cmd, unsigned long arg);

// cdev file_operations
static struct file_operations fops = {
      owner: THIS_MODULE,
      read: drv_read,
      write: drv_write,
      unlocked_ioctl: drv_ioctl,
      open: drv_open,
      release: drv_release,
};

// in and out function
void myoutc(unsigned char data,unsigned short int port);
void myouts(unsigned short data,unsigned short int port);
void myouti(unsigned int data,unsigned short int port);
unsigned char myinc(unsigned short int port);
unsigned short myins(unsigned short int port);
unsigned int myini(unsigned short int port);

// Arithmetic funciton
static void drv_arithmetic_routine(struct work_struct* ws);


// Input and output data from/to DMA
void myoutc(unsigned char data,unsigned short int port) {
    *(volatile unsigned char*)(dma_buf+port) = data;
}
void myouts(unsigned short data,unsigned short int port) {
    *(volatile unsigned short*)(dma_buf+port) = data;
}
void myouti(unsigned int data,unsigned short int port) {
    *(volatile unsigned int*)(dma_buf+port) = data;
}
unsigned char myinc(unsigned short int port) {
    return *(volatile unsigned char*)(dma_buf+port);
}
unsigned short myins(unsigned short int port) {
    return *(volatile unsigned short*)(dma_buf+port);
}
unsigned int myini(unsigned short int port) {
    return *(volatile unsigned int*)(dma_buf+port);
}

 static int drv_open(struct inode *ii, struct file *ff) {
	try_module_get(THIS_MODULE);
    	printk("%s:%s(): device open\n", PREFIX_TITLE, __func__);
	return 0;
 }

 static int drv_release(struct inode *ii, struct file *ff) {
	module_put(THIS_MODULE);
    	printk("%s:%s(): device close\n", PREFIX_TITLE, __func__);
	return 0;
 }


/* Read Operation for Device*/
 static ssize_t drv_read(struct file *filp, char __user *buffer, size_t ss, loff_t *lo)
 {
    unsigned int answer;
    ssize_t result = 0;

    //Obtain the answer
    answer = myini( DMAANSADDR);
    printk("%s:%s(): ans = %d\n",PREFIX_TITLE,__func__,answer);

    if (raw_copy_to_user(buffer, &answer, ss) < 0) {
        printk("%s:%s(): ##Error## ans = %d, raw_copy_to_user failed.\n",PREFIX_TITLE,__func__,answer);
        result = -EFAULT;
        goto out;
    }
    result = ss;

out:
    //Reset answer and variable
    myouti(0, DMAANSADDR);
    myouti(0, DMAREADABLEADDR);
    return result;
 }

/* Write Operation for Device*/
 static ssize_t drv_write(struct file *filp, const char __user *buffer, size_t ss, loff_t *lo) {
    char *data;
    ssize_t result;
    unsigned int blockMode, readable;
    struct work_struct *arithmetic_work;

    typedef struct {
        char a;
        int b;
        short c;
    } DataIn;


    DataIn* dataIn;


    result = 0;
    data = kmalloc(sizeof(char) * ss, GFP_KERNEL);

    if (!data) return -ENOMEM;

    if (raw_copy_from_user(data, buffer, ss) < 0) {
        printk("%s:%s(): ##Error## raw_copy_from_user failed.\n",PREFIX_TITLE,__func__);
        result = -EFAULT;
        goto out;
    }


    dataIn = (DataIn*)data;

    myoutc( dataIn->a , DMAOPCODEADDR);
    myouti( dataIn->b , DMAOPERANDBADDR);
    myouts( dataIn->c ,DMAOPERANDCADDR);

     /* Queue Work to System Queue*/
     printk("%s:%s():queue work\n",PREFIX_TITLE,__func__);
     arithmetic_work =  kmalloc(sizeof(typeof(*arithmetic_work)), GFP_KERNEL);
     INIT_WORK(arithmetic_work, drv_arithmetic_routine);
     schedule_work(arithmetic_work);

     blockMode = myini(DMABLOCKADDR);

     //Blocking mode
     if (blockMode ==1){
        printk("%s:%s():block \n",PREFIX_TITLE,__func__);
        readable = myini( DMAREADABLEADDR);
        while (readable ==0){
                msleep(1000);
                readable = myini( DMAREADABLEADDR);
        }
     }

out:
    kfree(data);
    return result;
 }

/* Ioctl Setting for device*/
 static long drv_ioctl( struct file *filp, unsigned int cmd, unsigned long arg) {
    int err = 0, result = 0;
    int value =0;

    //Decode the command and deal with exceptions
    if (_IOC_TYPE(cmd) != HW5_IOC_MAGIC || _IOC_NR(cmd) > HW5_IOC_MAXNR) return -1;

    if (_IOC_DIR(cmd) & _IOC_READ) {
        err = !access_ok(VERIFY_WRITE, (void __user*)arg, _IOC_SIZE(cmd));
    } else if (_IOC_DIR(cmd) & _IOC_WRITE) {
        err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    }

    //return error
    if (err)
        return -EFAULT;
    
    //Execution for a specific command
    switch (cmd) {
       case HW5_IOCSETSTUID:
            result = __get_user(value, (int __user *)arg);
            myouti(value,DMASTUIDADDR);
            printk("%s:%s(): My STUID is = %d\n",PREFIX_TITLE, __func__, value);
            break;
       case HW5_IOCSETRWOK:
            result = __get_user(value, (int __user *)arg);
            myouti(value,DMARWOKADDR);
            printk("%s:%s(): RW OK\n",PREFIX_TITLE, __func__);
            break;
       case HW5_IOCSETIOCOK:
            result = __get_user(value, (int __user *)arg);
            myouti(value,DMAIOCOKADDR);
            printk("%s:%s(): IOC OK\n", PREFIX_TITLE,__func__);
            break;
       case HW5_IOCSETIRQOK:
            result = __get_user(value, (int __user *)arg);
            myouti(value,DMAIRQOKADDR);
            printk("%s:%s(): IRQ OK\n",PREFIX_TITLE, __func__);
            break;

       case HW5_IOCSETBLOCK:
            result = __get_user(value, (int __user *)arg);
            myouti(value,DMABLOCKADDR);
            if (value == 1){
                printk("%s:%s(): Blocking IO\n",PREFIX_TITLE, __func__);
            }else{ 
                //value ==0
                printk("%s:%s(): Non-Blocking IO\n", PREFIX_TITLE,__func__);
            }
            break;
       case HW5_IOCWAITREADABLE:
            value = myini(DMAREADABLEADDR);
            while (value ==0){
                msleep(1000);
                value = myini( DMAREADABLEADDR);
            }
            printk("%s:%s(): wait readable %d\n",PREFIX_TITLE, __func__, value);
            result = __put_user(value, (int __user *)arg);

            break;
       default:
            return -1;
    }
    return result;
 }



/* Same as in test.c*/
 int prime(int base, short nth){
    int fnd=0;
    int i, num, isPrime;

    num = base;
    while(fnd != nth) {
        isPrime=1;
        num++;
        for(i=2;i<=num/2;i++) {
            if(num%i == 0) {
                isPrime=0;
                break;
            }
        }

        if(isPrime) {
            fnd++;
        }
    }
    return num;
}


 static void drv_arithmetic_routine(struct work_struct *ws){

    int ans;
    struct dataIn {
        char a;
        int b;
        short c;
    };

    struct dataIn data;

    data.a = myinc( DMAOPCODEADDR);
    data.b = myini(DMAOPERANDBADDR);
    data.c = myins( DMAOPERANDCADDR);

    switch(data.a) {
        case '+':
            ans=data.b+data.c;
            break;
        case '-':
            ans=data.b-data.c;
            break;
        case '*':
            ans=data.b*data.c;
            break;
        case '/':
            ans=data.b/data.c;
            break;
        case 'p':
            ans = prime(data.b, data.c);
            break;
        default:
            ans=0;
    }

    printk("%s:%s(): %d %c %hd = %d \n",PREFIX_TITLE,__func__, data.b, data.a,data.c, ans);

    /// key write back to dma
    myouti(ans, DMAANSADDR);
    myouti(1, DMAREADABLEADDR);

 }


static void initDMA(void){
    myouti(0, DMASTUIDADDR);
    myouti(0, DMARWOKADDR);
    myouti(0, DMAIOCOKADDR);
    myouti(0, DMAIRQOKADDR);
    myouti(0, DMACOUNTADDR);
    myouti(0, DMAANSADDR);
    myouti(0, DMAREADABLEADDR);
    myouti(0, DMABLOCKADDR);
    myoutc('0', DMAOPCODEADDR);
    myouti(0, DMAOPERANDBADDR);
    myouts(0, DMAOPERANDCADDR);
}



irqreturn_t keyboard_interrupt(int irq, void *dev_id){
  unsigned int count;
  count = myini( DMACOUNTADDR);
  ++count ;
  myouti( count , DMACOUNTADDR);

  return IRQ_HANDLED;
}

static int __init init_modules(void)
{
    printk("%s:%s():...............Start...............\n", PREFIX_TITLE, __func__);

    dev_t dev;
    int result;

    printk("%s:%s():request_irq %d return %d \n",PREFIX_TITLE,__func__,IRQ_NUM, 0);

    //Register irq into ISR
    result = request_irq(IRQ_NUM, keyboard_interrupt, IRQF_SHARED, "keyboard_IRQ_Count_device",  (void *)&keyboard_interrupt);

    //Failed
    if (result<0) {
        printk("request_irq() failed (%d)\n", result);
    }

    //Allocate a range of character device numbers
    result = alloc_chrdev_region(&dev, 0, 1, "mydev"); //equals to 0 when sucessfully build

    //Failed
    if (result<0) {
        printk("%s:%s():can't alloc chrdev\n",PREFIX_TITLE,__func__ );
        return result;
    }

    dev_major = MAJOR(dev);
    dev_minor = MINOR(dev);
    printk("%s:%s():register chrdev(%d,%d)\n",PREFIX_TITLE, __func__ , dev_major, dev_minor);


    dev_cdevp = kmalloc(sizeof(struct cdev), GFP_KERNEL);
    if (dev_cdevp == NULL) {
        printk("%s:%s():kmalloc *dev_cdevp failed\n",PREFIX_TITLE,__func__);
        goto failed;
    }

    /* Init cdev and make it alive */
    //Bind cdev and file_operations
    cdev_init(dev_cdevp, &fops);
    dev_cdevp->owner = THIS_MODULE;

    /* Add chr device*/
    result = cdev_add(dev_cdevp, MKDEV(dev_major, dev_minor), 1); 
    if (result < 0) {
        printk("%s:%s():add chr dev failed\n",PREFIX_TITLE,__func__ );
        goto failed;
    }

    /* Allocate DMA buffer */
    dma_buf = kmalloc(DMA_BUFSIZE, GFP_KERNEL);
    if (dma_buf == NULL) {
        printk("%s:%s():kmalloc *dma_buf failed\n",PREFIX_TITLE,__func__);
        goto failed;
    }else{
        printk("%s:%s():allocate dma buffer\n",PREFIX_TITLE, __func__);
    }

    initDMA();

    return 0;

failed:
    if (dev_cdevp) {
        kfree(dev_cdevp);
        dev_cdevp = NULL;
    }
    return 0;
}


static void exit_modules(void)
{
    dev_t dev;

    /* Free DMA buffer when exit modules */
    free_irq(IRQ_NUM, (void *)&keyboard_interrupt );

    /* Free work routine */
    dev = MKDEV(dev_major, dev_minor);
    if (dev_cdevp) {
        cdev_del(dev_cdevp);
        kfree(dev_cdevp);
    }

    printk("%s:%s():interrupt count=%d\n",PREFIX_TITLE,__func__,myini( DMACOUNTADDR));

    if (dma_buf != NULL){
        kfree(dma_buf);
        printk("%s:%s():free dma buffer\n",PREFIX_TITLE, __func__);
    }

    unregister_chrdev_region(dev, 1);
    printk("%s:%s():unregister chrdev\n",PREFIX_TITLE, __func__);
    printk("%s:%s():..............End..............\n", PREFIX_TITLE, __func__);
}


module_init(init_modules);
module_exit(exit_modules);