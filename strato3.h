#ifndef STRATO3_H
#define STRATO3_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#ifndef STRATO3_DEBUG
#define STRATO3_DEBUG (0)
#endif
#define debug_print(...) \
            do { if (STRATO3_DEBUG) fprintf(stderr, __VA_ARGS__); } while (0)


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
 * @brief Convert a scaled integer to a float
 *
 * @param[in]   f   the scaled integer
 *
 * @returns     value as float or NaN on error
 */
static inline float strato3_tofloat(const strato3_float_t *f)
{
    if (f->scale == 0) {
        return NAN;
    }

    return ((float) f->value / (float) f->scale);
}

/**
 * @brief Convert a scaled integer to a float
 *
 * @param[in]   f   the scaled coordinate
 *
 * @returns     coordinate as float or NaN on error
 */
 static inline float strato3_tocoord(const strato3_coord_t *c)
 {
     int sign = _dir2sign(c);

     if (sign == 0) {
         return NAN;
     }

     return ((sign * (float) c->deg) + (strato3_tofloat(&c->min) / 60.0));
 }

/**
 * @brief Parse strato3 sensor data from a given string
 *
 * @param[in]   line    the string to be parsed
 * @param[out]  data    the parsed data struct
 *
 * @returns     0       on success
 * @returns     >0      on error
 */
int strato3_parse(const char *line, strato3_data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* STRATO3_H */
