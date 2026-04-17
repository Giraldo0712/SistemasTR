#include <stdio.h>
// #include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_timer.h"


static QueueHandle_t led_queue = NULL;
#define LED_GPIO    GPIO_NUM_2
#define BUTTON_GPIO GPIO_NUM_0

static uint64_t last_time = 0; // Guarda el tiempo del último clic válido


typedef enum{
    LED_2_2,
    LED_2_1,
    LED_1_1,
    LED_05_05,
    LED_OFF,

}led_enum_state;


// Variable global para que el ISR sepa en qué estado estamos
static led_enum_state global_led_state = LED_2_2;

void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint64_t now = esp_timer_get_time(); //Obtenemos el tiempo actual en microsegundos

    if ((now - last_time) < 200000) { // Si han pasado menos de 200 ms desde el último clic, lo ignoramos
        return;
    }
    last_time = now; // Actualizamos el tiempo del último clic válido

    switch (global_led_state) {
        case LED_2_2:    global_led_state = LED_2_1; break;
        case LED_2_1:    global_led_state = LED_1_1; break;
        case LED_1_1:    global_led_state = LED_05_05; break;
        case LED_05_05:  global_led_state = LED_OFF; break;
        case LED_OFF:    global_led_state = LED_2_2; break;
    }

    // Enviamos el nuevo estado a la cola desde la interrupción
    xQueueSendFromISR(led_queue, &global_led_state, NULL);
}


void config_LED_and_Button (void){
    //zero-initialize the config structure.
    //LED
    gpio_config_t io_conf = {}; //Configuracion estructura de GPIO, se inicializa a 0
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE; //Desactiva las interrupciones, el LED no necesita interrupciones, solo se enciende o apaga
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT; //Configura el pin como salida, el LED es un dispositivo de salida, se enciende o apaga
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = 1ULL<<LED_GPIO; //Selecciona el pin del LED, se utiliza un desplazamiento de bits para configurar el pin específico
    //disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; //Desactiva la resistencia de pull-down, el LED no necesita una resistencia de pull-down, ya que se controla directamente con el pin
    //disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE; //Desactiva la resistencia de pull-up, el LED no necesita una resistencia de pull-up, ya que se controla directamente con el pin
    //configure GPIO with the given settings
    gpio_config(&io_conf); //Aplica la configuración al pin del LED, se llama a la función gpio_config para configurar el pin del LED con los parámetros establecidos en la estructura io_conf

    //BUTTON
    io_conf.intr_type = GPIO_INTR_NEGEDGE; //SE ACTIVA EN EL FLANCO DE BAJADA (de 1 a 0)
    io_conf.mode = GPIO_MODE_INPUT; //Configura el pin como entrada, el botón es un dispositivo de entrada, se presiona o no
    io_conf.pin_bit_mask = 1ULL<<BUTTON_GPIO; //Selecciona el pin del botón, se utiliza un desplazamiento de bits para configurar el pin específico
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; //Desactiva la resistencia de pull-down, el botón no necesita una resistencia de pull-down, ya que se controla directamente con el pin
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE; //Mantiene el pin en 1 hasta que presionas
    gpio_config(&io_conf); //Aplica la configuración al pin del botón, se llama a la función gpio_config para configurar el pin del botón con los parámetros establecidos en la estructura io_conf

    gpio_install_isr_service(0); // INSTALAR EL SERVICIO DE INTERRUPCIONES (Se hace una vez)
    gpio_isr_handler_add(BUTTON_GPIO, gpio_isr_handler, (void*) BUTTON_GPIO); // ASIGNAR EL MANEJADOR AL PIN DEL BOTÓN
}


void led_task(void *pvParameter){
    led_enum_state led_state_2 = LED_2_2;

    while(1){
        xQueueReceive(led_queue, &led_state_2, 0); // Recibe el estado del LED desde la cola, se bloquea hasta que haya un nuevo estado disponible en la cola, el tercer parámetro es el tiempo de espera, en este caso 0 significa que se bloqueará indefinidamente hasta recibir un nuevo estado
        if ( led_state_2 == LED_2_2){
            gpio_set_level(LED_GPIO,1);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            gpio_set_level( LED_GPIO, 0);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
        else if ( led_state_2 == LED_2_1){
            gpio_set_level(LED_GPIO,1);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            gpio_set_level( LED_GPIO, 0);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        else if ( led_state_2 == LED_1_1){
            gpio_set_level(LED_GPIO,1);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            gpio_set_level( LED_GPIO, 0);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        else if ( led_state_2 == LED_05_05){
            gpio_set_level(LED_GPIO,1);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            gpio_set_level( LED_GPIO, 0);
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
        else if ( led_state_2 == LED_OFF){
            gpio_set_level(LED_GPIO,0);
        }   
    }
}


void app_main(void)
{
    // 1. Primero crear la cola para que esté lista para recibir mensajes
    led_queue = xQueueCreate(10, sizeof(led_enum_state));

    // 2. Luego configurar el hardware e interrupciones
    config_LED_and_Button();

    // 3. Crear solo la tarea del LED (la del botón ya no es necesaria)
    xTaskCreate(&led_task, "led_task", 2048, NULL, 5, NULL);
}