#ifndef __QUEUE_H__
#define __QUEUE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct queue_s {
    size_t total_count;
    size_t last;
    size_t first;
    size_t count;
    /* lock goes here */
    size_t obj_head;
} queue_t;

#define QUEUE(name, N, _dtype) \
    struct {                   \
        size_t total_count;    \
        size_t rear;           \
        size_t front;          \
        size_t count;          \
        /* lock goes here */   \
        _dtype obj_head[N];    \
    } name

#define QHEAD(_queue) ((queue_t *)(&(_queue)))

typedef void (*enqueue)(void *obj_head, size_t idx, void *data);
typedef void (*dequeue)(void *obj_head, size_t idx, void *data);

bool queue_is_full(queue_t const *q);
bool queue_is_empty(queue_t const *q);
bool queue_init(queue_t *q, size_t count);
bool queue_enqueue(queue_t *q, void *data, enqueue fn);
void queue_enqueue_drop_first(queue_t *q, void *data, enqueue fn);
bool queue_dequeue(queue_t *q, void *data, dequeue fn);
bool queue_pick(queue_t *q, void *data, dequeue fn);
bool queue_peek_newest(queue_t *q, void *data, dequeue fn);
bool queue_peek_particular_from_newest(queue_t *q, void *data, dequeue fn, size_t position);
size_t queue_num_of(queue_t const *q);
void queue_register_locks(void (*lock)(void), void (*unlock)(void));

void enqueue8(void *obj_head, size_t idx, void *data);
void enqueue16(void *obj_head, size_t idx, void *data);
void enqueue32(void *obj_head, size_t idx, void *data);
void enqueue64(void *obj_head, size_t idx, void *data);

void dequeue8(void *obj_head, size_t idx, void *data);
void dequeue16(void *obj_head, size_t idx, void *data);
void dequeue32(void *obj_head, size_t idx, void *data);
void dequeue64(void *obj_head, size_t idx, void *data);

/* ----- cpp protection ----------------------------------------------------- */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // __QUEUE_H__
