/*
 * Dynomite - A thin, distributed replication layer for multi non-distributed storages.
 * Copyright (C) 2014 Netflix, Inc.
 */ 

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

#include "dyn_core.h"

#ifndef _DYN_STATS_H_
#define _DYN_STATS_H_



#define STATS_POOL_CODEC(ACTION)                                                                                          \
    /* client behavior */                                                                                                 \
    ACTION( client_eof,                   STATS_COUNTER,      "# eof on client connections")                              \
    ACTION( client_err,                   STATS_COUNTER,      "# errors on client connections")                           \
    ACTION( client_connections,           STATS_GAUGE,        "# active client connections")                              \
    /* pool behavior */                                                                                                   \
    ACTION( server_ejects,                STATS_COUNTER,      "# times backend server was ejected")                       \
    /* dnode client behavior */                                                                                           \
    ACTION( dnode_client_eof,             STATS_COUNTER,      "# eof on dnode client connections")                        \
    ACTION( dnode_client_err,             STATS_COUNTER,      "# errors on dnode client connections")                     \
    ACTION( dnode_client_connections,     STATS_GAUGE,        "# active dnode client connections")                        \
    /* peer behavior */                                                                                                   \
    ACTION( peer_eof,                     STATS_COUNTER,              "# eof on peer connections")                        \
    ACTION( peer_err,                     STATS_COUNTER,              "# errors on peer connections")                     \
    ACTION( peer_timedout,                STATS_COUNTER,              "# timeouts on peer connections")                   \
    ACTION( peer_connections,             STATS_GAUGE,                "# active peer connections")                        \
    ACTION( peer_forward_error,           STATS_GAUGE,        "# times we encountered a peer forwarding error")           \
    ACTION( peer_requests,                STATS_COUNTER,      "# peer requests")                                          \
    ACTION( peer_request_bytes,           STATS_COUNTER,      "total peer request bytes")                                 \
    ACTION( peer_responses,               STATS_COUNTER,      "# peer respones")                                          \
    ACTION( peer_response_bytes,          STATS_COUNTER,      "total peer response bytes")                                \
    ACTION( peer_ejects,                  STATS_COUNTER,      "# times a peer was ejected")                               \
    ACTION( peer_in_queue,                STATS_GAUGE,        "# peer requests in incoming queue")                        \
    ACTION( peer_in_queue_bytes,          STATS_GAUGE,        "current peer request bytes in incoming queue")             \
    ACTION( peer_out_queue,               STATS_GAUGE,        "# peer requests in outgoing queue")                        \
    ACTION( peer_out_queue_bytes,         STATS_GAUGE,        "current peer request bytes in outgoing queue")             \
    /* forwarder behavior */                                                                                              \
    ACTION( forward_error,                STATS_COUNTER,      "# times we encountered a forwarding error")                \
    ACTION( fragments,                    STATS_COUNTER,      "# fragments created from a multi-vector request")          \
    ACTION( stats_count,                  STATS_COUNTER,      "# stats request")                                          \

#define STATS_SERVER_CODEC(ACTION)                                                                                  \
    /* server behavior */                                                                                           \
    ACTION( server_eof,             STATS_COUNTER,      "# eof on server connections")                              \
    ACTION( server_err,             STATS_COUNTER,      "# errors on server connections")                           \
    ACTION( server_timedout,        STATS_COUNTER,      "# timeouts on server connections")                         \
    ACTION( server_connections,     STATS_GAUGE,        "# active server connections")                              \
    ACTION( server_ejected_at,      STATS_TIMESTAMP,    "timestamp when server was ejected in usec since epoch")    \
    /* data behavior */                                                                                             \
    ACTION( requests,               STATS_COUNTER,      "# requests")                                               \
    ACTION( request_bytes,          STATS_COUNTER,      "total request bytes")                                      \
    ACTION( responses,              STATS_COUNTER,      "# respones")                                               \
    ACTION( response_bytes,         STATS_COUNTER,      "total response bytes")                                     \
    ACTION( in_queue,               STATS_GAUGE,        "# requests in incoming queue")                             \
    ACTION( in_queue_bytes,         STATS_GAUGE,        "current request bytes in incoming queue")                  \
    ACTION( out_queue,              STATS_GAUGE,        "# requests in outgoing queue")                             \
    ACTION( out_queue_bytes,        STATS_GAUGE,        "current request bytes in outgoing queue")                  \


#define STATS_ADDR      "0.0.0.0"
#define STATS_PORT      22222
#define STATS_INTERVAL  (30 * 1000) /* in msec */

typedef enum stats_type {
    STATS_INVALID,
    STATS_COUNTER,    /* monotonic accumulator */
    STATS_GAUGE,      /* non-monotonic accumulator */
    STATS_TIMESTAMP,  /* monotonic timestamp (in nsec) */
    STATS_STRING,
    STATS_SENTINEL
} stats_type_t;

struct stats_metric {
    stats_type_t  type;         /* type */
    struct string name;         /* name (ref) */
    union {
        int64_t   counter;      /* accumulating counter */
        int64_t   timestamp;    /* monotonic timestamp */
        struct string str;     /* store string value */
    } value;
};

struct stats_dnode {
    struct string name;   /* dnode server name (ref) */
    struct array  metric; /* stats_metric[] for dnode server codec */
};

struct stats_server {
    struct string name;   /* server name (ref) */
    struct array  metric; /* stats_metric[] for server codec */
};

struct stats_pool {
    struct string name;   /* pool name (ref) */
    struct array  metric; /* stats_metric[] for pool codec */
    struct array  server; /* stats_server[] */
};

struct stats_buffer {
    size_t   len;   /* buffer length */
    uint8_t  *data; /* buffer data */
    size_t   size;  /* buffer alloc size */
};

struct stats {
    struct context      *ctx;
    uint16_t            port;           /* stats monitoring port */
    int                 interval;       /* stats aggregation interval */
    struct string       addr;           /* stats monitoring address */

    int64_t             start_ts;       /* start timestamp of dynomite */
    struct stats_buffer buf;            /* output buffer */

    struct array        current;        /* stats_pool[] (a) */
    struct array        shadow;         /* stats_pool[] (b) */
    struct array        sum;            /* stats_pool[] (c = a + b) */

    pthread_t           tid;            /* stats aggregator thread */
    int                 sd;             /* stats descriptor */

    struct string       service_str;    /* service string */
    struct string       service;        /* service */
    struct string       source_str;     /* source string */
    struct string       source;         /* source */
    struct string       version_str;    /* version string */
    struct string       version;        /* version */
    struct string       uptime_str;     /* uptime string */
    struct string       timestamp_str;  /* timestamp string */

    struct string       datacenter_str;
    struct string       datacenter;

    struct string       region_str;
    struct string       region;

    volatile int        aggregate;      /* shadow (b) aggregate? */
    volatile int        updated;        /* current (a) updated? */
};

#define DEFINE_ACTION(_name, _type, _desc) STATS_POOL_##_name,
typedef enum stats_pool_field {
    STATS_POOL_CODEC(DEFINE_ACTION)
    STATS_POOL_NFIELD
} stats_pool_field_t;
#undef DEFINE_ACTION

#define DEFINE_ACTION(_name, _type, _desc) STATS_SERVER_##_name,
typedef enum stats_server_field {
    STATS_SERVER_CODEC(DEFINE_ACTION)
    STATS_SERVER_NFIELD
} stats_server_field_t;
#undef DEFINE_ACTION


typedef enum stats_cmd {
    STATS_INFO,
    STATS_PING,
    STATS_DESCRIBE,
    STATS_STANDBY,
    STATS_WRITES_ONLY,
    STATS_RESUMING,
    STATS_NORMAL,
    STATS_BOOTSTRAPING,
    STATS_LEAVING,
    STATS_UNKNOWN
} stats_cmd_t;


#if defined DN_STATS && DN_STATS == 1

#define stats_pool_incr(_ctx, _pool, _name) do {                        \
    _stats_pool_incr(_ctx, _pool, STATS_POOL_##_name);                  \
} while (0)

#define stats_pool_decr(_ctx, _pool, _name) do {                        \
    _stats_pool_decr(_ctx, _pool, STATS_POOL_##_name);                  \
} while (0)

#define stats_pool_incr_by(_ctx, _pool, _name, _val) do {               \
    _stats_pool_incr_by(_ctx, _pool, STATS_POOL_##_name, _val);         \
} while (0)

#define stats_pool_decr_by(_ctx, _pool, _name, _val) do {               \
    _stats_pool_decr_by(_ctx, _pool, STATS_POOL_##_name, _val);         \
} while (0)

#define stats_pool_set_ts(_ctx, _pool, _name, _val) do {                \
    _stats_pool_set_ts(_ctx, _pool, STATS_POOL_##_name, _val);          \
} while (0)

#define stats_server_incr(_ctx, _server, _name) do {                    \
    _stats_server_incr(_ctx, _server, STATS_SERVER_##_name);            \
} while (0)

#define stats_server_decr(_ctx, _server, _name) do {                    \
    _stats_server_decr(_ctx, _server, STATS_SERVER_##_name);            \
} while (0)

#define stats_server_incr_by(_ctx, _server, _name, _val) do {           \
    _stats_server_incr_by(_ctx, _server, STATS_SERVER_##_name, _val);   \
} while (0)

#define stats_server_decr_by(_ctx, _server, _name, _val) do {           \
    _stats_server_decr_by(_ctx, _server, STATS_SERVER_##_name, _val);   \
} while (0)

#define stats_server_set_ts(_ctx, _server, _name, _val) do {            \
     _stats_server_set_ts(_ctx, _server, STATS_SERVER_##_name, _val);   \
} while (0)


#else

#define stats_pool_incr(_ctx, _pool, _name)

#define stats_pool_decr(_ctx, _pool, _name)

#define stats_pool_incr_by(_ctx, _pool, _name, _val)

#define stats_pool_decr_by(_ctx, _pool, _name, _val)

#define stats_server_incr(_ctx, _server, _name)

#define stats_server_decr(_ctx, _server, _name)

#define stats_server_incr_by(_ctx, _server, _name, _val)

#define stats_server_decr_by(_ctx, _server, _name, _val)

///for dnode
//#define stats_dnode_incr(_ctx, _server, _name)

//#define stats_dnode_decr(_ctx, _server, _name)

//#define stats_dnode_incr_by(_ctx, _server, _name, _val)

//#define stats_dnode_decr_by(_ctx, _server, _name, _val)


#endif

#define stats_enabled   DN_STATS

void stats_describe(void);

void _stats_pool_incr(struct context *ctx, struct server_pool *pool, stats_pool_field_t fidx);
void _stats_pool_decr(struct context *ctx, struct server_pool *pool, stats_pool_field_t fidx);
void _stats_pool_incr_by(struct context *ctx, struct server_pool *pool, stats_pool_field_t fidx, int64_t val);
void _stats_pool_decr_by(struct context *ctx, struct server_pool *pool, stats_pool_field_t fidx, int64_t val);
void _stats_pool_set_ts(struct context *ctx, struct server_pool *pool, stats_pool_field_t fidx, int64_t val);

void _stats_server_incr(struct context *ctx, struct server *server, stats_server_field_t fidx);
void _stats_server_decr(struct context *ctx, struct server *server, stats_server_field_t fidx);
void _stats_server_incr_by(struct context *ctx, struct server *server, stats_server_field_t fidx, int64_t val);
void _stats_server_decr_by(struct context *ctx, struct server *server, stats_server_field_t fidx, int64_t val);
void _stats_server_set_ts(struct context *ctx, struct server *server, stats_server_field_t fidx, int64_t val);

//for dnode
//void _stats_dnode_incr(struct context *ctx, struct server *server, stats_dnode_field_t fidx);
//void _stats_dnode_decr(struct context *ctx, struct server *server, stats_dnode_field_t fidx);
//void _stats_dnode_incr_by(struct context *ctx, struct server *server, stats_dnode_field_t fidx, int64_t val);
//void _stats_dnode_decr_by(struct context *ctx, struct server *server, stats_dnode_field_t fidx, int64_t val);
//void _stats_dnode_set_ts(struct context *ctx, struct server *server, stats_dnode_field_t fidx, int64_t val);


struct stats *stats_create(uint16_t stats_port, char *stats_ip, int stats_interval, char *source,
		                   struct array *server_pool, struct context *ctx);
void stats_destroy(struct stats *stats);
void stats_swap(struct stats *stats);

#endif