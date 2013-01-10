
/*
 * hal_usart1_lpc1788.c
 *
 *  Created on: 2012-12-18
 *      Author: YangZhiyong
 */

#include "hal_usart.h"
#include "lpc177x_8x_clkpwr.h"
#include "lpc177x_8x_pinsel.h"
#include "lpc177x_8x_uart.h"
#include "util/fifo.h"

#define FIFO_SIZE		0x200
#define FIFO_TRIGGER	UART_FIFO_TRGLEV3

#if FIFO_TRIGGER == UART_FIFO_TRGLEV3
#define FIFO_THRESHOLD		14
#elif FIFO_TRIGGER == UART_FIFO_TRGLEV2
#define FIFO_THRESHOLD		8
#elif FIFO_TRIGGER == UART_FIFO_TRGLEV1
#define FIFO_THRESHOLD		4
#elif FIFO_TRIGGER == UART_FIFO_TRGLEV0
#define FIFO_THRESHOLD		1
#endif


FIFO_DEFINE(RecvFifo, FIFO_SIZE);
FIFO_DEFINE(SendFifo, FIFO_SIZE);

static USART_SendDoneISR 		SendDoneISR = 0;
static USART_RecvByteDoneISR	RecvByteDoneISR = 0;
static USART_RecvTimeoutISR		RecvTimeoutISR = 0;

static void readRecvFifo(void);
static uint32_t writeSendFifo(void);

void USART1_Init(void)
{
	CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCUART1, ENABLE);

    /*
     * Initialize UART1 pin connect
     * P0.15: U1_TXD
     * P0.16: U1_RXD
     */
    PINSEL_ConfigPin(0, 15, 1);
    PINSEL_ConfigPin(0, 16, 1);

    /* Initialize UART Configuration parameter structure to default state:
     * Baudrate = 115200 bps
     * 8 data bit
     * 1 Stop bit
     * None parity
     */
    // UART Configuration structure variable
    UART_CFG_Type UARTConfigStruct;

    UART_ConfigStructInit(&UARTConfigStruct);

    // Initialize UART0 peripheral with given to corresponding parameter
    UART_Init(UART_1, &UARTConfigStruct);


    /* Initialize FIFOConfigStruct to default state:
     *              - FIFO_DMAMode = DISABLE
     *              - FIFO_Level = UART_FIFO_TRGLEV0
     *              - FIFO_ResetRxBuf = ENABLE
     *              - FIFO_ResetTxBuf = ENABLE
     *              - FIFO_State = ENABLE
     */
    // UART FIFO configuration Struct variable
    UART_FIFO_CFG_Type UARTFIFOConfigStruct;

    UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

    // Initialize FIFO for UART0 peripheral
    UARTFIFOConfigStruct.FIFO_Level = FIFO_TRIGGER;
    UART_FIFOConfig(UART_1, &UARTFIFOConfigStruct);

    // Enable UART Transmit
    UART_TxCmd(UART_1, ENABLE);


    /* Enable UART Rx interrupt */
    UART_IntConfig(UART_1, UART_INTCFG_RBR, ENABLE);
    /* Enable UART line status interrupt */
    UART_IntConfig(UART_1, UART_INTCFG_RLS, ENABLE);

    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(UART1_IRQn, ((0x01<<3)|0x01));

    /* Enable Interrupt for UART0 channel */
    NVIC_EnableIRQ(UART1_IRQn);
}



uint32_t USART1_Send(void* buffer, uint32_t length)
{
	uint32_t send_length;

	send_length = FIFO_Put(&SendFifo, buffer, length);
	writeSendFifo();

	return send_length;
}



void USART5_SetRecvByteDoneISR    (USART_RecvByteDoneISR isr)
{
	RecvByteDoneISR = isr;
}



void USART1_SetRecvTimeoutISR    (USART_RecvTimeoutISR isr)
{
	RecvTimeoutISR = isr;
}



void USART1_SetSendDoneISR       (USART_SendDoneISR isr)
{
	SendDoneISR = isr;
}



static void readRecvFifo(void)
{
	uint8_t fifo_buffer[FIFO_THRESHOLD-1];
	uint8_t fifo_count = 0;

	while( ((LPC_UART1->LSR) & UART_LSR_BITMASK) & UART_LSR_RDR )
	{
		fifo_buffer[fifo_count] = LPC_UART1->RBR;
		fifo_count ++;
		if( fifo_count >= (FIFO_THRESHOLD-1) )
		{
			break;
		}
	}

	FIFO_Put(&RecvFifo, fifo_buffer, fifo_count);
}



static uint32_t writeSendFifo(void)
{
	uint8_t fifo_buffer[16];
	uint8_t fifo_count;
	uint8_t fifo_idx;

	fifo_count = FIFO_Get(&SendFifo, fifo_buffer, sizeof(fifo_buffer));

	for( fifo_idx = 0; fifo_idx < fifo_count; fifo_idx ++ )
	{
		LPC_UART1->THR = fifo_buffer[fifo_idx];
	}

	return fifo_count;
}



void UART1_IRQHandler(void)
{
    uint32_t intsrc;
    uint32_t iir;
    uint32_t lsr;

    /* Determine the interrupt source */
    intsrc = (LPC_UART1->IIR) & UART_IIR_BITMASK;
    iir = intsrc & UART_IIR_INTID_MASK;

    // Receive Line Status
    if ( iir == UART_IIR_INTID_RLS )
    {
        // Check line status
        lsr = (LPC_UART1->LSR) & UART_LSR_BITMASK;
        // Mask out the Receive Ready and Transmit Holding empty status
        lsr &= (UART_LSR_OE|UART_LSR_PE|UART_LSR_FE|UART_LSR_BI|UART_LSR_RXFE);
        // If any error exist
        if (lsr)
        {
//        	UART_IntErr(lsr);
        }
    }


    // Receive Data Available
    if ( iir == UART_IIR_INTID_RDA )
    {
    	readRecvFifo();
    }

    // Receive Character time-out
    if( iir == UART_IIR_INTID_CTI )
    {
    	readRecvFifo();
    	if( RecvTimeoutISR != 0 )
    		RecvTimeoutISR();
    }

    // Transmit Holding Empty
    if ( iir == UART_IIR_INTID_THRE )
    {
    	if( writeSendFifo() == 0 )
    	{
    		if( SendDoneISR != 0 )
    		{
    			SendDoneISR();
    		}
    	}
    }

}


