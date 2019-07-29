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

#include "include/netlink-protocol.h"
#include "include/hashtable.h"
#include "include/definitions.h"

extern int user_pid;
extern struct hashtable hashset;

struct sock *nl_sk; /** netlink socket */

void hello_nl_recv_msg(struct sk_buff *skb)
{
	struct nlmsghdr *nlh = (struct nlmsghdr *)skb->data;
	int id = *(int *) nlmsg_data(nlh);
	int value = *((int *) nlmsg_data(nlh) + 1);
	int sender_pid = nlh->nlmsg_pid;

	print("Received message from user: pid: %d, id: %d, value: %d",
		sender_pid, id, value);

	if (user_pid == 0) {
		user_pid = sender_pid;
		print("ulsmd new PID: %d", user_pid);
	} else {
		int res = hashtable_set(&hashset, id, value);

		if (res == SUCCESS) {
			print("Waking up!");
			wake_up(&wait_queue_123);
		} else {
			print("Error waking %d up", id);
		}
	}
}

int send_message(const char *msg, int msg_size)
{
	struct nlmsghdr *nlh;
	int res;
	struct sk_buff *skb_out;

	if (user_pid == 0) {
		print("Can't send message before pid initalization");
		return -1;
	}

	skb_out = nlmsg_new(msg_size, 0);
	if (!skb_out) {
		print("Failed to allocate new skb");
		return -1;
	}

	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
	NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
	memcpy(nlmsg_data(nlh), msg, msg_size);
	res = nlmsg_unicast(nl_sk, skb_out, user_pid);

	if (res < 0)
		print("Failed to send: %d", res);

	return SUCCESS;
}


int __init init_netlink(void)
{
	struct netlink_kernel_cfg cfg = {
		.input = hello_nl_recv_msg,
	};

	nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
	if (!nl_sk) {
		pr_emerg("Error creating socket.\n");
		return -1;
	}

	return SUCCESS;
}


void cleanup_netlink(void)
{
	netlink_kernel_release(nl_sk);
}
