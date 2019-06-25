#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h> // changed from asm/uaccess.h to linux/uaccess.h
#include <linux/time.h>
#include <linux/io.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/fcntl.h> /*Helps fix O_ACCMODE*/
#include <linux/sched.h> /*Helps fix TASK_UNINTERRUPTIBLE */
#include <linux/fs.h> /*Helps fix the struct intializer */

int __init init_module(void);
void __exit cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param);

#define SUCCESS 0
#define DEVICE_NAME "hsdk"// Dev name
#define BUF_LEN 80//Max length of device message

/* These parameters depend on the RPI edition */

#define BCM2708_PERI_BASE       0x3F000000 // value needs to be changed to 0x3F000000 for a RPi3. 0x20000000 works for Pi W.
#define GPIO_BASE               (BCM2708_PERI_BASE + 0x200000)	// GPIO controller

// Defines  GPIO macros to control GPIOs.
#define INP_GPIO(g)   *(gpio.addr + ((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g)   *(gpio.addr + ((g)/10)) |=  (1<<(((g)%10)*3)) //001
#define SET_GPIO_ALT(g,a) *(gpio.addr + (((g)/10))) |= (((a)<=3?(a) + 4:(a)==4?3:2)<<(((g)%10)*3))
#define GPIO_SET  *(gpio.addr + 7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR  *(gpio.addr + 10) // clears bits which are 1 ignores bits which are 0
#define GPIO_READ(g)  *(gpio.addr + 13) &= (1<<(g))

//GPIO Clock. AT
#define CLOCK_BASE              (BCM2708_PERI_BASE + 0x00101000)
#define GZ_CLK_BUSY (1 << 7)

#define SAMPLE_SIZE 25000 //50000 pts

//ADC 1
#define BIT0_ADC1 9
#define BIT1_ADC1 25
#define BIT2_ADC1 23
#define BIT3_ADC1 22
#define BIT4_ADC1 27
#define BIT5_ADC1 17
#define BIT6_ADC1 18
#define BIT7_ADC1 15
#define BIT8_ADC1 14
#define BIT9_ADC1 24

//ADC 2
#define BIT0_ADC2 20
#define BIT1_ADC2 26
#define BIT2_ADC2 16
#define BIT3_ADC2 19
#define BIT4_ADC2 13
#define BIT5_ADC2 12
#define BIT6_ADC2 7
#define BIT7_ADC2 8
#define BIT8_ADC2 6
#define BIT9_ADC2 21

//CLOCK
#define CLOCK_GPIO 4

// IO Access
struct bcm2835_peripheral {
    unsigned long addr_p;
    int mem_fd;
    void *map;
    volatile unsigned int *addr;
};

static int map_peripheral(struct bcm2835_peripheral *p);
static void unmap_peripheral(struct bcm2835_peripheral *p);
static void readScope(void);

static int major_num;
static int open_flag = 0;

static struct file_operations fops = {
       read : device_read,
       write : device_write,
       open : device_open,
       release : device_release,
       unlocked_ioctl : device_ioctl
};

static struct bcm2835_peripheral myclock = {CLOCK_BASE};

static struct bcm2835_peripheral gpio = {GPIO_BASE};

typedef struct adc_output{
    uint32_t buffer[SAMPLE_SIZE];
    uint32_t time;
}adc_out;
adc_out sampled_output;

static uint32_t *buffer_start;
static uint32_t *buffer_stop;

static int map_peripheral(struct bcm2835_peripheral *p)
{
    p->addr=(uint32_t *)ioremap(GPIO_BASE, 41*4); //41 GPIO register with 32 bit (4*8)
    return 0;
}

static void unmap_peripheral(struct bcm2835_peripheral *p) {
    iounmap(p->addr);//unmap the address
}

static void readScope(){

    int counter=0;

    struct timespec ts_start,ts_stop;

    msleep(10);

    //disable IRQ
    local_irq_disable();
    local_fiq_disable();

    //Start time
    set_current_state(TASK_UNINTERRUPTIBLE);
    getnstimeofday(&ts_start);

    while(counter<SAMPLE_SIZE){
        sampled_output.buffer[counter++]= *(gpio.addr + 13);
    }
    //Stop time
    getnstimeofday(&ts_stop);

    set_current_state(TASK_INTERRUPTIBLE);
    //enable IRQ
    local_fiq_enable();
    local_irq_enable();

    //save the time difference
    sampled_output.time=timespec_to_ns(&ts_stop)-timespec_to_ns(&ts_start);//ns resolution

    buffer_start = sampled_output.buffer;
    buffer_stop = buffer_start+sizeof(struct adc_output);
}

int init_module(void)
{

    struct bcm2835_peripheral *p=&myclock;
    int speed_id = 6; //1 for to start with 19Mhz or 6 to start with 500 MHz

    major_num = register_chrdev(0, DEVICE_NAME, &fops);

    if (major_num < 0) {
        printk(KERN_ALERT "Registering char device failed with %d\n", major_num);
        return major_num;
    }

    printk(KERN_INFO "scope: Using major number %d.\n", major_num);
    printk(KERN_INFO "scope: Run 'mknod /dev/%s c %d 0' to create character device.\n", DEVICE_NAME, major_num);

    if(map_peripheral(&gpio) == -1)
    {
        printk(KERN_ALERT "Failed to map the physical GPIO registers into the virtual memory space.\n");
        return -1;
    }

    //ADC1
    INP_GPIO(BIT0_ADC1);
    INP_GPIO(BIT1_ADC1);
    INP_GPIO(BIT2_ADC1);
    INP_GPIO(BIT3_ADC1);
    INP_GPIO(BIT4_ADC1);
    INP_GPIO(BIT5_ADC1);
    INP_GPIO(BIT6_ADC1);
    INP_GPIO(BIT7_ADC1);
    INP_GPIO(BIT8_ADC1);
    INP_GPIO(BIT9_ADC1);
    // ADC2
    INP_GPIO(BIT0_ADC2);
    INP_GPIO(BIT1_ADC2);
    INP_GPIO(BIT2_ADC2);
    INP_GPIO(BIT3_ADC2);
    INP_GPIO(BIT4_ADC2);
    INP_GPIO(BIT5_ADC2);
    INP_GPIO(BIT6_ADC2);
    INP_GPIO(BIT7_ADC2);
    INP_GPIO(BIT8_ADC2);
    INP_GPIO(BIT9_ADC2);

    //Set a clock signal on Pin 4
    p->addr=(uint32_t *)ioremap(CLOCK_BASE, 41*CLOCK_GPIO);
    INP_GPIO(CLOCK_GPIO);
    SET_GPIO_ALT(CLOCK_GPIO,0);
    // Preparing the clock

    *(myclock.addr+28)=0x5A000000 | speed_id; //Turn off the clock
    while (*(myclock.addr+28) & GZ_CLK_BUSY) {}; //Wait until clock is no longer busy (BUSY flag)
    // Set divider //divide by 50 (0x32) -- ideally 41 (29) to fall on 12MHz clock
    *(myclock.addr+29)= 0x5A000000 | (0x29 << 12) | 0;
    // And let's turn clock on
    *(myclock.addr+28)=0x5A000010 | speed_id;

    return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void){
    unregister_chrdev(major_num, DEVICE_NAME);
    unmap_peripheral(&gpio);
    unmap_peripheral(&myclock);
}

static int device_open(struct inode *inode, struct file *file)
{
    if (open_flag)
        return -EBUSY;

    open_flag++;
    printk(KERN_INFO "scope: Device has been opened.\n");
    readScope();//Read n Samples into memory
    printk(KERN_INFO "scope: returned from readScope()\n");
    try_module_get(THIS_MODULE);
    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
    open_flag--;
    module_put(THIS_MODULE);
    return 0;
}

static ssize_t device_read(struct file *file,
        char *user_buffer,
        size_t size,
        loff_t *offset)
{
    ssize_t len = min(sizeof(struct adc_output) - *offset, size);

    if (len <= 0)
        return 0;
    printk(KERN_INFO "Time it ran for: %d\n",sampled_output.time);
    /* read data from device in my_data->buffer */
    if (copy_to_user(user_buffer, sampled_output.buffer + *offset, len))
        return -EFAULT;

    *offset += len;
    return len;
}

    static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
    printk(KERN_ALERT "scope: does not support writing to the device.\n");
    return -EINVAL;
}

static long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param){
    return sampled_output.time;
}

MODULE_AUTHOR("jholtom");
MODULE_LICENSE("GPL");
MODULE_VERSION("3");
