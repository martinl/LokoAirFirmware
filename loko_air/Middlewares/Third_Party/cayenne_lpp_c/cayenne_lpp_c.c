/*
 * Copyright (C) 2018 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @brief       Cayenne LPP implementation
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 */

#include <assert.h>
#include <inttypes.h>
#include <string.h>

#include "cayenne_lpp_c.h"

#define TO_UINT8(byte)  ((uint8_t)(byte))
#define TO_INT16(hword) ((int16_t)(hword))
#define TO_INT32(word)  ((int32_t)(word))

void cayenne_lpp_reset(cayenne_lpp_t *lpp) {
    memset(lpp->buffer, 0, CAYENNE_LPP_MAX_BUFFER_SIZE);
    lpp->cursor = 0;
}

void cayenne_lpp_add_digital_input(cayenne_lpp_t *lpp, uint8_t channel, uint8_t value) {
    assert((lpp->cursor + CAYENNE_LPP_DIGITAL_INPUT_SIZE) < CAYENNE_LPP_MAX_BUFFER_SIZE);

    lpp->buffer[lpp->cursor++] = channel;
    lpp->buffer[lpp->cursor++] = CAYENNE_LPP_DIGITAL_INPUT;
    lpp->buffer[lpp->cursor++] = value;
}

void cayenne_lpp_add_digital_output(cayenne_lpp_t *lpp, uint8_t channel, uint8_t value) {
    assert((lpp->cursor + CAYENNE_LPP_DIGITAL_OUTPUT_SIZE) < CAYENNE_LPP_MAX_BUFFER_SIZE);

    lpp->buffer[lpp->cursor++] = channel;
    lpp->buffer[lpp->cursor++] = CAYENNE_LPP_DIGITAL_OUTPUT;
    lpp->buffer[lpp->cursor++] = value;
}

void cayenne_lpp_add_analog_input(cayenne_lpp_t *lpp, uint8_t channel, float value) {
    assert((lpp->cursor + CAYENNE_LPP_ANALOG_INPUT_SIZE) < CAYENNE_LPP_MAX_BUFFER_SIZE);

    int16_t val = TO_INT16(value * 100);
    lpp->buffer[lpp->cursor++] = channel;
    lpp->buffer[lpp->cursor++] = CAYENNE_LPP_ANALOG_INPUT;
    lpp->buffer[lpp->cursor++] = TO_UINT8(val >> 8);
    lpp->buffer[lpp->cursor++] = TO_UINT8(val);
}

void cayenne_lpp_add_analog_output(cayenne_lpp_t *lpp, uint8_t channel, float value) {
    assert((lpp->cursor + CAYENNE_LPP_ANALOG_OUTPUT_SIZE) < CAYENNE_LPP_MAX_BUFFER_SIZE);

    int16_t val = TO_INT16(value * 100);
    lpp->buffer[lpp->cursor++] = channel;
    lpp->buffer[lpp->cursor++] = CAYENNE_LPP_ANALOG_OUTPUT;
    lpp->buffer[lpp->cursor++] = TO_UINT8(val >> 8);
    lpp->buffer[lpp->cursor++] = TO_UINT8(val);
}

void cayenne_lpp_add_luminosity(cayenne_lpp_t *lpp, uint8_t channel, uint16_t lux) {
    assert((lpp->cursor + CAYENNE_LPP_LUMINOSITY_SIZE) < CAYENNE_LPP_MAX_BUFFER_SIZE);

    lpp->buffer[lpp->cursor++] = channel;
    lpp->buffer[lpp->cursor++] = CAYENNE_LPP_LUMINOSITY;
    lpp->buffer[lpp->cursor++] = TO_UINT8(lux >> 8);
    lpp->buffer[lpp->cursor++] = TO_UINT8(lux);
}

void cayenne_lpp_add_presence(cayenne_lpp_t *lpp, uint8_t channel, uint8_t value) {
    assert((lpp->cursor + CAYENNE_LPP_PRESENCE_SIZE) < CAYENNE_LPP_MAX_BUFFER_SIZE);

    lpp->buffer[lpp->cursor++] = channel;
    lpp->buffer[lpp->cursor++] = CAYENNE_LPP_PRESENCE;
    lpp->buffer[lpp->cursor++] = value;
}

void cayenne_lpp_add_temperature(cayenne_lpp_t *lpp, uint8_t channel, float celsius) {
    assert((lpp->cursor + CAYENNE_LPP_TEMPERATURE_SIZE) < CAYENNE_LPP_MAX_BUFFER_SIZE);

    int16_t val = TO_INT16(celsius * 10);
    lpp->buffer[lpp->cursor++] = channel;
    lpp->buffer[lpp->cursor++] = CAYENNE_LPP_TEMPERATURE;
    lpp->buffer[lpp->cursor++] = TO_UINT8(val >> 8);
    lpp->buffer[lpp->cursor++] = TO_UINT8(val);
}

void cayenne_lpp_add_relative_humidity(cayenne_lpp_t *lpp, uint8_t channel, float rh) {
    assert((lpp->cursor + CAYENNE_LPP_RELATIVE_HUMIDITY_SIZE) < CAYENNE_LPP_MAX_BUFFER_SIZE);

    uint8_t val = TO_UINT8(rh * 2);
    lpp->buffer[lpp->cursor++] = channel;
    lpp->buffer[lpp->cursor++] = CAYENNE_LPP_RELATIVE_HUMIDITY;
    lpp->buffer[lpp->cursor++] = val;
}

void cayenne_lpp_add_accelerometer(cayenne_lpp_t *lpp, uint8_t channel, float x, float y, float z) {
    assert((lpp->cursor + CAYENNE_LPP_ACCELEROMETER_SIZE) < CAYENNE_LPP_MAX_BUFFER_SIZE);

    int16_t vx = TO_INT16(x * 1000);
    int16_t vy = TO_INT16(y * 1000);
    int16_t vz = TO_INT16(z * 1000);

    lpp->buffer[lpp->cursor++] = channel;
    lpp->buffer[lpp->cursor++] = CAYENNE_LPP_ACCELEROMETER;
    lpp->buffer[lpp->cursor++] = TO_UINT8(vx >> 8);
    lpp->buffer[lpp->cursor++] = TO_UINT8(vx);
    lpp->buffer[lpp->cursor++] = TO_UINT8(vy >> 8);
    lpp->buffer[lpp->cursor++] = TO_UINT8(vy);
    lpp->buffer[lpp->cursor++] = TO_UINT8(vz >> 8);
    lpp->buffer[lpp->cursor++] = TO_UINT8(vz);
}

void cayenne_lpp_add_barometric_pressure(cayenne_lpp_t *lpp, uint8_t channel, float hpa) {
    assert((lpp->cursor + CAYENNE_LPP_BAROMETRIC_PRESSURE_SIZE) < CAYENNE_LPP_MAX_BUFFER_SIZE);

    int16_t val = TO_INT16(hpa * 10);
    lpp->buffer[lpp->cursor++] = channel;
    lpp->buffer[lpp->cursor++] = CAYENNE_LPP_BAROMETRIC_PRESSURE;
    lpp->buffer[lpp->cursor++] = TO_UINT8(val >> 8);
    lpp->buffer[lpp->cursor++] = TO_UINT8(val);
}

void cayenne_lpp_add_gyrometer(cayenne_lpp_t *lpp, uint8_t channel, float x, float y, float z) {
    assert((lpp->cursor + CAYENNE_LPP_GYROMETER_SIZE) < CAYENNE_LPP_MAX_BUFFER_SIZE);

    int16_t vx = TO_INT16(x * 100);
    int16_t vy = TO_INT16(y * 100);
    int16_t vz = TO_INT16(z * 100);

    lpp->buffer[lpp->cursor++] = TO_UINT8(channel);
    lpp->buffer[lpp->cursor++] = CAYENNE_LPP_GYROMETER;
    lpp->buffer[lpp->cursor++] = TO_UINT8(vx >> 8);
    lpp->buffer[lpp->cursor++] = TO_UINT8(vx);
    lpp->buffer[lpp->cursor++] = TO_UINT8(vy >> 8);
    lpp->buffer[lpp->cursor++] = TO_UINT8(vy);
    lpp->buffer[lpp->cursor++] = TO_UINT8(vz >> 8);
    lpp->buffer[lpp->cursor++] = TO_UINT8(vz);
}

void cayenne_lpp_add_gps(cayenne_lpp_t *lpp, uint8_t channel, float latitude, float longitude, float meters) {
    assert((lpp->cursor + CAYENNE_LPP_GPS_SIZE) < CAYENNE_LPP_MAX_BUFFER_SIZE);

    int32_t lat = TO_INT32(latitude * 10000);
    int32_t lon = TO_INT32(longitude * 10000);
    int32_t alt = TO_INT32(meters * 100);

    lpp->buffer[lpp->cursor++] = channel;
    lpp->buffer[lpp->cursor++] = CAYENNE_LPP_GPS;

    lpp->buffer[lpp->cursor++] = TO_UINT8(lat >> 16);
    lpp->buffer[lpp->cursor++] = TO_UINT8(lat >> 8);
    lpp->buffer[lpp->cursor++] = TO_UINT8(lat);
    lpp->buffer[lpp->cursor++] = TO_UINT8(lon >> 16);
    lpp->buffer[lpp->cursor++] = TO_UINT8(lon >> 8);
    lpp->buffer[lpp->cursor++] = TO_UINT8(lon);
    lpp->buffer[lpp->cursor++] = TO_UINT8(alt >> 16);
    lpp->buffer[lpp->cursor++] = TO_UINT8(alt >> 8);
    lpp->buffer[lpp->cursor++] = TO_UINT8(alt);
}