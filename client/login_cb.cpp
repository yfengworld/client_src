#include "login.pb.h"

#include "msg_protobuf.h"
#include "cmd.h"
#include "net.h"

typedef void (*cb)(conn *, unsigned char *, size_t);
static cb cbs[SC_END - SC_BEGIN];

static void login_reply_cb(conn *c, unsigned char *msg, size_t sz)
{
	login::login_reply lr;
	msg_body<login::login_reply>(msg, sz, &lr);
	printf("login_reply_cb err:%d\n", lr.err());

	login::login_request r;
	r.set_account("abc");
	r.set_passwd("123");
	conn_write<login::login_request>(c, cl_login_request, &r);
}

static void connect_reply_cb(conn *c, unsigned char *msg, size_t sz)
{
	login::connect_reply cr;
	msg_body<login::connect_reply>(msg, sz, &cr);
	printf("connect_reply_cb err:%d\n", cr.err());
}

static void login_rpc_cb(conn *c, unsigned char *msg, size_t sz)
{
	msg_head h;
	if (0 != message_head(msg, sz, &h)) {
		fprintf(stderr, "message_head failed!\n");
		return;
	}

	printf("server -> client cmd:%d len:%d flags:%d\n", h.cmd, h.len, h.flags);
	if (h.cmd > SC_BEGIN && h.cmd < SC_END) {
		if (cbs[h.cmd - SC_BEGIN])
			(*(cbs[h.cmd - SC_BEGIN]))(c, msg, sz);
	} else {
		fprintf(stderr, "server -> client invalid cmd:%d len:%d flags:%d\n", h.cmd, h.len, h.flags);
	}
}

static void login_connect_cb(conn *c)
{
	login::login_request lr;
	lr.set_account("abc");
	lr.set_passwd("123");
	conn_write<login::login_request>(c, cl_login_request, &lr);
}

static void login_disconnect_cb(conn *c)
{

}

void login_cb_init(user_callback *cb)
{
	cb->rpc = login_rpc_cb;
	cb->connect = login_connect_cb;
	cb->disconnect = login_disconnect_cb;
	memset(cbs, 0, sizeof(cb) * (SC_END - SC_BEGIN));
	cbs[lc_login_reply - SC_BEGIN] = login_reply_cb;
	cbs[gc_connect_reply - SC_BEGIN] = connect_reply_cb;
}