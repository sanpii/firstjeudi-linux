#include <linux/time.h>
#include <uapi/linux/rtc.h>
#include <asm-generic/rtc.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#include "perceptron.h"

MODULE_LICENSE("Dual BSD/GPL");

#define TEMPLATE "{ \"is_first_jeudi\": %d, \"is_first_week\": %d, \"next_first_jeudi\": \"%s\" }\n"

static Perceptron* perceptron = NULL;

static char* get_now(void)
{
    char* str = NULL;
    struct rtc_time now;

    get_rtc_time(&now);
    str = kmalloc(sizeof(*str) * 11, GFP_KERNEL);
    snprintf(str, 11, "%d-%02d-%02d", now.tm_year + 1900, now.tm_mon + 1, now.tm_mday);
    return str;
}

static ssize_t firstj_read(
    struct file* file, char __user* data, size_t len, loff_t* ppose
) {
    ssize_t size;
    char* s = NULL;
    ssize_t retval = -EFAULT;

    if (*ppose > 0) {
        return 0;
    }

    size = min(len, strlen(TEMPLATE) + 7);
    s = kmalloc(size * sizeof(*s), GFP_KERNEL);
    if (s == NULL) {
        return -ENOMEM;
    }

    {
        int is_first_jeudi = 0;
        char* now = get_now();

        is_first_jeudi = perceptron_test(perceptron, now);
        if (is_first_jeudi >= 0) {
            snprintf(s, size, TEMPLATE, is_first_jeudi, 0, "");
            if (copy_to_user(data, s, size) == 0) {
                *ppose += retval = strlen(s);
            }
        }
        else {
            retval = is_first_jeudi;
        }
        kfree(now), now = NULL;
    }

    kfree(s), s = NULL;
    return retval;
}

static ssize_t firstj_write(
        struct file *file, const char __user *data, size_t len, loff_t *ppose
) {
    char *str = NULL;
    ssize_t retval = -EFAULT;

    str = kmalloc((len + 2) * sizeof(*str), 0);
    if (str != NULL) {
        if (copy_from_user(str, data, len) == 0) {
            char* output = NULL;

            if (str[len - 1] == '\n') {
                str[len - 1] = '\0';
            }

            output = strchr(str, ' ');
            if (output != NULL) {
                output[0] = '\0';
                output++;
                retval = perceptron_learn(perceptron, str, strcmp(output, "0") == 0);
                if (retval == 0) {
                    retval = len;
                }
            }
        }

        kfree(str), str = NULL;
    }
    else {
        retval = -ENOMEM;
    }

    return retval;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = firstj_write,
    .read = firstj_read,
};

static struct miscdevice firstj_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "firstj",
    .fops = &fops,
    .mode = 0666,
};

static int firstj_init(void)
{
    perceptron = perceptron_new(0, 100);
    return misc_register(&firstj_dev);
}

static void firstj_cleanup(void)
{
    misc_deregister(&firstj_dev);
    perceptron_delete(perceptron), perceptron = NULL;
}

module_init(firstj_init);
module_exit(firstj_cleanup);
