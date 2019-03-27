#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "strato3.h"

#ifndef DEBUG
#define DEBUG (0)
#endif
#define debug_print(...) \
            do { if (DEBUG) fprintf(stderr, __VA_ARGS__); } while (0)

char _check_and_find_delim(const char *line)
{
    debug_print("%s\n",__func__);
    size_t len = strlen(line);

    /* check for minimal length, i.e., no values only delims */
    if (len < STRATO3_FIELD_STATE) {
        return -1;
    }

    strato3_field_t field = STRATO3_FIELD_BEGIN;
    char delim = -1;
    /* parse line, find delim char and progess through all fields */
    for (size_t pos = 0; pos < len; ++pos) {
        if (field == STRATO3_FIELD_BEGIN) {
            if (line[pos] == '$') {
                delim = line[pos + 1];
            }
        }
        if ((delim > 0) && (line[pos] == delim)) {
            ++field;
        }
    }
    /* check if all fields were traversed */
    if (field != STRATO3_FIELD_STATE) {
        debug_print("ERROR stopped at %d!\n", (int)field);
        return -1;
    }
    /* check if delim is a printable character */
    if (!isprint(delim)) {
        debug_print("ERROR no valid delim!\n");
        return -1;
    }
    return delim;
}

static inline bool _isfield(char c, char delim)
{
    return (isprint(c) && (c != delim));
}

static inline const char *_next_field(const char *line, char delim)
{
    for (size_t pos = 0; pos < strlen(line); ++pos) {
        if (line[pos] == delim) {
            return &line[pos + 1];
        }
    }
    /* only reached if no next field is found */
    return NULL;
}

static int _parse_date(const char *fstr, char delim, strato3_date_t *d)
{
    debug_print("%s\n",__func__);
    if (!fstr || !isprint(delim)) {
        return -1;
    }
    char *next;
    d->day = strtol(fstr, &next, 10);
    d->month = strtol(++next, &next, 10);
    d->year = strtol(++next, NULL, 10);
    return 0;
}

static int _parse_time(const char *fstr, char delim, strato3_time_t *t)
{
    debug_print("%s\n",__func__);
    if (!fstr || !isprint(delim)) {
        return -1;
    }
    char *next;
    t->hour = strtol(fstr, &next, 10);
    t->min = strtol(++next, &next, 10);
    t->sec = strtol(++next, NULL, 10);
    return 0;
}

static int _parse_float(const char *fstr, char delim, strato3_float_t *f)
{
    debug_print("%s\n",__func__);
    if (!fstr || !isprint(delim)) {
        return -1;
    }
    /* init data */
    f->value = 0;
    f->scale = 0;

    int sign = 0;
    int32_t value = -1;
    int32_t scale = 0;
    while(_isfield(*fstr, delim)) {
        if ((*fstr == '+') && (sign == 0) && (value == -1)) {
            sign = 1;
        } else if ((*fstr == '-') && (sign == 0) && (value == -1)) {
            sign = -1;
        } else if (isdigit(*fstr)) {
            int digit = *fstr - '0';
            if (value == -1) {
                value = 0;
            }
            value = (10 * value) + digit;
            if (scale) {
                scale *= 10;
            }
        } else if ((*fstr == '.') && (scale == 0)) {
            scale = 1;
        } else if (*fstr == ' ') {
            /* ignore leading spaces, stop otherwise */
            if ((sign != 0) || (value != -1)) {
                break;
            }
        } else {
            /* invalid string */
            debug_print("%s: invalid string!\n", __func__);
            return -1;
        }
        fstr++;
    }
    if (sign) {
        value *= sign;
    }
    f->value = value;
    f->scale = scale;
    debug_print("%s: (%d, %d)\n", __func__, f->value, f->scale);
    return 0;
}

static int _parse_coord(const char *cstr, char delim, strato3_coord_t *c)
{
    debug_print("%s\n",__func__);
    if (!cstr || !isprint(delim)) {
        return -1;
    }
    /* init data */
    c->deg = 0;
    c->min.value = 0;
    c->min.scale = 0;
    c->dir = 0;

    char *next;
    /* data format: DEG MIN.DMIN DIR, e.g. 40 40.68437 N */
    c->deg = strtol(cstr, &next, 10);
    if (_parse_float(next, delim, &c->min) != 0) {
        return -1;
    }

    while (_isfield(*next, delim)) {
        if ((*next == 'N') || (*next == 'S') || (*next == 'E') || (*next == 'W')) {
            c->dir = *next;
        }
        next++;
    }
    if (c->dir == 0) {
        return -1;
    }
    debug_print("%s: (%u, %f, %c)\n", __func__, c->deg, strato3_tofloat(&c->min), c->dir);
    return 0;
}

int strato3_parse(const char *line, strato3_data_t *data)
{
    debug_print("%s\n",__func__);
    char delim = _check_and_find_delim(line);
    if (delim < 0) {
        return -1;
    }
    int error = 0;
    size_t len = strlen(line);
    const char *pos = line;
    strato3_field_t field = STRATO3_FIELD_BEGIN;
    while ((field < STRATO3_FIELD_END) && (pos != NULL)) {
        switch ((int) field) {
            case STRATO3_FIELD_VALID:
                data->valid = (*pos == 'Y');
                break;
            case STRATO3_FIELD_SATELLITES:
                data->satellites = strtol(pos, NULL, 10);
                break;
            case STRATO3_FIELD_UPTIME:
                if (_parse_time(pos, delim, &data->uptime) != 0) {
                    debug_print("ERROR parsing STRATO3_FIELD_UPTIME\n");
                    error++;
                }
                break;
            case STRATO3_FIELD_TIME:
                if (_parse_time(pos, delim, &data->time) != 0) {
                    debug_print("ERROR parsing STRATO3_FIELD_TIME\n");
                    error++;
                }
                break;
            case STRATO3_FIELD_DATE:
                if (_parse_date(pos, delim, &data->date) != 0) {
                    debug_print("ERROR parsing STRATO3_FIELD_TIME\n");
                    error++;
                }
                break;
            case STRATO3_FIELD_GPSLAT:
                if (_parse_coord(pos, delim, &data->latitude) != 0) {
                    debug_print("ERROR parsing STRATO3_FIELD_GPSLAT\n");
                    error++;
                }
                break;
            case STRATO3_FIELD_GPSLON:
                if (_parse_coord(pos, delim, &data->longitude) != 0) {
                    debug_print("ERROR parsing STRATO3_FIELD_GPSLON\n");
                    error++;
                }
                break;
            case STRATO3_FIELD_SPEED_KNT:
                if (_parse_float(pos, delim, &data->speed_knt) != 0) {
                    debug_print("ERROR parsing STRATO3_FIELD_SPEED_KNT\n");
                    error++;
                }
                break;
            case STRATO3_FIELD_SPEED_KPH:
                if (_parse_float(pos, delim, &data->speed_kph) != 0) {
                    debug_print("ERROR parsing STRATO3_FIELD_SPEED_KPH\n");
                    error++;
                }
                break;
            case STRATO3_FIELD_COURSE:
                if (_parse_float(pos, delim, &data->course) != 0) {
                    debug_print("ERROR parsing STRATO3_FIELD_COURSE\n");
                    error++;
                }
                break;
            case STRATO3_FIELD_GPSALT:
                if (_parse_float(pos, delim, &data->altitude) != 0) {
                    debug_print("ERROR parsing STRATO3_FIELD_GPSALT\n");
                    error++;
                }
                break;
            case STRATO3_FIELD_TEMPERATURE_BOARD:
                if (_parse_float(pos, delim, &data->temperature_board) != 0) {
                    debug_print("ERROR parsing STRATO3_FIELD_TEMPERATURE_BOARD\n");
                    error++;
                }
                break;
            case STRATO3_FIELD_TEMPERATURE:
                if (_parse_float(pos, delim, &data->temperature) != 0) {
                    debug_print("ERROR parsing STRATO3_FIELD_TEMPERATURE\n");
                    error++;
                }
                break;
            case STRATO3_FIELD_HUMIDITY:
                if (_parse_float(pos, delim, &data->humidity) != 0) {
                    debug_print("ERROR parsing STRATO3_FIELD_HUMIDITY\n");
                    error++;
                }
                break;
            case STRATO3_FIELD_PRESSURE:
                if (_parse_float(pos, delim, &data->pressure) != 0) {
                    debug_print("ERROR parsing STRATO3_FIELD_PRESSURE\n");
                    error++;
                }
                break;
            case STRATO3_FIELD_VOLTAGE:
                if (_parse_float(pos, delim, &data->voltage) != 0) {
                    debug_print("ERROR parsing STRATO3_FIELD_PRESSURE\n");
                    error++;
                }
                break;
            case STRATO3_FIELD_STATE:
                data->state = strtol(pos, NULL, 10);
                break;
            default:
                debug_print("FIELD %d\n", field);
        }
        pos = _next_field(pos, delim);
        field++;
    }
    return error;
}

void strato3_print(const strato3_data_t *data)
{
    printf("strato3_data = {\n");
    printf("  uptime:           %02u:%02u:%02u\n", data->uptime.hour, data->uptime.min, data->uptime.sec);
    printf("  time:             %02u:%02u:%02u\n", data->time.hour, data->time.min, data->time.sec);
    printf("  date:             %u-%02u-%02u\n", data->date.year, data->date.month, data->date.day);
    printf("  valid:            %c\n", data->valid ? 'Y' : 'N');
    printf("  satellites:       %u\n", data->satellites);
    printf("  latitude:         %.7f\n", strato3_tocoord(&data->latitude));
    printf("  longitude:        %.7f\n", strato3_tocoord(&data->longitude));
    printf("  speed (knt):      %.4f\n", strato3_tofloat(&data->speed_knt));
    printf("  speed (kph):      %.1f\n", strato3_tofloat(&data->speed_kph));
    printf("  course:           %.1f\n", strato3_tofloat(&data->course));
    printf("  altitude:         %.1f\n", strato3_tofloat(&data->altitude));
    printf("  temperature(in):  %.1f\n", strato3_tofloat(&data->temperature_board));
    printf("  temperature(ex):  %.1f\n", strato3_tofloat(&data->temperature));
    printf("  humidity:         %.1f\n", strato3_tofloat(&data->humidity));
    printf("  pressure:         %.3f\n", strato3_tofloat(&data->pressure));
    printf("  voltage:          %.1f\n", strato3_tofloat(&data->voltage));
    printf("  state:            %d\n", data->state);
    printf("}\n");
}

int main(void) {
    char line[] = "$;00:05:13;19:36:26;13.09.2016;Y;04;40 40.68437 N;005 30.70415 E;0.432;0.800;;91.2;11.625;12.625;37.56;1009.809;8.6;66679";
    strato3_data_t data;
    int ret = strato3_parse(line, &data);
    strato3_print(&data);
    return ret;
}
