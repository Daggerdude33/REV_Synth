/*
 * Copyright (c) 2021, Texas Instruments Incorporated
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

#include "ti/driverlib/dl_dma.h"
#include "ti_msp_dl_config.h"

#define START_COUNT 2155


#define SI 4095
//4600 Octave 3 A
#define A_s 2273 //a
#define Ash_s 2145 //w
#define B_s 2025 //s
#define C_s 1911 //d
#define Csh_s 1804 //r
#define D_s 1703 //f
#define Dsh_s 1607 //t
#define E_s 1517 //g
#define F_s 1432 //h
#define Fsh_s 1351 //u
#define G_s 1276 //j
#define Gsh_s 1204 //i


/* Repetitive sine wave */
const uint16_t gOutputSignalSine64[] = {2048, 2248, 2447, 2642, 2831, 3013,
    3185, 3347, 3496, 3631, 3750, 3854, 3940, 4007, 4056, 4086, 4095, 4086,
    4056, 4007, 3940, 3854, 3750, 3631, 3496, 3347, 3185, 3013, 2831, 2642,
    2447, 2248, 2048, 1847, 1648, 1453, 1264, 1082, 910, 748, 599, 464, 345,
    241, 155, 88, 39, 9, 0, 9, 39, 88, 155, 241, 345, 464, 599, 748, 910, 1082,
    1264, 1453, 1648, 1847};


uint16_t gOutputSignalSquare64[START_COUNT];
uint16_t gOutputSignalOff64[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


volatile uint8_t gEchoData = 0;
uint64_t N = 0; //Sample Size
//pNote = A;

uint16_t adj_count = START_COUNT;

void DAC_sample_set(uint16_t count){
    uint16_t Square[count];
    for(uint64_t i = 0; i < (count); i++){
        if (i < (count/2)) {
            Square[i] = 0;
        }
        else {
            Square[i] = SI;
        }    
    }
    DL_DMA_disableChannel(DMA, DMA_CH0_CHAN_ID);
    DL_DMA_setSrcAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t) &Square[0]);
    DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, sizeof(Square) / sizeof(uint16_t));
    DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID); 
}




int main(void)
{
    SYSCFG_DL_init();
    NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_1_INST_INT_IRQN);
    /* Configure DMA source, destination and size */
    DL_DMA_setSrcAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t) &gOutputSignalSine64[0]);
    DL_DMA_setDestAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t) & (DAC0->DATA0));
    DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, sizeof(gOutputSignalSine64) / sizeof(uint16_t));

    DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);
    DAC_sample_set(adj_count);
    DL_SYSCTL_enableSleepOnExit();
    while (1) {
        __WFI();
    }
}

void reset_DAC(){

}

void UART_0_INST_IRQHandler(void)
{
    switch (DL_UART_Main_getPendingInterrupt(UART_0_INST)) {  
        case DL_UART_MAIN_IIDX_RX:
            //DL_GPIO_togglePins(GPIO_LEDS_PORT,GPIO_LEDS_USER_LED_1_PIN | GPIO_LEDS_USER_TEST_PIN);
            gEchoData = DL_UART_Main_receiveData(UART_0_INST);
            DL_UART_Main_transmitData(UART_0_INST, gEchoData);
            if (gEchoData == '1') {
                DAC_sample_set(adj_count);    
            }
            else if (gEchoData == '2') {
                DL_DMA_disableChannel(DMA, DMA_CH0_CHAN_ID);
                DL_DMA_setSrcAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t) &gOutputSignalSine64[0]);
                DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, sizeof(gOutputSignalSine64) / sizeof(uint16_t));
                DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);
                
            }
            else if (gEchoData == '3') {
                DL_DMA_disableChannel(DMA, DMA_CH0_CHAN_ID);
                DL_DMA_setSrcAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t) &gOutputSignalOff64[0]);
                DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, sizeof(gOutputSignalOff64) / sizeof(uint16_t));
                DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);
            }
            else if (gEchoData == 'a') {
                DAC_sample_set(A_s);
            }
            else if (gEchoData == 'w') {
                DAC_sample_set(Ash_s);
            }
            else if (gEchoData == 's') {
                DAC_sample_set(B_s);  
            }
            else if (gEchoData == 'd') {
                DAC_sample_set(C_s);
            }
            else if (gEchoData == 'r') {
                DAC_sample_set(Csh_s);
            }
            else if (gEchoData == 'f') {
                DAC_sample_set(D_s);
            }
            else if (gEchoData == 't') {
                DAC_sample_set(Dsh_s);
            }
            else if (gEchoData == 'g') {
                DAC_sample_set(E_s);
            }
            else if (gEchoData == 'h') {
                DAC_sample_set(F_s);
            }
            else if (gEchoData == 'u') {
                DAC_sample_set(Fsh_s);
            }
            else if (gEchoData == 'j') {
                DAC_sample_set(G_s);
            }
            else if (gEchoData == 'i') {
                DAC_sample_set(Gsh_s);
            }
            else if(gEchoData == ','){
                adj_count++;
                DAC_sample_set(adj_count);
            }
            else if (gEchoData == '.') {
                adj_count--;
                DAC_sample_set(adj_count);
            }
            else if(gEchoData == '<'){
                adj_count = adj_count + 10;
                DAC_sample_set(adj_count);
            }
            else if (gEchoData == '>') {
                adj_count = adj_count - 10;
                DAC_sample_set(adj_count);
            }

            break;
        default:
            break;
    }
}
void UART_1_INST_IRQHandler(void)
{
   switch (DL_UART_Main_getPendingInterrupt(UART_1_INST)) {  
        case DL_UART_MAIN_IIDX_RX:
            //DL_GPIO_togglePins(GPIO_LEDS_PORT,GPIO_LEDS_USER_LED_1_PIN | GPIO_LEDS_USER_TEST_PIN);
            gEchoData = DL_UART_Main_receiveData(UART_1_INST);
            DL_UART_Main_transmitData(UART_0_INST, gEchoData);
            if (gEchoData != 0) {
                DAC_sample_set(gEchoData * 1);
            }
            break;
        default:
            break;
    }
}