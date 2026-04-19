#include <stdio.h>
#include "common.h"
#include "pico/stdlib.h"

int main()
{
    stdio_init_all();

    while (true) {
        printf("Hello, world! %d\n", (int)common_add(1, 1));
        sleep_ms(1000);
    }
}
