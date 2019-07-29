/*
 * user mode Linux security module kernel portion example
 * Copyright (C) 2019  Argus Cyber Security Ltd, Tel Aviv.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */


#include <linux/module.h>
#include <linux/version.h>
#include <linux/security.h>
#include <linux/lsm_hooks.h>
#include <linux/namei.h>
#include <linux/file.h>
#include <linux/cred.h>
#include <linux/types.h>
#include <linux/binfmts.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

#include "include/netlink-protocol.h"
#include "include/hashtable.h"
#include "include/definitions.h"
#include "process.capnp.h"
#include "capnp_c.h"

/* User mode connected process id */
int user_pid;
/* Hash set of all the pids. Maybe we can use the proc->security */
struct hashtable hashset;
/* netlink mesage counter */
static int counter;

DECLARE_WAIT_QUEUE_HEAD(wait_queue_123);

static capn_text chars_to_text(const char *chars)
{
	return (capn_text) {
		.len = (int) strlen(chars),
		.str = chars,
		.seg = NULL,
	};
}


int ulsm_bprm_check_security(struct linux_binprm *bprm)
{
	// TODO: atomic_fetch_inc
	int current_counter = counter;
	int wait_ret = 0;
	int ret = SUCCESS;

	/* Capn proto */
	struct capn c;
	capn_ptr cr;
	struct capn_segment *cs = NULL;
	struct Process p;
	uint8_t buf[1024];
	int setp_ret = 0;
	ssize_t sz = 0;
	Process_ptr pp;


	if (user_pid == 0) {
		print("Can't hook before pid initalization");
		return SUCCESS;
	}

	capn_init_malloc(&c);
	cr = capn_root(&c);
	cs = cr.seg;

	counter++;
	print("bprm_check_security: pid: %d, tgid: %d, path:%s\n",
		current->pid, current->tgid, bprm->filename);

	/* Extract data */
	p.id = current_counter;
	p.name = chars_to_text(bprm->filename);
	pp = new_Process(cs);
	write_Process(&p, pp);
	setp_ret = capn_setp(capn_root(&c), 0, pp.p);
	sz = capn_write_mem(&c, buf, sizeof(buf), 0 /* packed */);
	capn_free(&c);

	/* Send to userspace in netfilter */
	send_message(buf, sz);

	/* Wait on atomic var */
	print("bprm_check_security: going to sleep");

	wait_ret = wait_event_timeout(wait_queue_123,
			hashtable_get(&hashset, current_counter) != -1, 3*HZ);

	print("bprm_check_security: woke");
	if (wait_ret >= 1) {
		/* Wait timedout and condition not met */
		ret = -hashtable_get(&hashset, current_counter);
	}
	hashtable_delete(&hashset, current_counter);
	return ret;
}

static void ulsm_task_free(struct task_struct *task)
{
	if (user_pid == 0) {
		print("Can't hook task free before pid initalization");
	} else if (user_pid == task->pid) {
		user_pid = 0;
		print("PID zeroed!");
	}
}

static struct security_hook_list ulsm_ops[] = {
	LSM_HOOK_INIT(bprm_check_security, ulsm_bprm_check_security),
	LSM_HOOK_INIT(task_free, ulsm_task_free),
};




static int __init ulsm_security_init(void)
{
	user_pid = 0;
	hashtable_init(&hashset);
	security_add_hooks(ulsm_ops, ARRAY_SIZE(ulsm_ops), MODULE_NAME);

	print(MODULE_NAME " security_initcall finished successfully");
	return SUCCESS;
}


static int __init ulsm_late_initcall(void)
{
	if (init_netlink())
		print(MODULE_NAME " late_initcall failed");

	print(MODULE_NAME " late_initcall finished successfully");
	return SUCCESS;
}

security_initcall(ulsm_security_init);
late_initcall(ulsm_late_initcall);
