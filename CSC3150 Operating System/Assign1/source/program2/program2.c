#include <linux/module.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/printk.h>
#include <linux/jiffies.h>
#include <linux/kmod.h>
#include <linux/fs.h>
#include <linux/signal.h> //Newly added
#include <linux/sched/signal.h>//Newly added

MODULE_LICENSE("GPL");

static struct task_struct *task;

int status;

struct wait_opts wo;

/*the code below are extern clarification copied from kernel source code*/
struct wait_opts {
	enum pid_type		wo_type;
	int			wo_flags;
	struct pid		*wo_pid;

	struct waitid_info	*wo_info;
	int			wo_stat;
	struct rusage		*wo_rusage;

	wait_queue_entry_t		child_wait;
	int			notask_error;
};



extern long _do_fork(unsigned long clone_flags,
	      unsigned long stack_start,
	      unsigned long stack_size,
	      int __user *parent_tidptr,
	      int __user *child_tidptr,
	      unsigned long tls);

extern int do_execve(struct filename *filename,
	const char __user *const __user *__argv,
	const char __user *const __user *__envp);

extern long do_wait(struct wait_opts *wo);

extern struct filename * getname(const char __user * filename);


/* excute a test program in child process */
/* A forked function withnin which to execute a child process*/
int fork_func(void) {
	const char __user *filename = "/home/estelle/Downloads/CSC3150_Assignment1_1/source/program2/test";
	
	//Use getname function externed from the source code
	struct filename *test = getname(filename);

	// Execute the test program
	status = do_execve(test,NULL,NULL);

	printk("[program2] : The child process is core-dumped\n");
	printk("[program2] : The return signal number = %d\n",status);
	printk("[program2] : child process\n");

	return 0;
}


//implement fork function
int my_fork(void *argc){	
	//set default sigaction for current process
	int i;
	struct k_sigaction *k_action = &current->sighand->action[0];
	for(i=0;i<_NSIG;i++){
		k_action->sa.sa_handler = SIG_DFL;
		k_action->sa.sa_flags = 0;
		k_action->sa.sa_restorer = NULL;
		sigemptyset(&k_action->sa.sa_mask);
		k_action++;
	}
	
	printk("[program2] : module_init kthread start\n");
	long pid=0;
	long sig=0;
	int __user *status;

	/* fork a process using do_fork */
	//The stack address point to the forked function
	pid=_do_fork(SIGCHLD,(unsigned long)&fork_func,0,NULL,NULL,0);

	printk("[program2] : The child process has pid = %ld\n", pid);
	printk("[program2] : This is the parent process, pid = %d\n",(int)current->pid);
	
	/* wait until child process terminates */
    wo.wo_type = PIDTYPE_PID;
	wo.wo_flags = WEXITED;
	wo.wo_pid = find_get_pid(pid);
	wo.wo_info = NULL;
	wo.wo_rusage = NULL;
	wo.wo_stat = (int)&status;
	sig = do_wait(&wo);
	printk("get signal, signal number = %d\n", wo.wo_stat);
	
	return 0;
}

static int __init program2_init(void){

	printk("[program2] : module_init\n");
	
	/* write your code here */
	/* create a kernel thread to run my_fork */

	task=kthread_create(&my_fork,NULL,"MyThread");
	//Wake up the new thread if ok
	if (!IS_ERR(task)) {
		printk("[program2] : module_init create kthread start\n");
		wake_up_process(task);
	}
	else {
		// err=PTR_ERR(task);
		// task=NULL;
		// return err;
		printk("[program2]:error\n");
	}
	
	return 0;
}

static void __exit program2_exit(void){
	printk("[program2] : module_exit\n");
}

module_init(program2_init);
module_exit(program2_exit);
