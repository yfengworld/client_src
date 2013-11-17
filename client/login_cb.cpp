#include "net.h"
#include "msg_protobuf.h"

void login_rpc_cb(conn *c, unsigned char *msg, size_t sz)
{

}

void login_connect_cb(conn *c)
{

}

void login_disconnect_cb(conn *c)
{

}

void login_cb_init(user_callback *cb)
{
	cb->rpc = login_rpc_cb;
	cb->connect = login_connect_cb;
	cb->disconnect = login_disconnect_cb;
}