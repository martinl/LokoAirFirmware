#include "CppUTest/TestHarness.h"

#include <stdlib.h>
#include <string.h>

#include <lwgps.h>

TEST_GROUP(lwgnps_test) {
    lwgps_t lwgps;
    void setup() {
        lwgps_init(&lwgps);
    }

    void teardown() {
    }
};

TEST(lwgnps_test, init_state) {

    CHECK_EQUAL(0, lwgps.latitude);
    CHECK_EQUAL(0, lwgps.longitude);
    CHECK_EQUAL(0, lwgps.altitude);
    CHECK_EQUAL(0, lwgps.geo_sep);
    CHECK_EQUAL(0, lwgps.sats_in_use);
    CHECK_EQUAL(0, lwgps.fix);
    CHECK_EQUAL(0, lwgps.hours);
    CHECK_EQUAL(0, lwgps.minutes);
    CHECK_EQUAL(0, lwgps.seconds);
}

TEST(lwgnps_test, noise_reception) {

    for (size_t i = 0; i < 10000; i++) {

        char random_symbol = (char)random();
        lwgps_process(&lwgps, &random_symbol, 1);
        lwgps_process(&lwgps, "\r\n", 2);
        CHECK_EQUAL(0, lwgps.latitude);
        CHECK_EQUAL(0, lwgps.longitude);
        CHECK_EQUAL(0, lwgps.altitude);
        CHECK_EQUAL(0, lwgps.geo_sep);
        CHECK_EQUAL(0, lwgps.sats_in_use);
        CHECK_EQUAL(0, lwgps.fix);
        CHECK_EQUAL(0, lwgps.hours);
        CHECK_EQUAL(0, lwgps.minutes);
        CHECK_EQUAL(0, lwgps.seconds);
    }
}

TEST(lwgnps_test, test_gprmc) {
    char test1[] = "$GPRMC,203522.00,A,5109.0262308,N,11401.8407342,W,0.004,133.4,130522,0.0,E,D*2B\r\n";
    lwgps_process(&lwgps, test1, strlen(test1));
    CHECK_EQUAL(133.4, lwgps.course);

    char test2[] = "$GNRMC,204520.00,A,5109.0262239,N,11401.8407338,W,0.004,102.3,130522,0.0,E,D*3B\r\n";
    lwgps_process(&lwgps, test2, strlen(test2));
    CHECK_EQUAL(102.3, lwgps.course);
}

TEST(lwgnps_test, test_GPGGA) {
    char test1[] = "$GPGGA,202530.00,5109.0262,N,11401.8407,W,5,40,0.5,1097.36,M,-17.00,M,18,TSTR*61\r\n";
    lwgps_process(&lwgps, test1, strlen(test1));
    // CHECK_EQUAL(51.1504, lwgps.latitude);
    // CHECK_EQUAL(-114.030678, lwgps.longitude);
    CHECK_EQUAL(1097.36, lwgps.altitude);
}

TEST(lwgnps_test, test_GPGSA) {
    char test1[] = "$GPRMC,235316.000,A,4003.9040,N,10512.5792,W,0.09,144.75,141112,,*19\r\n"
                   "$GPGGA,235317.000,4003.9039,N,10512.5793,W,1,08,1.6,1577.9,M,-20.7,M,,0000*5F\r\n"
                   "$GPGSA,A,3,22,18,21,06,03,09,24,15,,,,,2.5,1.6,1.9*3E\r\n";

    lwgps_process(&lwgps, test1, strlen(test1));
    // CHECK_EQUAL(51.1504, lwgps.latitude);
    // CHECK_EQUAL(-114.030678, lwgps.longitude);
    CHECK_EQUAL(1.6, lwgps.dop_h);
    // CHECK_EQUAL(1.9, lwgps.dop_v);
    CHECK_EQUAL(2.5f, lwgps.dop_p);
}
