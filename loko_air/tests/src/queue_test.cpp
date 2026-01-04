#include "CppUTest/TestHarness.h"

#include "queue/queue.h"
#include <stdint.h>
#include <stdlib.h>

size_t lock_counter;
size_t unlock_counter;

void reset_lock_unlock_counters() {
    lock_counter = 0;
    unlock_counter = 0;
}

void lock(void) {
    CHECK_EQUAL(lock_counter, unlock_counter);
    lock_counter++;
}

void unlock(void) {
    unlock_counter++;
    CHECK_EQUAL(lock_counter, unlock_counter);
}

TEST_GROUP(queue_test) {
    void setup() {
        reset_lock_unlock_counters();
        queue_register_locks(lock, unlock);
    }

    void teardown() {
    }
};

TEST(queue_test, check_no_lock_unlock) {
    const int TEST_QUEUE_SIZE = 1;

    queue_register_locks(NULL, NULL);

    QUEUE(test_queue, TEST_QUEUE_SIZE, uint32_t);
    CHECK_EQUAL(true, queue_init(QHEAD(test_queue), TEST_QUEUE_SIZE));
    uint32_t test_val = 0x12345678;
    CHECK_EQUAL(true, queue_enqueue(QHEAD(test_queue), &test_val, enqueue32));
    uint32_t check_val = 0;
    CHECK_EQUAL(true, queue_dequeue(QHEAD(test_queue), &check_val, dequeue32));
    CHECK_EQUAL(test_val, check_val);
}

TEST(queue_test, check_enqueue_dequeue) {
    const int TEST_QUEUE_SIZE = 128;

    QUEUE(test_queue, TEST_QUEUE_SIZE, uint32_t);

    // Zero size, expacted fail
    CHECK_EQUAL(false, queue_init(QHEAD(test_queue), 0));

    CHECK_EQUAL(true, queue_init(QHEAD(test_queue), TEST_QUEUE_SIZE));

    /* should empty for new queue */
    CHECK_EQUAL(true, queue_is_empty(QHEAD(test_queue)));
    /* should  zero  for new queue */
    CHECK_EQUAL(0, queue_num_of(QHEAD(test_queue)));
    /* Check is full, expect fail */
    CHECK_EQUAL(false, queue_is_full(QHEAD(test_queue)));

    /* Fill queue */
    for (uint32_t i = 0; i < TEST_QUEUE_SIZE; i++) {
        CHECK_EQUAL(true, queue_enqueue(QHEAD(test_queue), &i, enqueue32));
    }

    uint32_t test_val = 123456;
    /* Try to add one more, expect fail*/
    CHECK_EQUAL(false, queue_enqueue(QHEAD(test_queue), &test_val, enqueue32));

    /* Check is full */
    CHECK_EQUAL(true, queue_is_full(QHEAD(test_queue)));
    /* Check element count */
    CHECK_EQUAL(TEST_QUEUE_SIZE, queue_num_of(QHEAD(test_queue)));
    /* Check is empty */
    CHECK_EQUAL(false, queue_is_empty(QHEAD(test_queue)));

    /* Get all and check */
    uint32_t check_val = 0;
    for (uint32_t i = 0; i < TEST_QUEUE_SIZE; i++) {
        CHECK_EQUAL(true, queue_dequeue(QHEAD(test_queue), &check_val, dequeue32));
        CHECK_EQUAL(i, check_val);
    }

    /* Try to get one more, expect fail */
    CHECK_EQUAL(false, queue_dequeue(QHEAD(test_queue), &check_val, dequeue32));

    /* Check is full, expect fail */
    CHECK_EQUAL(false, queue_is_full(QHEAD(test_queue)));
    /* Check is empty */
    CHECK_EQUAL(true, queue_is_empty(QHEAD(test_queue)));
    /* Check element count, expect zero*/
    CHECK_EQUAL(0, queue_num_of(QHEAD(test_queue)));
}

TEST(queue_test, check_enqueue_dequeue64) {
    const int TEST_QUEUE_SIZE = 128;

    QUEUE(test_queue, TEST_QUEUE_SIZE, uint64_t);
    CHECK_EQUAL(true, queue_init(QHEAD(test_queue), TEST_QUEUE_SIZE));

    /* should empty for new queue */
    CHECK_EQUAL(true, queue_is_empty(QHEAD(test_queue)));
    /* should  zero  for new queue */
    CHECK_EQUAL(0, queue_num_of(QHEAD(test_queue)));
    /* Check is full, expect fail */
    CHECK_EQUAL(false, queue_is_full(QHEAD(test_queue)));

    /* Fill queue */
    for (uint64_t i = 0; i < TEST_QUEUE_SIZE; i++) {
        CHECK_EQUAL(true, queue_enqueue(QHEAD(test_queue), &i, enqueue64));
    }

    uint64_t test_val = 123456;
    /* Try to add one more, expect fail*/
    CHECK_EQUAL(false, queue_enqueue(QHEAD(test_queue), &test_val, enqueue64));

    /* Check is full */
    CHECK_EQUAL(true, queue_is_full(QHEAD(test_queue)));
    /* Check element count */
    CHECK_EQUAL(TEST_QUEUE_SIZE, queue_num_of(QHEAD(test_queue)));
    /* Check is empty */
    CHECK_EQUAL(false, queue_is_empty(QHEAD(test_queue)));

    /* Get all and check */
    uint64_t check_val = 0;
    for (uint64_t i = 0; i < TEST_QUEUE_SIZE; i++) {
        CHECK_EQUAL(true, queue_dequeue(QHEAD(test_queue), &check_val, dequeue64));
        CHECK_EQUAL(i, check_val);
    }

    /* Try to get one more, expect fail */
    CHECK_EQUAL(false, queue_dequeue(QHEAD(test_queue), &check_val, dequeue64));

    /* Check is full, expect fail */
    CHECK_EQUAL(false, queue_is_full(QHEAD(test_queue)));
    /* Check is empty */
    CHECK_EQUAL(true, queue_is_empty(QHEAD(test_queue)));
    /* Check element count, expect zero*/
    CHECK_EQUAL(0, queue_num_of(QHEAD(test_queue)));
}

TEST(queue_test, check_enqueue_dequeue16) {
    const int TEST_QUEUE_SIZE = 128;

    QUEUE(test_queue, TEST_QUEUE_SIZE, uint16_t);
    CHECK_EQUAL(true, queue_init(QHEAD(test_queue), TEST_QUEUE_SIZE));

    /* should empty for new queue */
    CHECK_EQUAL(true, queue_is_empty(QHEAD(test_queue)));
    /* should  zero  for new queue */
    CHECK_EQUAL(0, queue_num_of(QHEAD(test_queue)));
    /* Check is full, expect fail */
    CHECK_EQUAL(false, queue_is_full(QHEAD(test_queue)));

    /* Fill queue */
    for (uint16_t i = 0; i < TEST_QUEUE_SIZE; i++) {
        CHECK_EQUAL(true, queue_enqueue(QHEAD(test_queue), &i, enqueue16));
    }

    uint16_t test_val = 1234;
    /* Try to add one more, expect fail*/
    CHECK_EQUAL(false, queue_enqueue(QHEAD(test_queue), &test_val, enqueue16));

    /* Check is full */
    CHECK_EQUAL(true, queue_is_full(QHEAD(test_queue)));
    /* Check element count */
    CHECK_EQUAL(TEST_QUEUE_SIZE, queue_num_of(QHEAD(test_queue)));
    /* Check is empty */
    CHECK_EQUAL(false, queue_is_empty(QHEAD(test_queue)));

    /* Get all and check */
    uint16_t check_val = 0;
    for (uint16_t i = 0; i < TEST_QUEUE_SIZE; i++) {
        CHECK_EQUAL(true, queue_dequeue(QHEAD(test_queue), &check_val, dequeue16));
        CHECK_EQUAL(i, check_val);
    }

    /* Try to get one more, expect fail */
    CHECK_EQUAL(false, queue_dequeue(QHEAD(test_queue), &check_val, dequeue16));

    /* Check is full, expect fail */
    CHECK_EQUAL(false, queue_is_full(QHEAD(test_queue)));
    /* Check is empty */
    CHECK_EQUAL(true, queue_is_empty(QHEAD(test_queue)));
    /* Check element count, expect zero*/
    CHECK_EQUAL(0, queue_num_of(QHEAD(test_queue)));
}

TEST(queue_test, check_enqueue_dequeue8) {
    const int TEST_QUEUE_SIZE = 128;

    QUEUE(test_queue, TEST_QUEUE_SIZE, uint8_t);
    CHECK_EQUAL(true, queue_init(QHEAD(test_queue), TEST_QUEUE_SIZE));

    /* should empty for new queue */
    CHECK_EQUAL(true, queue_is_empty(QHEAD(test_queue)));
    /* should  zero  for new queue */
    CHECK_EQUAL(0, queue_num_of(QHEAD(test_queue)));
    /* Check is full, expect fail */
    CHECK_EQUAL(false, queue_is_full(QHEAD(test_queue)));

    /* Fill queue */
    for (uint8_t i = 0; i < TEST_QUEUE_SIZE; i++) {
        CHECK_EQUAL(true, queue_enqueue(QHEAD(test_queue), &i, enqueue8));
    }

    uint8_t test_val = 123;
    /* Try to add one more, expect fail*/
    CHECK_EQUAL(false, queue_enqueue(QHEAD(test_queue), &test_val, enqueue8));

    /* Check is full */
    CHECK_EQUAL(true, queue_is_full(QHEAD(test_queue)));
    /* Check element count */
    CHECK_EQUAL(TEST_QUEUE_SIZE, queue_num_of(QHEAD(test_queue)));
    /* Check is empty */
    CHECK_EQUAL(false, queue_is_empty(QHEAD(test_queue)));

    /* Get all and check */
    uint8_t check_val = 0;
    for (uint8_t i = 0; i < TEST_QUEUE_SIZE; i++) {
        CHECK_EQUAL(true, queue_dequeue(QHEAD(test_queue), &check_val, dequeue8));
        CHECK_EQUAL(i, check_val);
    }

    /* Try to get one more, expect fail */
    CHECK_EQUAL(false, queue_dequeue(QHEAD(test_queue), &check_val, dequeue8));

    /* Check is full, expect fail */
    CHECK_EQUAL(false, queue_is_full(QHEAD(test_queue)));
    /* Check is empty */
    CHECK_EQUAL(true, queue_is_empty(QHEAD(test_queue)));
    /* Check element count, expect zero*/
    CHECK_EQUAL(0, queue_num_of(QHEAD(test_queue)));
}

TEST(queue_test, check_pick) {
    const int TEST_QUEUE_SIZE = 128;

    QUEUE(test_queue, TEST_QUEUE_SIZE, uint32_t);
    CHECK_EQUAL(true, queue_init(QHEAD(test_queue), TEST_QUEUE_SIZE));

    /* Check fail result */
    uint32_t test_val = 123456;
    CHECK_EQUAL(false, queue_pick(QHEAD(test_queue), &test_val, enqueue32));

    /* Fill queue */
    for (uint32_t i = 0; i < TEST_QUEUE_SIZE; i++) {
        CHECK_EQUAL(true, queue_enqueue(QHEAD(test_queue), &i, enqueue32));
    }

    /* Replace oldest */
    CHECK_EQUAL(true, queue_pick(QHEAD(test_queue), &test_val, enqueue32));
    /* Get oldest and check */
    uint32_t check_val = 0;
    CHECK_EQUAL(true, queue_dequeue(QHEAD(test_queue), &check_val, dequeue32));
    CHECK_EQUAL(test_val, check_val);

    /* Get other and check */
    for (uint32_t i = 1; i < TEST_QUEUE_SIZE; i++) {
        CHECK_EQUAL(true, queue_dequeue(QHEAD(test_queue), &check_val, dequeue32));
        CHECK_EQUAL(i, check_val);
    }
}

TEST(queue_test, check_peek_newest) {
    const int TEST_QUEUE_SIZE = 128;
    const uint32_t RANDOM_VAL = (uint32_t)rand();

    QUEUE(test_queue, TEST_QUEUE_SIZE, uint32_t);
    CHECK_EQUAL(true, queue_init(QHEAD(test_queue), TEST_QUEUE_SIZE));

    uint32_t check_val = 0;
    /* Peek for empty queue should be fail */
    CHECK_EQUAL(false, queue_peek_newest(QHEAD(test_queue), &check_val, dequeue32));

    /* Fill queue */
    for (uint32_t i = 0; i < TEST_QUEUE_SIZE; i++) {
        uint32_t test_val = RANDOM_VAL + i;
        CHECK_EQUAL(true, queue_enqueue(QHEAD(test_queue), &test_val, enqueue32));
    }

    /* Check total elements */
    CHECK_EQUAL(TEST_QUEUE_SIZE, queue_num_of(QHEAD(test_queue)));

    /* peel and check value */
    CHECK_EQUAL(true, queue_peek_newest(QHEAD(test_queue), &check_val, dequeue32));
    CHECK_EQUAL(check_val, RANDOM_VAL + TEST_QUEUE_SIZE - 1);

    /* Check total elements, should be the same count */
    CHECK_EQUAL(TEST_QUEUE_SIZE, queue_num_of(QHEAD(test_queue)));
}

TEST(queue_test, check_peek_particular_from_newest) {
    const int TEST_QUEUE_SIZE = 128;
    const uint32_t RANDOM_VAL = (uint32_t)rand();

    QUEUE(test_queue, TEST_QUEUE_SIZE, uint32_t);
    CHECK_EQUAL(true, queue_init(QHEAD(test_queue), TEST_QUEUE_SIZE));

    uint32_t check_val = 0;
    /* Queue is empty - Expect fail result */
    CHECK_EQUAL(false, queue_peek_particular_from_newest(QHEAD(test_queue), &check_val, dequeue32, 0));

    /* Fill queue */
    for (uint32_t i = 0; i < TEST_QUEUE_SIZE; i++) {
        uint32_t test_val = RANDOM_VAL + i;
        CHECK_EQUAL(true, queue_enqueue(QHEAD(test_queue), &test_val, enqueue32));
    }

    /* Check element count, expected TOTAL */
    CHECK_EQUAL(TEST_QUEUE_SIZE, queue_num_of(QHEAD(test_queue)));

    /* Peek and check last element */
    CHECK_EQUAL(true, queue_peek_particular_from_newest(QHEAD(test_queue), &check_val, dequeue32, 0));
    CHECK_EQUAL(check_val, RANDOM_VAL + TEST_QUEUE_SIZE - 1);

    /* Peek and check first element */
    CHECK_EQUAL(true, queue_peek_particular_from_newest(QHEAD(test_queue), &check_val, dequeue32, TEST_QUEUE_SIZE - 1));
    CHECK_EQUAL(check_val, RANDOM_VAL + 0);

    /* Out of renge pick - Expect fail result */
    CHECK_EQUAL(false,
                queue_peek_particular_from_newest(QHEAD(test_queue), &check_val, dequeue32, TEST_QUEUE_SIZE + 1));

    /* Check element count, still expected TOTAL */
    CHECK_EQUAL(TEST_QUEUE_SIZE, queue_num_of(QHEAD(test_queue)));
}

TEST(queue_test, check_overwrite_latest) {
    const int TEST_QUEUE_SIZE = 128;

    QUEUE(test_queue, TEST_QUEUE_SIZE, uint32_t);
    CHECK_EQUAL(true, queue_init(QHEAD(test_queue), TEST_QUEUE_SIZE));

    /* Fill queue */
    for (uint32_t i = 0; i < TEST_QUEUE_SIZE; i++) {
        queue_enqueue_drop_first(QHEAD(test_queue), &i, enqueue32);
    }

    /* Check element count, expected TOTAL */
    CHECK_EQUAL(TEST_QUEUE_SIZE, queue_num_of(QHEAD(test_queue)));

    uint32_t check_val = TEST_QUEUE_SIZE + 1;
    /* Add one more and drop first  */
    queue_enqueue_drop_first(QHEAD(test_queue), &check_val, enqueue32);

    /* Get other and check */
    for (uint32_t i = 1; i < TEST_QUEUE_SIZE; i++) {
        CHECK_EQUAL(true, queue_dequeue(QHEAD(test_queue), &check_val, dequeue32));
        CHECK_EQUAL(i, check_val);
    }

    CHECK_EQUAL(true, queue_dequeue(QHEAD(test_queue), &check_val, dequeue32));
    CHECK_EQUAL(TEST_QUEUE_SIZE + 1, check_val);

    /* Check element count, still expected TOTAL */
    CHECK_EQUAL(0, queue_num_of(QHEAD(test_queue)));
}

TEST(queue_test, check_long_usage) {
    const int TEST_QUEUE_SIZE = 20;

    QUEUE(test_queue, TEST_QUEUE_SIZE, uint32_t);
    CHECK_EQUAL(true, queue_init(QHEAD(test_queue), TEST_QUEUE_SIZE));

    uint32_t in_index = 0;
    uint32_t out_index = 0;

    /* Fill queue */
    for (uint32_t i = 0; i < TEST_QUEUE_SIZE - 1; i++) {
        CHECK_EQUAL(true, queue_enqueue(QHEAD(test_queue), &in_index, enqueue32));
        in_index++;
    }

    /* push then pop and check */
    for (uint32_t i = 0; i < UINT16_MAX; i++) {
        CHECK_EQUAL(true, queue_enqueue(QHEAD(test_queue), &in_index, enqueue32));
        in_index++;

        uint32_t check_val = 0;
        CHECK_EQUAL(true, queue_dequeue(QHEAD(test_queue), &check_val, dequeue32));
        CHECK_EQUAL(out_index, check_val);
        out_index++;
    }

    uint32_t test_val = 123456;
    /* Pop last, expected true*/
    CHECK_EQUAL(true, queue_enqueue(QHEAD(test_queue), &test_val, enqueue32));
    /* Pop last more, expected false*/
    CHECK_EQUAL(false, queue_enqueue(QHEAD(test_queue), &test_val, enqueue32));
}
