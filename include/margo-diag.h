/*
 * (C) 2015 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */

#include <stdio.h>      /* defines printf for tests */
#include <time.h>       /* defines time_t for timings in the test */
#include <stdint.h>     /* defines uint32_t etc */
#include <sys/param.h>  /* attempt to define endianness */
#ifdef linux
#include <endian.h>    /* attempt to define endianness */
#include <sys/time.h>
#include <sys/resource.h>
#endif

#ifndef __MARGO_DIAG
#define __MARGO_DIAG

#ifdef __cplusplus
extern "C" {
#endif

#define GET_SELF_ADDR_STR(__mid, __addr_str) do { \
     hg_addr_t __self_addr; \
     hg_size_t __size; \
     __addr_str = NULL; \
     if (margo_addr_self(__mid, &__self_addr) != HG_SUCCESS) break; \
     if (margo_addr_to_string(__mid, NULL, &__size, __self_addr) != HG_SUCCESS) { \
         margo_addr_free(__mid, __self_addr); \
         break; \
     } \
     if ((__addr_str = malloc(__size)) == NULL) { \
         margo_addr_free(__mid, __self_addr); \
         break; \
     } \
     if (margo_addr_to_string(__mid, __addr_str, &__size, __self_addr) != HG_SUCCESS) { \
         free(__addr_str); \
         __addr_str = NULL; \
         margo_addr_free(__mid, __self_addr); \
         break; \
     } \
     margo_addr_free(__mid, __self_addr); \
} while(0)

/******************************************************************************************************/

/* used to identify a globally unique breadcrumb */
struct global_breadcrumb_key
{
  uint64_t rpc_breadcrumb; /* a.k.a RPC callpath */
  uint64_t addr_hash; /* hash of server addr */
  uint16_t provider_id; /* provider_id within a server. NOT a globally unique identifier */
};

enum breadcrumb_type
{
  origin, target
};

typedef enum breadcrumb_type breadcrumb_type;

struct breadcrumb_stats
{
    /* stats for breadcrumb call times */
    double min;
    double max;
    double cumulative;

    /* additional stats */
    double handler_time;
    double completion_callback_time;
    double internal_rdma_transfer_time;
    double input_serial_time;
    double input_deserial_time;
    double output_serial_time;
    size_t internal_rdma_transfer_size;
    double bulk_transfer_time;
    double operation_time;

    /* stats for RPC handler pool sizes */
    /* Total pool size = Total number of runnable items + items waiting on a lock */
    unsigned long abt_pool_total_size_lwm; /* low watermark */
    unsigned long abt_pool_total_size_hwm; /* high watermark */
    unsigned long abt_pool_total_size_cumulative;

    unsigned long abt_pool_size_lwm; /* low watermark */
    unsigned long abt_pool_size_hwm; /* high watermark */
    unsigned long abt_pool_size_cumulative;

    /* count of occurrences of breadcrumb */
    unsigned long count;
};

typedef struct breadcrumb_stats breadcrumb_stats;

/* structure to store breadcrumb snapshot, for consumption outside of margo.
   reflects the margo-internal structure used to hold diagnostic data */
struct margo_breadcrumb
{
    breadcrumb_stats stats;
    /* 0 is this is a origin-side breadcrumb, 1 if this is a target-side breadcrumb */
    breadcrumb_type type;

    struct global_breadcrumb_key key;

    struct margo_breadcrumb* next;
};

/* snapshot contains linked list of breadcrumb data */
struct margo_breadcrumb_snapshot
{
  struct margo_breadcrumb* ptr;
};

/* Request tracing definitions */
enum ev_type
{
  cs, sr, ss, cr
};

typedef enum ev_type ev_type;

struct trace_metadata
{
   size_t abt_pool_size;
   size_t abt_pool_total_size;
   uint64_t mid;
   #ifdef linux
   struct rusage usage;
   #endif
};

struct margo_trace_record
{
  uint64_t trace_id;
  double ts;
  uint64_t rpc;
  size_t ofi_events_read;
  ev_type ev;
  uint64_t order;
  struct trace_metadata metadata;
  double bulk_transfer_bw;
  double bulk_transfer_start;
  double bulk_transfer_end;
  double operation_start;
  double operation_stop;
  size_t operation_size;
  double operation_bw;
  char name[30];
};

struct margo_system_stat
{
  size_t abt_pool_size;
  size_t abt_pool_total_size;
  double system_cpu_util;
  double system_memory_util;
  double loadavg_1m;
  double loadavg_5m;
  double loadavg_15m;
  double ts;
};

struct request_metadata
{
  uint64_t rpc_breadcrumb;
  uint64_t trace_id;
  uint64_t order;
  uint64_t current_rpc;
};

typedef struct request_metadata request_metadata;
typedef struct margo_trace_record margo_trace_record;
typedef struct margo_system_stat margo_system_stat;

#ifdef __cplusplus
}
#endif

#endif /* __MARGO_DIAG */
