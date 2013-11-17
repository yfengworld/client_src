#include "net.h"
#include "msg.h"

#include <stdio.h>

int sk_buff_used(struct sk_buff *);
int sk_buff_pop(struct sk_buff *, int);
void read_cb(conn *c)
{
	unsigned int total_len;
	char *buffer;
	unsigned short *cur;
	unsigned int len;
	size_t msg_len;
	total_len = sk_buff_used(c->rdbuff);

	while (1) {
		if (total_len < MSG_HEAD_SIZE)
			return;

		buffer = c->rdbuff->buffer + c->rdbuff->read_pos;
		cur = (unsigned short *)buffer;
		len = ntohs(*(unsigned int *)cur);
		cur += 2;

		if (MSG_MAX_SIZE < len)
		{
			fprintf(stderr, "len:%d > MSG_MAX_SIZE!", len);
			goto err;
		}

		if (total_len < MSG_HEAD_SIZE + len)
			return;

		msg_len = MSG_HEAD_SIZE + len;

		/* callback */
		if (c->cb.rpc)
			(*(c->cb.rpc))(c, buffer, msg_len);

		if (0 != sk_buff_pop(c->rdbuff, msg_len)) {
			fprintf(stderr, "sk_buff_pop failed!");
			goto err;
		}
		total_len -= msg_len;
	}
	return;

err:
	disconnect(c);
	return;
}

void connect_cb(conn *c)
{
	if (c->cb.connect)
		(*(c->cb.connect))(c);
}

void disconnect_cb(conn *c)
{
	if (c->cb.disconnect)
		(*(c->cb.disconnect))(c);
}