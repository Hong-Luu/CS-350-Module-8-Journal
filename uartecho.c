/*
 * Copyright (c) 2015-2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== uartecho.c ========
 */
#include <stdint.h>
#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>

/* Driver configuration */
#include "ti_drivers_config.h"

#define LED_ON_STATE  1
#define LED_OFF_STATE 0

void controlLED(unsigned char state, UART_Handle uart) {
    // Define constant messages to be sent through UART when the LED is turned on or off
    const char onMsg[] = "ON\r\n";
    const char offMsg[] = "OFF\r\n";

    // Check if the desired state is to turn the LED on
    if(state == LED_ON_STATE) {
        // If yes, set the LED GPIO pin to the on state, turning the LED on
        GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);
        // Send the "ON" message over UART to be displayed in the console
        UART_write(uart, onMsg, sizeof(onMsg));
    } else {
        // If the desired state is not LED_ON_STATE, then turn the LED off
        GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);
        // Send the "OFF" message over UART to be displayed in the console
        UART_write(uart, offMsg, sizeof(offMsg));
    }
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0) {
    char input;
    unsigned char state = LED_OFF_STATE; // Only one byte of RAM for the state
    const char  echoPrompt[] = "Echoing characters:\r\n";
    UART_Handle uart;
    UART_Params uartParams;

    /* Initialize drivers and configure LED */
    GPIO_init();
    UART_init();
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Initialize UART */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.baudRate = 115200;
    uart = UART_open(CONFIG_UART_0, &uartParams);
    if (uart == NULL) {
        /* UART_open() failed */
        while (1);
    }

    /* Initial LED state */
    controlLED(state, uart);

    /* Main loop */
    while (1) {
        UART_read(uart, &input, 1);
        UART_write(uart, &input, 1); // Echo the input character

        // State machine
        switch (state) {
            case LED_OFF_STATE:
                if (input == 'O') state = 'O'; // Temporary state holding 'O'
                break;
            case 'O':
                if (input == 'N') {
                    state = LED_ON_STATE;
                    controlLED(LED_ON_STATE, uart);
                } else if (input == 'F') {
                    state = 'F'; // Temporary state holding 'OF'
                } else {
                    state = LED_OFF_STATE;
                }
                break;
            case 'F':
                if (input == 'F') {
                    state = LED_OFF_STATE;
                    controlLED(LED_OFF_STATE, uart);
                } else {
                    state = LED_OFF_STATE;
                }
                break;
            case LED_ON_STATE:
                if (input == 'O') state = 'O'; // Could be the beginning of "OFF"
                break;
            default:
                state = LED_OFF_STATE;
                break;
        }
    }
}




