#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/interrupt.h>
#include <linux/list.h>
#include <asm/atomic.h>
#include <linux/kthread.h>
#include <linux/slab.h>

MODULE_DESCRIPTION("threads, lists, synchronization tests");
MODULE_AUTHOR("rtchoke");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior



static struct task_struct **thread_st;
static int thr_amnt = 0;
static int inc_cnt = 0;
atomic_t my_lock = ATOMIC_INIT(0);
module_param(thr_amnt, int, 0000);
module_param(inc_cnt, int, 0000);

struct k_list {
        struct list_head list;
        int data;
}
LIST_HEAD(head_list);
static int thread_func(void *args)
{


}
static void print_list(void)
{


}
static void delete_list(void)
{


}

static inline void lock(atomic_t *lock)
{
        while(*lock);
        cas(lock, 0, 1); // if lock == 0 ? lock = 1 : lock = 0

}

static inline void unlock(atomic_t *lock)
{
        *lock = 0;
 
}
static int __init threads_test_init(void)
{
        static int *cnt = kmalloc(sizeof(*cnt), GFP_KERNEL);
        if(NULL == cnt) {
                printk(KERN_ERR "Can't allocate memory for counter");
                // goto...
        }

        struct task_struct **kthread_t = kmalloc(thr_amnt * sizeof(**kthread_t));
        if(NULL == kthread_t) {
                printk(KERN_ERR "Can't allocate memory for threads");
                // goto..
        }

}

static void __exit threads_test_exit(void)
{


}