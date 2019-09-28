/*
* Based on MaksHolub's threadmod.c https://bit.ly/2llv9Tc
*/


#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/list.h>		// used by linked list
#include <linux/slab.h>		// used by kmalloc()
#include <linux/kthread.h>	// used by kernel threads
#include <asm/atomic.h>		// used by atomic instruction
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/workqueue.h>


// LOCK - is defined through Makefile
#ifdef LOCK
        #define SPINLOCK 1
#else
        #define SPINLOCK 0
#endif


MODULE_DESCRIPTION("threads, lists, synchronization tests");
MODULE_AUTHOR("rtchoke");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior


static void work_handler(struct work_struct *);
static void timer_handler(unsigned long _unused);
static void delete_list(void);
static void print_list(int mode);
static int thread_func(void *args);
/*
* lock - function which defends shared region(counter) from intruding of alien threads
* @lock - address of lock variable 
*/
static inline void lock(atomic_t *lock);
/*
* unlock - function unlocks shared region, so alien threads could intrude the foxhole..
* @lock - address of lock variable
*/
static inline void unlock(atomic_t *lock);

/*
* thr_amnt - variable which declares amount of running threads
*/
static int thr_amnt = 3;
/*
* inc_cnt - declares how many times should thread increment glob_cnt.
*/
static int inc_cnt = 1000000;
/*
* glob_cnt - variable which is incremented by threads (actually it's the foxhole)
*/
static int *glob_cnt;
atomic_t my_lock = ATOMIC_INIT(0);
module_param(thr_amnt, int, 0);
module_param(inc_cnt, int, 0);
static int thr1_lock, thr2_lock;

/**
 * LIST_HEAD provide initialization of kernel linked list
 */
LIST_HEAD(head_list);

/**
 * struct k_list - struct of element kernel linked list.
 * @test_list: 	list_head type, which neccessary to provide kernel 
 *		linked list mechanism 				
 * @data: 	Current count value of node.
 * @thr_num: used to identify of node
 *          
 * This struct neccessary to work with kernel linked list
 * It contains a special variable test_list, 
 * which is an instance of a struct kernel linked list.
 */
struct k_list {
        struct list_head list;
        int thr_num;
        int data;
};

struct my_thread_struct {
        int mode;
        int *thr_lock;      
};

struct k_list *klist_wrk, *klist_tmr;
struct work_struct my_work;
struct timer_list my_timer;
/*
* thread_func - it's a thread function which is used by every called thread
* (Battlefield for the foxhole. The one who has a magic key from lock may be called a winner)
* @args - it's void pointer to glob_cnt
*/
static int thread_func(void *args)
{       
        struct my_thread_struct *arguments = (struct my_thread_struct *)args;
        arguments->thr_lock = (int *)args;
        if(1 == arguments->mode) { // work
                INIT_WORK(&my_work, work_handler);
                schedule_work(&my_work);
        }

        if(0 == arguments->mode) { // timer
                init_timer(&my_timer);
                my_timer.data = (unsigned long)NULL;
                my_timer.function = &timer_handler;
                
                my_timer.expires += jiffies + 17;
                add_timer(&my_timer);
        }
        while(!(*arguments->thr_lock));
        lock(&my_lock);
        print_list(arguments->mode);
        unlock(&my_lock);
        return 0;
        // if(SPINLOCK) {
        //         printk(KERN_INFO "Running module with lock");
        //         lock(&my_lock);
        // } else 
        //         printk(KERN_INFO "Running module without lock");

        // printk(KERN_INFO "args_cnt before loop : %d", *(int*)args);
        // for(int i = 0; i < inc_cnt; i++)
        //         (*(int*)args)++;
        // printk(KERN_INFO "args_cnt after loop: %d", *(int*)args);
        // klist = kmalloc(sizeof(struct k_list), GFP_KERNEL);       
        // if(NULL == klist) {
        //         printk(KERN_ERR "Can't allocate memory for list");
        //         goto errlist;
        // }

        // klist->data = *(int*)args;
        // klist->thr_num = (*(int*)args) / inc_cnt;
        // list_add(&klist->list, &head_list);

        // if(SPINLOCK)
        //         unlock(&my_lock);
        // return 0;

        // errlist:
        //         kfree(klist);
        //         return -ENOMEM;
}


static void work_handler(struct work_struct *_unused)
{
        while(1) {
                if(jiffies % 11 == 0) {
                        thr2_lock = 1;
                        break;
                } else {
                        klist_wrk->data = jiffies;
                        klist_wrk->thr_num = 2;
                        lock(&my_lock);
                        list_add(&klist_wrk->list, &head_list);
                        unlock(&my_lock);
                        while(time_before(jiffies, 17))
                                schedule();
                }
        }

}

static void timer_handler(unsigned long _unused)
{
        while(1) {
                if(jiffies % 11 == 0) {
                        thr1_lock = 1;
                        del_timer_sync(&my_timer);
                        break;
                } else {
                        my_timer.expires += jiffies + 17;
                        add_timer(&my_timer);
                        klist_tmr->data = jiffies;
                        klist_tmr->thr_num = 1;
                        lock(&my_lock);
                        list_add(&klist_tmr->list, &head_list);
                        unlock(&my_lock);
                }
        }

}
/*
* print_list - printing information from list about data which every thread counted
*/
static void print_list(int mode)
{
        struct k_list *_list = NULL;
        struct list_head *entry_ptr = NULL;
        list_for_each(entry_ptr, &head_list) {
                _list = list_entry(entry_ptr, struct k_list, list);
                if(_list->thr_num - 1 == mode) { 
                        if(0 == mode)
                                printk(KERN_INFO "Timer_list has such data:\n");
                        if(1 == mode)
                                printk(KERN_INFO "Work_list has such data:\n");

                        printk(KERN_INFO" jiffies: %d\n", _list->data);

                }
               
        }
}
/*
* delete_list - deletes information from list(also delete list)
*/
static void delete_list(void)
{
        struct k_list *_list = NULL;
        struct list_head *entry_ptr = NULL, *tmp = NULL;
        list_for_each_safe(entry_ptr, tmp, &head_list) {
                _list = list_entry(entry_ptr, struct k_list, list);  
                printk(KERN_NOTICE "Deleting #%d!\n", _list->thr_num);
                list_del(entry_ptr);
                kfree(_list);
        }
        printk(KERN_INFO "Successfully deleted all nodes!\n");
}

static inline void lock(atomic_t *lock)
{
        while(atomic_cmpxchg(lock, 0, 1) == 1);
}

static inline void unlock(atomic_t *lock)
{
        atomic_set(lock, 0);
}
/*
* threads_test_init - init function where battle begins, threads are preparing for a fight (memory allocation)
*                     but after not a long fight, they're thrown away to trashcan (by deallocating memory)
*/
static int __init threads_test_init(void)
{
        //TODO: add mem error handling
        klist_wrk = kmalloc(sizeof(struct k_list), GFP_KERNEL);       
        if(NULL == klist_wrk) {
                printk(KERN_ERR "Can't allocate memory for list");
                //goto errlist;
        }
        klist_tmr = kmalloc(sizeof(struct k_list), GFP_KERNEL);       
        if(NULL == klist_tmr) {
                printk(KERN_ERR "Can't allocate memory for list");
                //goto errlist;
        }
        struct task_struct *kthread_tmr = kmalloc(sizeof(*kthread_tmr), GFP_KERNEL);
        struct task_struct *kthread_wrk = kmalloc(sizeof(*kthread_wrk), GFP_KERNEL);
        if(NULL == kthread_wrk || NULL == kthread_tmr) {
                printk(KERN_ERR "Can't allocate memory for threads");
                goto errmem;
        }
        struct my_thread_struct timer_struct = { .mode = 0,
                                              .thr_lock = &thr1_lock
                                            };

        struct my_thread_struct work_struct =  { .mode = 1,
                                              .thr_lock = &thr2_lock
                                            };
                                           
	kthread_tmr = kthread_run(thread_func, (void*)&timer_struct, "thread%d", 0);	
        kthread_wrk = kthread_run(thread_func, (void*)&work_struct, "thread%d", 1);

	kfree(kthread_tmr);
        kfree(kthread_wrk);
        flush_scheduled_work();
	return 0;

        errmem:
                kfree(glob_cnt);
                kfree(kthread_tmr);
                kfree(kthread_wrk);
                return 0;


}
/*
* threads_test_exit - where the battle ends, function that beats up the results, and clears the battlefield(by deallocating memory)
*/
static void __exit threads_test_exit(void)
{
        
        delete_list();
        printk(KERN_ALERT "EXIT!\n");
}

module_init(threads_test_init);
module_exit(threads_test_exit);
