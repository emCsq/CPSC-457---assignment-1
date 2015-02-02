/*Some code was taken from the communicate.c file created by Robin Gonzalez from the tutorial. Taken from the internet: http://pages.cpsc.ucalgary.ca/~gonzalre/CPSC457/Source_Code/LKM/ */

#include <linux/fs.h> /*Needed for file operations*/
#include <linux/miscdevice.h> /*Needed for registering device*/
#include <linux/module.h> /*Needed by all modules*/
#include <linux/kernel.h> /*Needed for KERN_INFO*/
#include <linux/init.h> /*Needed for the macros*/
#include <linux/sched.h> /*Needed for the for_each_process*/
#include <linux/crc32.h> /*Needed for crc32 checksum*/
#include <asm/page.h>

static struct miscdevice device_dev;
int ret;

//Alerts that device write is not supported
static ssize_t device_write(struct file * file, const char * buf, size_t count, loff_t * off) {
  printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
  return -EINVAL;
}

//Reads from device and performs all of the tasks within it
static ssize_t device_read(struct file * file, char * buf, size_t count, loff_t * ppos) {
// 	//initializes all needed variables 
	struct task_struct *task;
	int t_state;
	long t_ptrace;
	int t_pers;
	pid_t t_pid;
	uid_t t_uid;
	long t_pgd;
	unsigned long t_thread;
	unsigned long checksum;
	char array[30];
	
	//a macro such that we are able to do the following for each task
	for_each_process(task) {

	  //handles the "mm is null" case
	  if ((task->mm == NULL)) {
	    t_pgd = 0;				//if null, set to 0
	  } else {
	    t_pgd = (long)task->mm->pgd;	//else, set properly to pgd
	  }
	  
	  //retrieves all of the fields and concats into an array that will be used for the checksum
	  t_state = task->state;	 /*State: -1 unrunnable 0 runnable, >0 stopped*/
	  memcpy(array, &t_state, 4);
	  t_ptrace = task->ptrace;		//sets to variable
	  memcpy(array+4, &t_ptrace, 4);	//copies into array
	  t_pers = task->personality;		//*repeat*
	  memcpy(array+8, &t_pers, 4);
	  t_pid = task->pid;
	  memcpy(array+12, &t_pid, 4);
	  t_uid = task->real_cred->uid;
	  memcpy(array+16, &t_uid, 4);
	  t_thread = task->thread.ip;
	  memcpy(array+20, &t_thread, 4);
	  memcpy(array+24, &t_pgd, 4);
	  
	  //computes the checksum using the concatanated array which contains all of the field values from the task struct and prints it out
	  checksum = crc32_le(0xFFFFFFFF, (const void*)array, 30);
	  checksum = checksum ^ 0xFFFFFFFF;		//XORs the checksum with -1 for the proper checksum value
	  printk("Checksum is : %lX\n", checksum);
	}
	return EINVAL;
}


/*initializes file operations for the device*/
static const struct file_operations device_fops = {
	.read = device_read, /*Register read fuction of the device driver*/
	.write = device_write, /*Registers write function of the device driver... kind of (because it's not supported in our version)*/
};

//*from the communicate.c file* displays kernel print that alerts that the kernel is being loaded
static int device_start(void) {
	printk("Loading checksum module...\n"); /*printk is kernel version of printf()*/
	device_dev.minor = MISC_DYNAMIC_MINOR; /*Initializing device parameters*/
	device_dev.name = "checksum";
	device_dev.fops = &device_fops;
	ret = misc_register(&device_dev);
	if (ret)
		printk("Unable to register...\n");
	return ret;
}

//*from the communicate.c file* displays kernel print that alerts that the kernel is being deregistered
static void device_end(void) {
	printk("Goodbye\n");
	ret = misc_deregister(&device_dev);
	printk("Unregistering....\n");
	if (ret)
		printk("Unable to deregister...\n");
}

module_init(device_start); /*Registers hello_start as module's entry point. The kernel invokes hello_start when LKM is loaded*/
module_exit(device_end); /*Registers hello_end as module's exit point. The kernel invokes hello_end when LKM is unloaded*/
