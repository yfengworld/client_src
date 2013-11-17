#ifndef NET_H_INCLUDED
#define NET_H_INCLUDED

#include <WinSock2.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sk_buff {
	int len;
	char *buffer;
	int read_pos;
	int write_pos;
};

typedef struct conn conn;
typedef void (*rpc_cb_func)(conn *, unsigned char *, size_t);
typedef void (*connect_cb_func)(conn *);
typedef void (*disconnect_cb_func)(conn *);

typedef struct {
    char type;
    rpc_cb_func rpc;
    connect_cb_func connect;
    disconnect_cb_func disconnect;
} user_callback;

struct conn {
	user_callback cb;
#define CONN_STATE_NONE 0
#define CONN_STATE_ESTABLISH 1
	int state;
	SOCKET fd;
	struct sk_buff *rdbuff;
	struct sk_buff *wrbuff;
	conn *prev;
	conn *next;
};

int net_init();
void net_release();
void net_loop();
conn *conn_new(user_callback *cb);
void conn_free(conn *c);
void disconnect(conn *c);
int conn_connect(conn *c, struct sockaddr *sa, size_t len);
int conn_write(conn *c, unsigned char *msg, size_t sz);

#ifdef __cplusplus
}
#endif

#endif /* NET_H_INCLUDED */