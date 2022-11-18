#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/ktime.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

static int lab1_show(struct seq_file *m, void *v)
{
  const struct cred * cred = current_cred();
  //printk("Current Process PCB Information\n");
  seq_printf(m, "Current Process PCB Information\n");
  seq_printf(m, "Name =  %s\n", current->comm);
  seq_printf(m, "PID =  %d\n", current->pid);
  seq_printf(m, "PPID = %d\n", task_ppid_nr(current));
  switch (current->state)
  {
  case 0 :
    seq_printf(m, "State = Running\n");
    break;
  case 4:
    seq_printf(m, "State = Stopped\n");
    break;
  default:
    seq_printf(m, "State = Waiting\n");
  }
  seq_printf(m, "Real UID =  %d\n", cred->uid.val);
  seq_printf(m, "Effective UID =  %d\n", cred->euid.val);
  seq_printf(m, "Saved UID =  %d\n", cred->suid.val);
  seq_printf(m, "Real GID =  %d\n", cred->gid.val);
  seq_printf(m, "Effecitve GID =  %d\n", cred->egid.val);
  seq_printf(m, "Saved GID =  %d\n", cred->sgid.val);
  return 0;
}

static int lab1_open(struct inode *inode, struct file *file)
{
  return single_open(file, lab1_show, NULL);
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops lab1_fops = {
    .proc_open = lab1_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release
};
#else
static const struct file_operations lab1_fops = {
    .owner = THIS_MODULE,
    .open = lab1_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release
};
#endif

static int __init lab1_init(void)
{
  if (proc_create("lab1", 0, NULL, &lab1_fops) == NULL)
  {
    return -ENOMEM;
  }
  printk(KERN_INFO "lab1mod in\n");
  return 0;
}

static void __exit lab1_exit(void)
{
  remove_proc_entry("lab1", NULL);
  printk(KERN_INFO "lab1mod out\n");
}

MODULE_LICENSE("GPL");
module_init(lab1_init);
module_exit(lab1_exit);
