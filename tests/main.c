#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "strato3.h"

char line[] = "$;00:05:13;19:36:26;13.09.2016;Y;04;40 40.68437 N;005 30.70415 E;0.432;0.800;;91.2;11.625;12.625;37.56;1009.809;8.6;66679";

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
    strato3_data_t data;
    int ret = strato3_parse(line, &data);
    strato3_print(&data);
    return ret;
}
