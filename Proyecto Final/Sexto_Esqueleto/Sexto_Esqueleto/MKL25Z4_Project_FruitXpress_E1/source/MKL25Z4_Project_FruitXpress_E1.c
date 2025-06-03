//Sexto esqueleto Proyecto Final FruitXpress
//Interfáz de usuario/bomba

#include <MKL25Z4.h>
#include <stdint.h>
#include <stdbool.h>
#include "hal_gpio.h"
#include "hal_ultrasonic.h"
#include "hal_bomba.h"
#include "ultrasonic_config.h"
#include "hal_lcd.h"
#include "hal_keypad.h"
#include "hal_delay.h"

// Constantes
#define UMBRAL_CM 10
#define TIEMPO_SERVIR_MS 3000

// Variables globales
volatile uint32_t lastKeyPressTime = 0;
volatile uint8_t displayMode = 0;

// Prototipos de funciones
void System_Init(void);
void delayMs(uint32_t ms);
void mostrar_menu_principal(void);
void mostrar_menu_seleccion(void);
void mostrar_estado_tanques(void);
void servir_jugo(uint8_t tanque_idx);
uint8_t hay_jugo_disponible(uint8_t tanque_idx);
void manejar_entrada_usuario(void);

int main(void) {
    System_Init();
    mostrar_menu_principal();
    while(1) {
        manejar_entrada_usuario();

        if((TPM0->CNT - lastKeyPressTime) > MENU_RESET_TIME) {
            mostrar_menu_principal();
        }
    }
}

// Función de inicialización del sistema
void System_Init(void) {
    SysTick_Init();
    TPM0_Init();
    HAL_LCD_Init();
    HAL_Keypad_Init();
    HAL_Delay_Init();

    for(uint32_t i = 0; i < NUM_TANQUES; i++) {
        HAL_Ultrasonic_Init(&tanques_config[i].sensor);
        HAL_Bomba_Init(&tanques_config[i].bomba);
    }

    HAL_LCD_Print("FruitXpress Ready!");
    delayMs(1000);
}

// Función para servir jugo
void servir_jugo(uint8_t tanque_idx) {
    if(tanque_idx >= NUM_TANQUES) return;

    const char* nombre = tanques_config[tanque_idx].nombre;

    HAL_LCD_Clear();
    HAL_LCD_Print("Sirviendo ");
    HAL_LCD_Print(nombre);

    // Activar bomba por tiempo determinado
    HAL_Bomba_Activar(&tanques_config[tanque_idx].bomba, true);
    delayMs(TIEMPO_SERVIR_MS);
    HAL_Bomba_Activar(&tanques_config[tanque_idx].bomba, false);

    HAL_LCD_Clear();
    HAL_LCD_Print("Listo!");
    delayMs(1500);
}

// Función para mostrar menú principal
void mostrar_menu_principal(void) {
    HAL_LCD_Clear();
    HAL_LCD_Print("1.Seleccionar");
    HAL_LCD_SetCursor(0, 1);
    HAL_LCD_Print("2.Estado tanques");

    displayMode = 0;
}

// Función para mostrar menú de selección
void mostrar_menu_seleccion(void) {
    HAL_LCD_Clear();
    HAL_LCD_Print("1.Naranja 2.Uva");
    HAL_LCD_SetCursor(0, 1);
    HAL_LCD_Print("3.Fresa #.Atras");

    displayMode = 1;
}

// Función para mostrar estado de los tanques
void mostrar_estado_tanques(void) {
    HAL_LCD_Clear();

    for(uint8_t i = 0; i < NUM_TANQUES; i++) {
        uint32_t distancia = HAL_Ultrasonic_GetDistance(i);
        uint8_t nivel = 0;

        if(distancia < 5) nivel = 3;       // Lleno
        else if(distancia < 10) nivel = 2; // Medio
        else if(distancia < 15) nivel = 1; // Bajo

        HAL_LCD_SetCursor(0, i);
        HAL_LCD_Print(tanques_config[i].nombre);
        HAL_LCD_Print(":");

        switch(nivel) {
            case 0: HAL_LCD_Print("Vacío"); break;
            case 1: HAL_LCD_Print("Bajo "); break;
            case 2: HAL_LCD_Print("Medio"); break;
            case 3: HAL_LCD_Print("Lleno"); break;
        }

        delayMs(100); // Pausa
    }

    HAL_LCD_SetCursor(0, 3);
    HAL_LCD_Print("#.Volver");

    displayMode = 2;
}

// Función para verificar disponibilidad de jugo
uint8_t hay_jugo_disponible(uint8_t tanque_idx) {
    if(tanque_idx >= NUM_TANQUES) return 0;

    uint32_t distancia = HAL_Ultrasonic_GetDistance(tanque_idx);
    return (distancia < UMBRAL_CM) ? 1 : 0;
}

// Función para manejar entrada del usuario
void manejar_entrada_usuario(void) {
    char tecla = HAL_Keypad_GetKey();

    if(!tecla) return;

    lastKeyPressTime = TPM0->CNT;

    switch(displayMode) {
        case 0: // Menú principal
            if(tecla == '1') {
                mostrar_menu_seleccion();
            } else if(tecla == '2') {
                mostrar_estado_tanques();
            }
            break;

        case 1: // Menú selección
            if(tecla >= '1' && tecla <= '3') {
                uint8_t tanque_idx = tecla - '1';
                if(hay_jugo_disponible(tanque_idx)) {
                    servir_jugo(tanque_idx);
                } else {
                    HAL_LCD_Clear();
                    HAL_LCD_Print("Tanque vacio!");
                    delayMs(1500);
                }
                mostrar_menu_principal();
            } else if(tecla == '#') {
                mostrar_menu_principal();
            }
            break;

        case 2: // Estado tanques
            if(tecla == '#') {
                mostrar_menu_principal();
            }
            break;
    }
}

// Funciones de delay
void delayMs(uint32_t ms) {
    for(uint32_t i = 0; i < ms; i++) {
        delayUs(1000);
    }
}

void delayUs(uint32_t us) {
    uint32_t start = TPM0->CNT;
    while((TPM0->CNT - start) < (us * 48)); // Ajustar según tu reloj
}


