#include <linux/module.h>	// required by all modules
#include <linux/kernel.h>	// required for sysinfo
#include <linux/init.h>		// used by module_init, module_exit macros
#include <linux/list.h>		// used by linked list
#include <linux/slab.h>		// used by kmalloc()
#include <linux/kthread.h>	// used by kernel threads
#include <asm/atomic.h>		// used by atomic instruction

MODULE_DESCRIPTION("threads, lists, synchronization tests");
MODULE_AUTHOR("rtchoke");
MODULE_VERSION("0.1");
MODULE_LICENSE("Dual MIT/GPL");		// this affects the kernel behavior

static inline void lock(atomic_t *lock);
static inline void unlock(atomic_t *lock);


static int thr_amnt = 3;
static int inc_cnt = 1000000;
static int *glob_cnt;
atomic_t my_lock = ATOMIC_INIT(0);
module_param(thr_amnt, int, 0);
module_param(inc_cnt, int, 0);
// MODULE_PARAM_DESC(thr_amnt, "Amount of threads");
// MODULE_PARAM_DESC(inc_cnt, "How many times counter will increment in every thread");
LIST_HEAD(head_list);

struct k_list {
        struct list_head list;
        int thr_num;
        int data;
};
struct k_list *klist;
//struct list_head head_list;

static int thread_func(void *args)
{
        lock(&my_lock);
        printk(KERN_INFO "args_cnt before loop : %d", *(int*)args);
        for(int i = 0; i < inc_cnt; i++)
                (*(int*)args)++;
        printk(KERN_INFO "args_cnt after loop: %d", *(int*)args);
        klist = kmalloc(sizeof(struct k_list), GFP_KERNEL);       
        if(NULL == klist) {
                printk(KERN_ERR "Can't allocate memory for list");
                goto errlist;
        }

        klist->data = *(int*)args;
        klist->thr_num = (*(int*)args) / inc_cnt;
        list_add(&klist->list, &head_list);
        unlock(&my_lock);
        return 0;

        errlist:
                kfree(klist);
                return -ENOMEM;
}
static void print_list(void)
{
        struct k_list *_list = NULL;
        struct list_head *entry_ptr = NULL;
        list_for_each(entry_ptr, &head_list) {
                _list = list_entry(entry_ptr, struct k_list, list);
                printk(KERN_INFO" thread %d has data: %d\n", _list->thr_num, _list->data);
        }
}
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
        while(atomic_cmpxchg(lock, 0, 1) == 1); // if lock == 0 ? lock = 1 : lock = 0
}

static inline void unlock(atomic_t *lock)
{
        atomic_set(lock, 0);
}
static int __init threads_test_init(void)
{
        glob_cnt = kmalloc(sizeof(int), GFP_KERNEL);
        if(NULL == glob_cnt) {
                printk(KERN_ERR "Can't allocate memory for counter");
                goto errcnt;
        }
        *glob_cnt = 0;
        struct task_struct **kthread_t = kmalloc(thr_amnt * sizeof(**kthread_t), GFP_KERNEL);
        if(NULL == kthread_t) {
                printk(KERN_ERR "Can't allocate memory for threads");
                goto errmem;
        }
        for (int i = 0; i < thr_amnt; i++) 
		kthread_t[i] = kthread_run(thread_func, (void*)glob_cnt, "thread%d", i);	

	kfree(kthread_t);	

	return 0;

        errcnt:
                kfree(glob_cnt);
                return 0;
        errmem:
                kfree(glob_cnt);
                kfree(kthread_t);
                return 0;


}

static void __exit threads_test_exit(void)
{
        printk(KERN_ALERT "Global counter = %d \n", *glob_cnt);
        print_list();
        delete_list();
        //kfree(klist);
        kfree(glob_cnt);
        printk(KERN_ALERT "EXIT!\n");
}

module_init(threads_test_init);
module_exit(threads_test_exit);
