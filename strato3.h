#ifndef STRATO3_H
#define STRATO3_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

typedef enum stato3_field {
    STRATO3_FIELD_BEGIN,
    STRATO3_FIELD_UPTIME,
    STRATO3_FIELD_TIME,
    STRATO3_FIELD_DATE,
    STRATO3_FIELD_VALID,
    STRATO3_FIELD_SATELLITES,
    STRATO3_FIELD_GPSLAT,
    STRATO3_FIELD_GPSLON,
    STRATO3_FIELD_SPEED_KNT,
    STRATO3_FIELD_SPEED_KPH,
    STRATO3_FIELD_COURSE,
    STRATO3_FIELD_GPSALT,
    STRATO3_FIELD_TEMPERATURE_BOARD,
    STRATO3_FIELD_TEMPERATURE,
    STRATO3_FIELD_HUMIDITY,
    STRATO3_FIELD_PRESSURE,
    STRATO3_FIELD_VOLTAGE,
    STRATO3_FIELD_STATE,
    STRATO3_FIELD_END
} strato3_field_t;

typedef struct stato3_time {
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
} strato3_time_t;

typedef struct strato3_date {
    uint8_t day;
    uint8_t month;
    uint16_t year;
} strato3_date_t;

typedef struct strato3_float {
    int32_t value;
    int32_t scale;
} strato3_float_t;

typedef struct strato3_coord {
    uint8_t deg;
    strato3_float_t min;
    char dir;
} strato3_coord_t;

/* $;00:05:13;19:36:26;13.09.2016;Y;04;40 40.68437 N;005 30.70415 E;0.432;0.800;;91.2;11.625;12.625;37.56;1009.809;8.6;66679 */
typedef struct strato3_data {
    strato3_time_t uptime;
    strato3_time_t time;
    strato3_date_t date;
    bool valid;
    uint8_t satellites;
    strato3_coord_t latitude;
    strato3_coord_t longitude;
    strato3_float_t speed_knt;
    strato3_float_t speed_kph;
    strato3_float_t course;
    strato3_float_t altitude;
    strato3_float_t temperature_board;
    strato3_float_t temperature;
    strato3_float_t humidity;
    strato3_float_t pressure;
    strato3_float_t voltage;
    int32_t state;
} strato3_data_t;

static inline int _dir2sign(const strato3_coord_t *c)
{
    if ((c->dir == 'N') || (c->dir == 'E')) {
        return 1;
    }
    if ((c->dir == 'S') || (c->dir == 'W')) {
        return -1;
    }
    return 0;
}
/**
 * Convert a fixed-point value to a floating-point value.
 * Returns NaN for "unknown" values.
 */
static inline float strato3_tofloat(const strato3_float_t *f)
{
    if (f->scale == 0)
        return NAN;
    return (float) f->value / (float) f->scale;
}

/**
 * Convert a raw coordinate to a floating point DD.DDD... value.
 * Returns NaN for "unknown" values.
 */
static inline float strato3_tocoord(const strato3_coord_t *c)
{
    int sign = _dir2sign(c);
    if (sign == 0) {
        return NAN;
    }
    return (sign * (float) c->deg + (strato3_tofloat(&c->min) / 60.0));
}

int strato3_parse(const char *line, strato3_data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* STRATO3_H */
