/*
 * twemproxy - A fast and lightweight proxy for memcached protocol.
 * Copyright (C) 2011 Twitter, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _NC_CONNECTION_H_
#define _NC_CONNECTION_H_

#include <nc_core.h>

typedef rstatus_t (*conn_recv_t)(struct context *, struct conn*);
typedef struct msg* (*conn_recv_next_t)(struct context *, struct conn *, bool);
typedef void (*conn_recv_done_t)(struct context *, struct conn *, struct msg *, struct msg *);

typedef rstatus_t (*conn_send_t)(struct context *, struct conn*);
typedef struct msg* (*conn_send_next_t)(struct context *, struct conn *);
typedef void (*conn_send_done_t)(struct context *, struct conn *, struct msg *);

typedef void (*conn_close_t)(struct context *, struct conn *);
typedef bool (*conn_active_t)(struct conn *);

typedef void (*conn_ref_t)(struct conn *, void *);
typedef void (*conn_unref_t)(struct conn *);

typedef void (*conn_msgq_t)(struct context *, struct conn *, struct msg *);

struct conn {
    TAILQ_ENTRY(conn)  conn_tqe;      /* link in server_pool / server / free q */
    void               *owner;        /* connection owner - server_pool / server */

    int                sd;            /* socket descriptor */
    int                family;        /* socket address family */
    socklen_t          addrlen;       /* socket length */
    struct sockaddr    *addr;         /* socket address (ref in server or server_pool) */

    struct msg_tqh     imsg_q;        /* incoming request Q */
    struct msg_tqh     omsg_q;        /* outstanding request Q */
    struct msg         *rmsg;         /* current message being rcvd */
    struct msg         *smsg;         /* current message being sent */

    conn_recv_t        recv;          /* recv (read) handler */
    conn_recv_next_t   recv_next;     /* recv next message handler */
    conn_recv_done_t   recv_done;     /* read done handler */
    conn_send_t        send;          /* send (write) handler */
    conn_send_next_t   send_next;     /* write next message handler */
    conn_send_done_t   send_done;     /* write done handler */
    conn_close_t       close;         /* close handler */
    conn_active_t      active;        /* active? handler */

    conn_ref_t         ref;           /* connection reference handler */
    conn_unref_t       unref;         /* connection unreference handler */

    conn_msgq_t        enqueue_inq;   /* connection inq msg enqueue handler */
    conn_msgq_t        dequeue_inq;   /* connection inq msg dequeue handler */
    conn_msgq_t        enqueue_outq;  /* connection outq msg enqueue handler */
    conn_msgq_t        dequeue_outq;  /* connection outq msg dequeue handler */

    size_t             recv_bytes;    /* received (read) bytes */
    size_t             send_bytes;    /* sent (written) bytes */

    uint32_t           events;        /* connection io events */
    err_t              err;           /* connection errno */
    unsigned           recv_active:1; /* recv active? */
    unsigned           recv_ready:1;  /* recv ready? */
    unsigned           send_active:1; /* send active? */
    unsigned           send_ready:1;  /* send ready? */

    unsigned           client:1;      /* client? or server? */
    unsigned           proxy:1;       /* proxy? */
    unsigned           connecting:1;  /* connecting? */
    unsigned           connected:1;   /* connected? */
    unsigned           eof:1;         /* eof? aka passive close? */
    unsigned           done:1;        /* done? aka close? */
    unsigned           redis:1;       /* redis? */
    unsigned           need_auth:1;   /* need_auth? */

#if 1 //shenzheng 2015-7-14 config-reload
	unsigned           reload_conf:1;   /* for reload_conf? */
#endif //shenzheng 2015-7-14 config-reload

#if 1 //shenzheng 2015-7-28 replace server
	unsigned		   replace_server:1;/* 1:this msg is for replace_server command, 0:other msgs */
	long long		   conf_version_curr;
	struct context 	   *ctx;
#endif //shenzheng 2015-7-28 replace server
};

TAILQ_HEAD(conn_tqh, conn);

struct context *conn_to_ctx(struct conn *conn);
struct conn *conn_get(void *owner, bool client, bool redis);
struct conn *conn_get_proxy(void *owner);
void conn_put(struct conn *conn);
ssize_t conn_recv(struct conn *conn, void *buf, size_t size);
ssize_t conn_sendv(struct conn *conn, struct array *sendv, size_t nsend);
void conn_init(void);
void conn_deinit(void);
uint32_t conn_ncurr_conn(void);
uint64_t conn_ntotal_conn(void);
uint32_t conn_ncurr_cconn(void);

#if 1 //shenzheng 2015-4-27 proxy administer
uint32_t conn_ncurr_conn_proxy_adm(void);
uint64_t conn_ntotal_conn_proxy_adm(void);
uint32_t conn_ncurr_cconn_proxy_adm(void);
void conn_put_proxy_adm(struct conn *conn);
struct conn * conn_get_proxy_adm(void *owner, int sd, bool client);
#endif //shenzheng 2015-7-9 proxy administer

#if 1 //shenzheng 2015-7-9 config-reload
struct conn * conn_get_proxy_for_reload(void *owner);
struct conn * conn_get_for_reload(void *owner);
void conn_put_for_reload(struct conn *conn);
#endif //shenzheng 2015-7-9 config-reload

#if 1 //shenzheng 2015-7-28 replace server
void conn_close_for_replace_server(struct conn *conn, int err);
#endif //shenzheng 2015-7-28 replace server

#endif
