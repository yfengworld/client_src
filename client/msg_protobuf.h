#ifndef MSG_PROTOBUF_H_INCLUDED
#define MSG_PROTOBUF_H_INCLUDED

#include "msg.h"
#include "net.h"

#include "google/protobuf/stubs/common.h"
#include "google/protobuf/wire_format_lite_inl.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

int create_msg(unsigned short cmd, unsigned char **msg, size_t *sz);
int message_head(unsigned char *src, size_t src_sz, msg_head *h);

template<typename S>
int create_msg(unsigned short cmd, S *s, unsigned char **msg, size_t *sz)
{
    size_t body_sz = s->ByteSize();
    *sz = MSG_HEAD_SIZE + body_sz;
    *msg = (unsigned char *)malloc(*sz);
    if (NULL == *msg) {
        merror("msg alloc failed!");
        return -1;
    }

    unsigned short *cur = (unsigned short *)*msg;
    *((unsigned int *)cur) = htons((unsigned short)body_sz);
    cur += 2;
    *cur++ = htons((unsigned short)cmd);
    *cur++ = htons((unsigned short)0);
    
    google::protobuf::io::ArrayOutputStream arr(cur, body_sz);
    google::protobuf::io::CodedOutputStream output(&arr);
    s->SerializeToCodedStream(&output);
    return 0;
}

template<typename S>
int msg_body(unsigned char *src, size_t src_sz, S *s)
{
    if (MSG_HEAD_SIZE > src_sz) {
        merror("msg less than head size!");
        return -1;
    }
    unsigned short *cur = (unsigned short *)src;
    unsigned int len = ntohs(*((unsigned int *)cur));
    cur += 2;
    if (MSG_HEAD_SIZE + len != src_sz) {
        merror("msg length error!");
        return -1;
    }

    cur += 2;
    google::protobuf::io::CodedInputStream input((const google::protobuf::uint8*)cur, len);
    s->ParseFromCodedStream(&input);
    return 0;
}

template<typename S>
int conn_write(conn *c, unsigned short cmd, S *s)
{
    unsigned char *msg;
    size_t sz;
    int ret = create_msg(cmd, s, &msg, &sz);
    if (ret != 0)
        return ret;
    ret = conn_write(c, msg, sz);
    free(msg);
    return ret;
}

int conn_write(conn *c, unsigned short cmd);

#endif /* MSG_PROTOBUF_H_INCLUDED */