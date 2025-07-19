#ifndef PQUV_H
#define PQUV_H

#include <libpq-fe.h>
#include "uv.h"

// Forward declarations
typedef struct pg_async pg_async_t;
typedef struct pg_query pg_query_t;

// Result callback function type
typedef void (*pg_result_cb_t)(pg_async_t *pg, PGresult *result, void *data);

// Query structure
struct pg_query
{
    char *sql;
    char **params;
    int param_count;
    pg_result_cb_t result_cb;
    void *data;
    pg_query_t *next;
};

// Main async PostgreSQL context
struct pg_async
{
    PGconn *conn;
    int owns_connection;
    int is_connected;
    int is_executing;
    int handle_initialized;

#ifdef _WIN32
    uv_timer_t timer;
#else
    uv_poll_t poll;
#endif

    pg_query_t *query_queue;
    pg_query_t *query_queue_tail;
    pg_query_t *current_query;

    char *error_message;
    void *data; // User data
};

// Public API functions
pg_async_t *pquv_create(PGconn *existing_conn, void *data);
int pquv_queue(pg_async_t *pg,
               const char *sql,
               int param_count,
               const char **params,
               pg_result_cb_t result_cb,
               void *query_data);
int pquv_execute(pg_async_t *pg);

#endif