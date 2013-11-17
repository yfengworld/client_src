#include "net.h"
#include "msg_protobuf.h"
#include <stdio.h>

void login_cb_init(user_callback *);
static user_callback login_cb;

void server_cb_init(user_callback *);
static user_callback server_cb;

int main(int argc, char **argv)
{
	if (0 != net_init()) {
		return 1;
	}
	
	login_cb_init(&login_cb);
	server_cb_init(&server_cb);
	conn *c = conn_new(&login_cb);
	if (NULL == c) {
		return 1;
	}

	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr("192.168.56.1");
	sa.sin_port = htons(41000);
	if (0 != conn_connect(c, (struct sockaddr *)&sa, sizeof(sa))) {
		return 1;
	}

	while (1)
		net_loop();

	net_release();
	return 0;
}