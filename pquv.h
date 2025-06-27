#ifndef PQUV_H
#define PQUV_H

#include <libpq-fe.h>
#include <uv.h>

// Forward declarations
typedef struct pg_async_s pg_async_t;
typedef struct pg_query_s pg_query_t;

// Callback function types
typedef void (*pg_result_cb_t)(pg_async_t *pg, PGresult *result, void *data);
typedef void (*pg_error_cb_t)(pg_async_t *pg, const char *error, void *data);
typedef void (*pg_complete_cb_t)(pg_async_t *pg, void *data);

// Query structure
struct pg_query_s
{
    char *sql;
    char **params;
    int param_count;
    pg_result_cb_t result_cb;
    void *data;
    struct pg_query_s *next;
};

// Main async PostgreSQL context
struct pg_async_s
{
    PGconn *conn;
    int owns_connection;
    int is_connected;
    int is_executing;

    // Callback
    void *data;

    // Query queue
    pg_query_t *query_queue;
    pg_query_t *query_queue_tail;
    pg_query_t *current_query;

    // Platform-specific handles
#ifdef _WIN32
    uv_timer_t timer;
#else
    uv_poll_t poll;
#endif

    // Error message storage
    char *error_message;
};

// Create new async PostgreSQL context (full interface)
// Note: Context will automatically cleanup when all queries complete or on error
pg_async_t *pquv_create(PGconn *existing_conn, void *data);

// Add query to execution queue
int pquv_queue(pg_async_t *pg,
               const char *sql,
               int param_count,
               const char **params,
               pg_result_cb_t result_cb,
               void *query_data);

// Start executing queued queries
// Note: Context will automatically cleanup when execution completes
int pquv_execute(pg_async_t *pg);

#endif // PQUV_H
