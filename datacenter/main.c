/*
 * ================================================================
 * Edoardo Marinelli (matricola: 194845)
 * Real-Time Data Center Security Monitor
 * ELABORATO REAL-TIME EMBEDDED SYSTEMS
 * 
 * FREERTOS 
 * Hardware: Raspberry Pi Pico W, DHT11, PIR, Pulsante, Led RGB
 * Comunicazione: micro-ROS
 * ================================================================
 */

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <std_msgs/msg/string.h> 

#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rmw_microros/rmw_microros.h>

#include "pico/stdlib.h"

#include "pico_uart_transports.h"
#include "debug_uart.h"

#include <hardware/gpio.h>
#include <dht.h>

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){uart_printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc);vTaskDelete(NULL);}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){uart_printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

#define DEBUGx 1

#if defined(DEBUGx) && (DEBUGx == 1)
    #define DEBUGPRINT uart_printf
#else
    #define DEBUGPRINT(...) do {} while (0)
#endif

// Configurazione pins
#define PIR_PIN 16              // Sensore movimento PIR
#define DHT_PIN 17              // Sensore temperatura/umidità DHT11
#define TECH_BUTTON_PIN 18      // Pulsante ingresso/uscita tecnico
#define TECH_LED_PIN 19         // LED indicatore presenza tecnico

#define TOLERANCE_PERIOD 1000   // Periodo tolleranza movimento post-uscita tecnico

static const dht_model_t DHT_MODEL = DHT11;

// Struttura dati sensori datacenter
typedef struct {
    float temperature;
    bool temp_valid;
    float humidity;
    bool humidity_valid;
    bool motion;
    bool motion_valid;
    bool tech_inside;
    bool tech_status_changed;
    bool pir_enabled;
    bool motion_attributed_to_tech;
} datacenter_data_t;

// Struttura stato tecnico 
typedef struct {
    bool technician_inside;
    bool pir_monitoring_enabled;
    uint32_t technician_entry_time;
    uint32_t technician_exit_time;
    uint32_t technician_entry_counter;
    uint32_t tolerance_period_end_time;
} technitian_state_t;

static technitian_state_t technitian_state = {
    .technician_inside = false,
    .pir_monitoring_enabled = true,
    .technician_entry_time = 0,
    .technician_exit_time = 0,
    .technician_entry_counter = 0,
    .tolerance_period_end_time = 0
};

// Creazione code FreeRTOS
QueueHandle_t datacenterQueue;
QueueHandle_t technitianStateQueue;

static SemaphoreHandle_t state_mutex = NULL;

// Publisher ROS
rcl_publisher_t datacenter_publisher;
rcl_publisher_t technitian_state_publisher;

// Buffer JSON per messaggi ROS
static char datacenter_json_buffer[512];
static char technitian_json_buffer[512];

// Conversione timestamp millisecondi --> formato HH:MM:SS.mmm
void format_timestamp(uint32_t timestamp_ms, char* buffer, size_t buffer_size) {
    if (timestamp_ms == 0) {
        snprintf(buffer, buffer_size, "null");
        return;
    }
    
    // Converti millisecondi in secondi
    uint32_t total_seconds = timestamp_ms / 1000;
    uint32_t hours = total_seconds / 3600;
    uint32_t minutes = (total_seconds % 3600) / 60;
    uint32_t seconds = total_seconds % 60;
    uint32_t milliseconds = timestamp_ms % 1000;
    
    snprintf(buffer, buffer_size, "\"%02lu:%02lu:%02lu.%03lu\"", hours, minutes, seconds, milliseconds);
}

// Accesso allo stato del tecnico
bool is_technician_inside(void) {
    bool result = false;
    if (xSemaphoreTake(state_mutex, portMAX_DELAY) == pdTRUE) {
        result = technitian_state.technician_inside;
        xSemaphoreGive(state_mutex);
    } else {
        DEBUGPRINT("[ERROR] Impossibile acquisire mutex per stato tecnico\n\r");
    }
    return result;
}

// Accesso allo stato del PIR
bool is_pir_enabled(void) {
    return technitian_state.pir_monitoring_enabled;
}

// Verifica se ci troviamo nel periodo di tolleranza post-uscita
bool is_in_tolerance_period(void) {
    bool result = false;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    // Verifica se il tempo corrente è inferiore al tempo di fine tolleranza
    // SI -> il movimento è attribuibile al tecnico appena uscito
    // NO -> il movimento è un intrusione
    if (xSemaphoreTake(state_mutex, portMAX_DELAY) == pdTRUE) {
        result = (current_time < technitian_state.tolerance_period_end_time);
        xSemaphoreGive(state_mutex);
    } else {
        DEBUGPRINT("[ERROR] Impossibile acquisire mutex per periodo tolleranza\n\r");
    }
    return result;
}

// Controllo LED indicatore presenza tecnico
void set_technician_led(bool tech_inside) {
    gpio_put(TECH_LED_PIN, tech_inside);
}

// Gestione cambio stato tecnico (ingresso/uscita)
void set_technician_state(bool inside) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    BaseType_t xStatus;

    if (xSemaphoreTake(state_mutex, portMAX_DELAY) == pdTRUE) {
        technitian_state.technician_inside = inside;
        technitian_state.pir_monitoring_enabled = !inside;  // PIR disabilitato quando il tecnico è presente e viceversa
        
        if (inside) {
            // TECNICO ENTRA
            technitian_state.technician_entry_time = current_time; // Aggiornamento tempo entrata tecnico
            technitian_state.technician_exit_time = 0;       // Reset tempo uscita tecnico
            technitian_state.tolerance_period_end_time = 0;  // Reset periodo di tolleranza
            technitian_state.technician_entry_counter++;     // Contatore ingressi tecnico
            DEBUGPRINT("[STATE] Tecnico ENTRATO #%lu - PIR BLOCCATO - Exit time resettato\n\r", technitian_state.technician_entry_counter);
            
            set_technician_led(true);
            
        } else {
            // TECNICO ESCE
            technitian_state.technician_exit_time = current_time; // Aggiornamento tempo uscita tecnico
            technitian_state.tolerance_period_end_time = current_time + TOLERANCE_PERIOD; // Aggiornamento fine tolleranza tecnico
            DEBUGPRINT("[STATE] Tecnico USCITO (Ingresso #%lu) - PIR RIATTIVATO - Periodo tolleranza: %lu ms\n\r", technitian_state.technician_entry_counter, TOLERANCE_PERIOD);
            
            set_technician_led(false);
        }
        
        technitian_state_t state_msg = technitian_state;
        
        xSemaphoreGive(state_mutex);
        
        // Invia aggiornamento stato alla coda publisher
        xStatus = xQueueSendToBack(technitianStateQueue, &state_msg, 0);
        if( xStatus != pdPASS ) {
            DEBUGPRINT("[ERROR] Impossibile inviare stato alla coda technitian state\n\r");
        }
    } else {
        DEBUGPRINT("[ERROR] CRITICO: Impossibile acquisire mutex per cambio stato tecnico!\n\r");
    }

}

// Task gestione pulsante tecnico (ingresso/uscita)
static void TechnicianButtonTask(void *pvParameters) {
    DEBUGPRINT("[TECHNICIAN] Task avviato - Gestione Ingresso/Uscita Tecnico\n\r");
    
    bool button_pressed = false;
    bool last_button_pressed = false;
    BaseType_t xStatus;
    
    for( ;; ) {
        button_pressed = gpio_get(TECH_BUTTON_PIN);
        
        // Rileva pressione pulsante
        if (button_pressed && !last_button_pressed) { // Senza last_button_pressed ci sarebbero problemi in caso di pulsante tenuto premuto e memoria pressione precedente

            bool new_state = !is_technician_inside(); // Se il tecnico è dentro, il nuovo stato sarà "fuori" e viceversa
            
            DEBUGPRINT("[TECHNICIAN] Pressione pulsante rilevata - Cambio stato: %s\n\r", new_state ? "ENTRATA" : "USCITA");
            
            set_technician_state(new_state);
            
            // Invia evento alla coda datacenter
            datacenter_data_t tech_data = {0};
            tech_data.tech_inside = new_state;
            tech_data.tech_status_changed = true;
            tech_data.pir_enabled = is_pir_enabled();
            
            xStatus = xQueueSendToBack(datacenterQueue, &tech_data, 0);
            if( xStatus != pdPASS ) {
                DEBUGPRINT("[ERROR] Impossibile inviare dati tecnico alla coda\n\r");
            }
        }
        
        last_button_pressed = button_pressed;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Task lettura sensori ambientali (DHT11)
static void EnvironmentalRead(void *pvParameters) {
    DEBUGPRINT("[ENVIRONMENTAL] Task avviato - Monitoraggio Continuo Data Center\n\r");
    dht_t dht;
    BaseType_t xStatus;

    dht_init(&dht, DHT_MODEL, pio0, DHT_PIN, true);
    
    for( ;; ){
        datacenter_data_t env_data = {0};
        
        // Lettura temperatura e umidità
        dht_start_measurement(&dht);
        dht_result_t result = dht_finish_measurement_blocking(&dht, &env_data.humidity, &env_data.temperature);
        
        if (result == DHT_RESULT_OK) {
            DEBUGPRINT("[ENVIRONMENTAL] T: %.1f°C, H: %.1f%%. \n\r", env_data.temperature, env_data.humidity);
            env_data.temp_valid = true;
            env_data.humidity_valid = true;
        } else {
            DEBUGPRINT("[ENVIRONMENTAL] Errore lettura DHT (bad timeout or checksum error).\n\r");
            env_data.temp_valid = false;
            env_data.humidity_valid = false;
        }
        
        // Invia dati ambientali alla coda publisher
        xStatus = xQueueSendToBack(datacenterQueue, &env_data, 0);
        if( xStatus != pdPASS ){
            DEBUGPRINT("[ERROR] Impossibile inviare dati ambientali alla coda.\n\r");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));  // Lettura ogni 2 secondi
    }
}

// Task lettura sensore movimento PIR
static void PIRRead(void *pvParameters) {
    BaseType_t xStatus;
    bool current_state = false;
    bool previous_state = false;

    DEBUGPRINT("[MOTION] Task avviato - Monitoraggio con controllo tecnico.\n\r");
    
    for( ;; ){
        // Se il tecnico è presente, disabilita il monitoraggio PIR
        if (is_technician_inside()) {
            DEBUGPRINT("[MOTION] PIR disabilitato - Tecnico presente\n\r");
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        
        current_state = gpio_get(PIR_PIN);
        
        // Rileva cambiamento di stato movimento
        if (current_state != previous_state) {
            
            DEBUGPRINT("[MOTION] Rilevato %s\n\r", current_state ? "MOVIMENTO" : "FINE MOVIMENTO");
            
            datacenter_data_t motion_data = {0};
            motion_data.motion = current_state;
            motion_data.motion_valid = true;
            motion_data.pir_enabled = true;
            
            // Controlla se il movimento è nel periodo di tolleranza post-uscita --> rimasuglio del tecnico
            if (is_in_tolerance_period() && current_state) {
                motion_data.motion_attributed_to_tech = true;
                DEBUGPRINT("[MOTION] Movimento nel periodo di tolleranza - Attribuito al tecnico uscito\n\r");
            } else {
                motion_data.motion_attributed_to_tech = false;
            }
            
            // Invia evento movimento alla coda publisher
            xStatus = xQueueSendToBack(datacenterQueue, &motion_data, 0);
            if( xStatus != pdPASS ){
                DEBUGPRINT("[ERROR] Impossibile inviare dati movimento alla coda\n\r");
            }
        }
        
        previous_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Task publisher dati datacenter su topic ROS
void datacenter_publisher_task(void * arg) {
    datacenter_data_t lReceivedData;
    
    // Stato ultimo messaggio pubblicato
    static float last_temp = 0.0;
    static float last_humidity = 0.0;
    static bool last_motion = false;
    static bool last_tech_inside = false;
    static bool last_pir_enabled = true;
    static bool last_motion_attributed_to_tech = false;
    
    std_msgs__msg__String msg;
    msg.data.data = datacenter_json_buffer;
    msg.data.capacity = sizeof(datacenter_json_buffer);

    DEBUGPRINT("[PUBLISHER] Task datacenter avviato\n\r");

    while(1) {        
        if(xQueueReceive(datacenterQueue, &lReceivedData, portMAX_DELAY)) {
            // Aggiorna stato solo se il dato è valido
            if (lReceivedData.temp_valid) {
                last_temp = lReceivedData.temperature;
            }
            if (lReceivedData.humidity_valid) {
                last_humidity = lReceivedData.humidity;
            }
            if (lReceivedData.motion_valid) {
                last_motion = lReceivedData.motion;
                last_motion_attributed_to_tech = lReceivedData.motion_attributed_to_tech;
            }
            if (lReceivedData.tech_status_changed) {
                last_tech_inside = lReceivedData.tech_inside;
                last_pir_enabled = lReceivedData.pir_enabled;
            }

            // Messaggio JSON con ultimo stato completo
            int len = snprintf(datacenter_json_buffer, sizeof(datacenter_json_buffer),
                "{"
                "\"temp\":%.1f,"
                "\"hum\":%.1f,"
                "\"motion\":%s,"
                "\"motion_attributed_to_tech\":%s,"
                "\"tech_inside\":%s,"
                "\"pir_enabled\":%s"
                "}",
                last_temp,
                last_humidity,
                last_motion ? "true" : "false",
                last_motion_attributed_to_tech ? "true" : "false",
                last_tech_inside ? "true" : "false",
                last_pir_enabled ? "true" : "false"
            );
                        
            msg.data.size = len;
            RCSOFTCHECK(rcl_publish(&datacenter_publisher, &msg, NULL));
            
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

// Task publisher stato tecnico su topic ROS
void technitian_state_publisher_task(void * arg) {
    technitian_state_t lReceivedState;
        
    std_msgs__msg__String state_msg;
    state_msg.data.data = technitian_json_buffer;
    state_msg.data.capacity = sizeof(technitian_json_buffer);

    DEBUGPRINT("[PUBLISHER] Task stato tecnico avviato\n\r");

    while(1) {        
        if(xQueueReceive(technitianStateQueue, &lReceivedState, portMAX_DELAY)) {
            // Buffer per i timestamp formattati
            char entry_time_str[32];
            char exit_time_str[32];
            char tolerance_end_str[32];
            
            // Creazione timestamp formattati
            format_timestamp(lReceivedState.technician_entry_time, entry_time_str, sizeof(entry_time_str));
            format_timestamp(lReceivedState.technician_exit_time, exit_time_str, sizeof(exit_time_str));
            format_timestamp(lReceivedState.tolerance_period_end_time, tolerance_end_str, sizeof(tolerance_end_str));
            
            // Messaggio JSON stato tecnico
            int len = snprintf(technitian_json_buffer, sizeof(technitian_json_buffer),
                "{"
                "\"entry_time\":%s,"
                "\"exit_time\":%s,"
                "\"entry_counter\":%lu,"
                "\"tolerance_period_end\":%s"
                "}",
                entry_time_str,
                exit_time_str,
                lReceivedState.technician_entry_counter,
                tolerance_end_str
            ); 
            
            state_msg.data.size = len;
            RCSOFTCHECK(rcl_publish(&technitian_state_publisher, &state_msg, NULL));
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// Task principale micro-ROS
void micro_ros_task(void * arg) {
    rcl_allocator_t allocator = rcl_get_default_allocator();
    rclc_support_t support;

    // Inizializzazione micro-ROS
    rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
    RCCHECK(rcl_init_options_init(&init_options, allocator));
    RCCHECK(rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator));

    rcl_node_t node;
    RCCHECK(rclc_node_init_default(&node, "datacenter_monitor", "", &support));
    
    // Creazione publisher per dati datacenter
    RCCHECK(rclc_publisher_init_default(
        &datacenter_publisher,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
        "/datacenter_status"));

    // Creazione publisher per stato tecnico
    RCCHECK(rclc_publisher_init_default(
        &technitian_state_publisher,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
        "/datacenter_technitian"));

    // Creazione tasks publishers
    xTaskCreate(datacenter_publisher_task,
        "datacenter_pub",
        1024, 
        NULL,
        2,
        NULL);

    xTaskCreate(technitian_state_publisher_task,
        "technitian_pub",
        1024, 
        NULL,
        1,
        NULL);
    
    xTaskCreate( PIRRead, "PIR_Task", 512, NULL, 3, NULL );
    xTaskCreate( EnvironmentalRead, "ENV_Task", 512, NULL, 1, NULL );    
    xTaskCreate( TechnicianButtonTask, "TECH_Task", 512, NULL, 2, NULL );
     
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    RCCHECK(rcl_publisher_fini(&datacenter_publisher, &node));
    RCCHECK(rcl_publisher_fini(&technitian_state_publisher, &node));
    RCCHECK(rcl_node_fini(&node));

    vTaskDelete(NULL);
}

int main(void) {

    initialize_debug_uart();
    sleep_ms(3000);
    
    DEBUGPRINT("=== DATACENTER SECURITY MONITOR ===\n\r");

    // Configurazione pin GPIO
    gpio_init(DHT_PIN);
    gpio_set_dir(DHT_PIN, GPIO_IN);
    
    gpio_init(PIR_PIN);
    gpio_set_dir(PIR_PIN, GPIO_IN);

    gpio_init(TECH_BUTTON_PIN);
    gpio_set_dir(TECH_BUTTON_PIN, GPIO_IN);
    
    gpio_init(TECH_LED_PIN);
    gpio_set_dir(TECH_LED_PIN, GPIO_OUT);
    gpio_put(TECH_LED_PIN, false);  // LED spento all'avvio

    state_mutex = xSemaphoreCreateMutex();

    rmw_uros_set_custom_transport(
		true,
		NULL,
		pico_serial_transport_open,
		pico_serial_transport_close,
		pico_serial_transport_write,
		pico_serial_transport_read
	);

    // Creazione code FreeRTOS
    datacenterQueue = xQueueCreate(10, sizeof(datacenter_data_t));
    technitianStateQueue = xQueueCreate(10, sizeof(technitian_state_t));

    // Creazione task principale micro-ROS
    TaskHandle_t task;
    xTaskCreate( micro_ros_task, 
        "MICROROS_Task", 
        5000,
        NULL, 
        1,
        &task 
    );
    
	vTaskCoreAffinitySet(task, 1);

	vTaskStartScheduler();

    // Il programma non dovrebbe mai arrivare qui
	while (1){
	}
}