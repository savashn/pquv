# pquv

A library that integrates [libpq](https://www.postgresql.org/docs/current/libpq.html) with [libuv](https://libuv.org/) to simplify running asynchronous Postgres queries in libuv-based projects.

## Usage

`pquv.h` is providing three functions:

- `pquv.create()` to create an async operation. It takes 2 parameters: a context and database connection.
- `pquv.queue()` to queue a database query. It takes 5 parameters: the variable that created with pq.create(), the SQL query, the count of params, the result callback, and the context.
- `pquv.execute()` to execute the async operation. It takes 1 parameter: the variable that created with pq.create().

```c
#include "pquv.h"

// PostgreSQL connection info
#define PG_CONNINFO "host=localhost user=postgres dbname=postgres password=yourpassword"

// Callback
void on_result(pg_async_t *pg, PGresult *result, void *data)
{
    ExecStatusType status = PQresultStatus(result);
    if (status != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "Query failed: %s\n", PQresultErrorMessage(result));
        return;
    }

    int rows = PQntuples(result);
    int cols = PQnfields(result);

    printf("Received %d row(s):\n", rows);
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            printf("%s = %s\t", PQfname(result, j), PQgetvalue(result, i, j));
        }
        printf("\n");
    }
}

int main()
{
    // Create PostgreSQL connection
    PGconn *conn = PQconnectdb(PG_CONNINFO);
    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Connection failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return 1;
    }

    // Create an async operation
    pg_async_t *pg = pquv_create(conn, NULL);
    if (!pg)
    {
        fprintf(stderr, "Failed to create pg_async_t\n");
        PQfinish(conn);
        return 1;
    }

    // Queue the query
    const char *sql = "SELECT NOW() AS current_time";
    if (pquv_queue(pg, sql, 0, NULL, on_result, NULL) != 0)
    {
        fprintf(stderr, "Failed to queue query\n");
        return 1;
    }

    // Execute the queue
    if (pquv_execute(pg) != 0)
    {
        fprintf(stderr, "Failed to execute query\n");
        return 1;
    }

    // Run libuv
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    return 0;
}
```

## Advanced Usage

Here is an [advanced example usage](https://ecewo.vercel.app/docs/async-operations/#async-postgres-queries) with [Ecewo](https://github.com/savashn/ecewo), which is a minimalist C framework based on libuv.
