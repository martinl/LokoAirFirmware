#include "queue.h"
#include <stddef.h>

static void (*_lock_cb)(void) = NULL;
static void (*_unlock_cb)(void) = NULL;

static void _lock(void) {
    if (_lock_cb) {
        _lock_cb();
    }
}

static void _unlock(void) {
    if (_unlock_cb) {
        _unlock_cb();
    }
}

void queue_register_locks(void (*lock)(void), void (*unlock)(void)) {
    _lock_cb = lock;
    _unlock_cb = unlock;
}

bool queue_is_full(queue_t const *q) {
    if (q->count >= q->total_count) {
        return true;
    }

    return false;
}

bool queue_is_empty(queue_t const *q) {
    if (0 == q->count) {
        return true;
    }

    return false;
}

bool queue_init(queue_t *q, size_t count) {
    if (count == 0) {
        return false;
    }

    _lock();

    q->total_count = count;
    q->last = 0;
    q->first = 0;
    q->count = 0;

    _unlock();

    return true;
}

bool queue_enqueue(queue_t *q, void *data, enqueue fn) {
    if (queue_is_full(q)) {
        return false;
    }

    _lock();

    fn(&(q->obj_head), q->last, data);
    q->last = (q->last + 1) % q->total_count;
    q->count++;

    _unlock();

    return true;
}

void queue_enqueue_drop_first(queue_t *q, void *data, enqueue fn) {
    _lock();

    if (queue_is_full(q)) {
        q->first = (q->first + 1) % q->total_count;
    } else {
        q->count++;
    }

    fn(&(q->obj_head), q->last, data);
    q->last = (q->last + 1) % q->total_count;

    _unlock();
}

bool queue_dequeue(queue_t *q, void *data, dequeue fn) {
    if (queue_is_empty(q)) {
        return false;
    }

    _lock();

    fn(&(q->obj_head), q->first, data);
    q->first = (q->first + 1) % q->total_count;
    q->count--;

    _unlock();

    return true;
}

bool queue_pick(queue_t *q, void *data, dequeue fn) {
    if (queue_is_empty(q)) {
        return false;
    }

    fn(&(q->obj_head), q->first, data);

    return true;
}

bool queue_peek_newest(queue_t *q, void *data, dequeue fn) {
    if (queue_is_empty(q)) {
        return false;
    }

    _lock();

    size_t index = (q->first + q->count - 1) % q->total_count;
    fn(&(q->obj_head), index, data);

    _unlock();

    return true;
}

bool queue_peek_particular_from_newest(queue_t *q, void *data, dequeue fn, size_t position) {
    if (queue_is_empty(q)) {
        return false;
    }

    if (position > (q->count - 1)) {
        return false;
    }

    _lock();

    size_t index = (q->first + q->count - 1 - position) % q->total_count;
    fn(&(q->obj_head), index, data);

    _unlock();

    return true;
}

size_t queue_num_of(queue_t const *q) {
    return q->count;
}

void enqueue8(void *obj_head, size_t idx, void *data) {
    ((uint8_t *)obj_head)[idx] = *((uint8_t *)data);
}

void enqueue16(void *obj_head, size_t idx, void *data) {
    ((uint16_t *)obj_head)[idx] = *((uint16_t *)data);
}

void enqueue32(void *obj_head, size_t idx, void *data) {
    ((uint32_t *)obj_head)[idx] = *((uint32_t *)data);
}

void enqueue64(void *obj_head, size_t idx, void *data) {
    ((uint64_t *)obj_head)[idx] = *((uint64_t *)data);
}

void dequeue8(void *obj_head, size_t idx, void *data) {
    *((uint8_t *)data) = ((uint8_t *)obj_head)[idx];
}

void dequeue16(void *obj_head, size_t idx, void *data) {
    *((uint16_t *)data) = ((uint16_t *)obj_head)[idx];
}

void dequeue32(void *obj_head, size_t idx, void *data) {
    *((uint32_t *)data) = ((uint32_t *)obj_head)[idx];
}

void dequeue64(void *obj_head, size_t idx, void *data) {
    *((uint64_t *)data) = ((uint64_t *)obj_head)[idx];
}
