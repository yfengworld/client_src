#include "net.h"

#include <stdio.h>
#include <stdlib.h>

struct net_ctx {
	fd_set fds;
	conn *cs;
	fd_set readfds;
	fd_set writefds;
	fd_set exceptfds;
};

static struct net_ctx ctx;
static void net_ctx_init(struct net_ctx *ctx)
{
	FD_ZERO(&ctx->fds);
	ctx->cs = NULL;
}

static conn *get_conn(struct net_ctx *ctx, SOCKET fd)
{
	conn *c = ctx->cs;
	while (c) {
		if (c->fd == fd)
			return c;
	}
	return NULL;
}

static int del_conn(struct net_ctx *ctx, SOCKET fd)
{
	conn *c = ctx->cs;
	while (c) {
		if (c->fd == fd) {
			if (c->prev)
				c->prev->next = c->next;
			if (c->next)
				c->next->prev = c->prev;
			return 0;
		}
	}
	return -1;
}

int net_init()
{
	WSADATA wsa;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsa)) {
		fprintf(stderr, "WSAStartup Failed!");
		return -1;
	}
	return 0;
}

void net_release()
{
	WSACleanup();
}

static struct sk_buff *sk_buff_new(int len)
{
	struct sk_buff *buff = (struct sk_buff *)malloc(sizeof(struct sk_buff));
	if (NULL == buff) {
		fprintf(stderr, "sk_buff alloc failed!");
		return NULL;
	}
	buff->buffer = malloc(len);
	if (NULL == buff->buffer) {
		free(buff);
		fprintf(stderr, "sk_buff->buffer alloc failed!");
		return NULL;
	}
	buff->len = len;
	buff->read_pos = 0;
	buff->write_pos = 0;
	return buff;
}

static void sk_buff_free(struct sk_buff *buff)
{
	if (buff->buffer)
		free(buff->buffer);
	free(buff);
}

int sk_buff_used(struct sk_buff *buff)
{
	return buff->write_pos - buff->read_pos;
}

static int sk_buff_remain(struct sk_buff *buff)
{
	return buff->len - (buff->write_pos - buff->read_pos);
}

static int sk_buff_tail_remain(struct sk_buff *buff)
{
	return buff->len - buff->write_pos;
}

static int sk_buff_expand(struct sk_buff *buff, int need_len)
{
	int used;
	char *newbuffer;
	int len = buff->len;
	while (len < need_len)
		len <<= 1;

	newbuffer = malloc(len);
	if (NULL == newbuffer) {
		fprintf(stderr, "newbuff alloc failed!");
		return -1;
	}
	if (0 < (used = sk_buff_used(buff))) {
		memcpy(newbuffer, buff->buffer + buff->read_pos, used);
	}
	buff->read_pos = 0;
	buff->write_pos = used;
	buff->len = len;
	free(buff->buffer);
	buff->buffer = newbuffer;
	return 0;
}

static int sk_buff_push(struct sk_buff *buff, char *data, int len)
{
	int remain = sk_buff_remain(buff);
	int used;
	if (len > remain) {
		if (0 != sk_buff_expand(buff, remain + len))
			return -1;
	}
	if (len > sk_buff_tail_remain(buff)) {
		used = sk_buff_used(buff);
		memmove(buff->buffer, buff->buffer + buff->read_pos, buff->read_pos);
		buff->read_pos = 0;
		buff->write_pos = used;
	}
	memcpy(buff->buffer + buff->write_pos, data, len);
	buff->write_pos += len;
	return 0;
}

int sk_buff_pop(struct sk_buff *buff, int len)
{
	if (len <= sk_buff_used(buff)) {
		buff->read_pos += len;
		return 0;
	}
	return -1;
}

conn *conn_new(user_callback *cb)
{
	conn *c = (conn *)malloc(sizeof(conn));
	if (NULL == c) {
		fprintf(stderr, "conn alloc failed!");
		return NULL;
	}
	c->cb = *cb;
	c->state = CONN_STATE_NONE;
	c->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == c->fd) {
		free(c);
		fprintf(stderr, "create socket failed!");
		return NULL;
	}
	c->rdbuff = sk_buff_new(100 * 1024);
	if (NULL == c->rdbuff) {
		closesocket(c->fd);
		free(c);
		return NULL;
	}
	c->wrbuff = sk_buff_new(100 * 1024);
	if (NULL == c->wrbuff) {
		closesocket(c->fd);
		sk_buff_free(c->rdbuff);
		c->rdbuff = NULL;
		free(c);
		return NULL;
	}
	c->prev = NULL;
	c->next =NULL;
	return c;
}

void conn_free(conn *c)
{
	if (c->rdbuff)
	{
		sk_buff_free(c->rdbuff);
		c->rdbuff = NULL;
	}
	if (c->wrbuff)
	{
		sk_buff_free(c->wrbuff);
		c->wrbuff = NULL;
	}
	free(c);
}

void connect_cb(conn *);
int conn_connect(conn *c, struct sockaddr *sa, size_t len)
{
	int err;
	unsigned long ul = 1;
	int rc = connect(c->fd, sa, len);
	if (0 == rc) {
		c->state = CONN_STATE_ESTABLISH;
		/*set nonblocking */
		ioctlsocket(c->fd, FIONBIO, (unsigned long *)&ul);
		FD_SET(c->fd, &ctx.fds);
		c->prev = NULL;
		if (ctx.cs)
			ctx.cs->prev = c;
		c->next = ctx.cs;
		ctx.cs = c;
		connect_cb(c);
		return 0;
	}
	err = WSAGetLastError();
	fprintf(stderr, "connect err:%d", err);
	return -1;
}

int conn_write(conn *c, unsigned char *msg, size_t sz)
{
	if (CONN_STATE_ESTABLISH == c->state) {
		return sk_buff_push(c->wrbuff, msg, sz);
	}
	return -1;
}

void disconnect_cb(conn *);
void disconnect(conn *c) {
	if (CONN_STATE_ESTABLISH == c->state) {
		c->state = CONN_STATE_NONE;
		FD_CLR(c->fd, &ctx.fds);
		del_conn(&ctx, c->fd);
		closesocket(c->fd);
		disconnect_cb(c);
	}
}

static void except(conn *c)
{
}

void read_cb(conn *);
static void readable(conn *c)
{
	int rc;
	char buffer[512];
	
	if (CONN_STATE_NONE == c->state)
		return;

	rc = recv(c->fd, buffer, 512, 0);
	if (0 > rc) {
		if (10035 != WSAGetLastError())
			goto err;
	} else if (0 == rc) {
		goto err;
	} else {
		if (0 != sk_buff_push(c->rdbuff, buffer, rc))
			goto err;
		else
			read_cb(c);
	}
	return;
err:
	disconnect(c);
	return;
}


#define MAX_WRITE_ONCE 8192
static void writable(conn *c)
{
	int rc;
	int to_write;
	if (CONN_STATE_ESTABLISH == c->state && 0 < (to_write = sk_buff_used(c->wrbuff))) {
		if (MAX_WRITE_ONCE < to_write)
			to_write = MAX_WRITE_ONCE;
		rc = send(c->fd, c->wrbuff->buffer + c->wrbuff->read_pos, to_write, 0);
		if (0 > rc) {
			if (10035 != WSAGetLastError())
				goto err;
		} else {
			if (0 != sk_buff_pop(c->wrbuff, rc))
				goto err;
		}
	}
	return;
err:
	disconnect(c);
	return;
}

#define MAX_SELECT 16

void net_loop()
{
	int rc;
	unsigned int i;
	conn *c;
	memcpy(&ctx.readfds, &ctx.fds, sizeof(ctx.fds));
	memcpy(&ctx.writefds, &ctx.fds, sizeof(ctx.fds));
	memcpy(&ctx.exceptfds, &ctx.fds, sizeof(ctx.fds));
	rc = select(MAX_SELECT, &ctx.readfds, &ctx.writefds, &ctx.exceptfds, NULL);
	if (0 < rc) {
		for (i = 0; i < ctx.fds.fd_count; i++) {
			c = get_conn(&ctx, ctx.fds.fd_array[i]);
			if (FD_ISSET(ctx.fds.fd_array[i], &ctx.exceptfds)) {
				except(c);
			}
			if (FD_ISSET(ctx.fds.fd_array[i], &ctx.writefds)) {
				writable(c);
			}
			if (FD_ISSET(ctx.fds.fd_array[i], &ctx.readfds)) {
				readable(c);
			}
		}
	}
}