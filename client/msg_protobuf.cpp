#include "msg_protobuf.h"
#include "msg.h"

int create_msg(unsigned short cmd, unsigned char **msg, size_t *sz)
{
    *sz = MSG_HEAD_SIZE;
    *msg = (unsigned char *)malloc(*sz);
    if (NULL == *msg) {
        return -1;
    }

    unsigned short *cur = (unsigned short *)*msg;
    *((unsigned int *)cur) = htons((unsigned int)0);
    cur += 2;
    *cur++ = htons((unsigned short)cmd);
    *cur++ = htons((unsigned short)0);
    return 0;
}

int message_head(unsigned char *src, size_t src_sz, msg_head *h)
{
    if (MSG_HEAD_SIZE > src_sz) {
        fprintf(stderr, "msg less than head size!\n");
        return -1;
    }

    unsigned short *cur = (unsigned short *)src;
    h->len = ntohs(*((unsigned int *)cur));
    cur += 2;
    h->cmd = ntohs(*cur++);
    h->flags = ntohs(*cur);
    return 0;
}

int conn_write(conn *c, unsigned short cmd)
{
    unsigned char *msg;
    size_t sz;
    int ret = create_msg(cmd, &msg, &sz);
    if (0 != ret)
        return ret;
    ret = conn_write(c, msg, sz);
    free(msg);
    return ret;
}