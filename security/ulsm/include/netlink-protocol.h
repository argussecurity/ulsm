/**
 * @file
 *
 * Defines netlink communication protocol between lsm to user
 */

#ifndef NETLINK_PROTO_H
#define NETLINK_PROTO_H

#include <linux/skbuff.h>

/**
 * Received a message using netlink, and Initializes connection if needed
 *
 * @param [in]	skb - socket buffer, with message data or initialization message.
 */
void hello_nl_recv_msg(struct sk_buff *skb);

/**
 * Sends the message using netlink socket.
 *
 * @param [in]	msg - the message to send
 * @param [in]	msg_size - size of the message in bytes
 *
 * @return Error code (-1) on failure, SUCCESS otherwise.
 */
int send_message(const char * msg, int msg_size);

/**
 * Initialized netlink conenction and sets input function
 *
 * @return Error code (-1) on failure, SUCCESS otherwise.
 */
int __init init_netlink(void);

/**
 * Releases the netlink connection
 */
void	cleanup_netlink(void);

#endif /* NETLINK_PROTO_H */
