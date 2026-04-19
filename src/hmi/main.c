#include <stdio.h>

#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"

#include "common.h"

void counter_task(void *pvParameters) {
    uint8_t counter = 0;

    while (true) {
        printf("Counter: %d\n", counter++);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void main_task(void *pvParameters) {
    xTaskCreate(counter_task, "counter_thread", configMINIMAL_STACK_SIZE, NULL,
                tskIDLE_PRIORITY + 1UL, NULL);

    while (true) {
        printf("Hello, world! %d\n", (int)common_add(1, 1));
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void vLaunch() {
    TaskHandle_t task;
    xTaskCreate(main_task, "main_thread", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1UL,
                &task);

#if configUSE_CORE_AFFINITY && (configNUMBER_OF_CORES > 1)
    vTaskCoreAffinitySet(task, 1UL << portGET_CORE_ID());
#endif

    vTaskStartScheduler();
}

inline const char *get_rtos_name() {
#if (configNUMBER_OF_CORES > 1)
    return "FreeRTOS SMP";
#else
    return "FreeRTOS";
#endif
}

int main() {
    stdio_init_all();

#if (configNUMBER_OF_CORES > 1)
    printf("Starting %s on both cores\n", get_rtos_name());
    vLaunch();
#else
    printf("Starting %s on core 0\n", get_rtos_name());
    vLaunch();
#endif

    return 0;
}