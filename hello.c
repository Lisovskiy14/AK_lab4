/*
 * Copyright (c) 2017, GlobalLogic Ukraine LLC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 * must display the following acknowledgement:
 * This product includes software developed by the GlobalLogic.
 * 4. Neither the name of the GlobalLogic nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY GLOBALLOGIC UKRAINE LLC ``AS IS`` AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL GLOBALLOGIC UKRAINE LLC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/moduleparam.h>
#include <linux/ktime.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/errno.h>

MODULE_AUTHOR("Lisovskiy Nazar");
MODULE_DESCRIPTION("Hello, world in Linux Kernel Training");
MODULE_LICENSE("Dual BSD/GPL");


static uint count = 1;
module_param(count, uint, S_IRUGO);
MODULE_PARM_DESC(count, "Кількість разів для друку 'Hello, world!'");

struct hello_event {
	struct list_head list;
	ktime_t timestamp;
};

static LIST_HEAD(event_list_head);

static int __init hello_init(void)
{
	int i;
	struct hello_event *new_event;

	if (count > 10) {
		printk(KERN_ERR "Помилка: Значення count (%u) > 10. Модуль не завантажено.\n", count);
		return -EINVAL;
	}

	if (count == 0 || (count >= 5 && count <= 10)) {
		printk(KERN_WARNING "Попередження: Значення count = %u.\n", count);
	}

	for (i = 0; i < count; i++) {
		new_event = kmalloc(sizeof(*new_event), GFP_KERNEL);
		if (!new_event) {
			printk(KERN_ERR "Не вдалося виділити пам'ять. Очищення...\n");
			goto cleanup_list;
		}

		new_event->timestamp = ktime_get();
		printk(KERN_EMERG "Hello, world! (%d/%u)\n", i + 1, count);
		list_add_tail(&new_event->list, &event_list_head);
	}

	return 0;

cleanup_list:
{
	struct hello_event *entry, *tmp;
	list_for_each_entry_safe(entry, tmp, &event_list_head, list) {
		list_del(&entry->list);
		kfree(entry);
	}
	return -ENOMEM;
}
}


static void __exit hello_exit(void)
{
	struct hello_event *entry, *tmp;
	printk(KERN_EMERG "Goodbye, world! Друк збережених часових міток:\n");

	list_for_each_entry_safe(entry, tmp, &event_list_head, list) {
		printk(KERN_EMERG "  Час події: %lld ns\n", ktime_to_ns(entry->timestamp));
		list_del(&entry->list);
		kfree(entry);
	}
}

module_init(hello_init);
module_exit(hello_exit);