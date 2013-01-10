/*
 * lpc1788_usart.c
 *
 *  Created on: 2013-1-10
 *      Author: YangZhiyong
 */

#include "rtthread.h"
#include "LPC177x_8x.h"
#include "lpc177x_8x_uart.h"

rt_uint8_t RecvBuffer[0x40];



static rt_err_t configure(	struct rt_serial_device *serial,
							struct serial_configure *cfg)
{
	rt_err_t error = RT_EOK;


	return error;
}



static rt_err_t control(	struct rt_serial_device *serial,
							int cmd,
							void *arg	)
{
	rt_err_t error = RT_EOK;


	return error;
}



static int putc(struct rt_serial_device *serial, char c)
{
	int result = -1;


	return result;
}



static int getc(struct rt_serial_device *serial)
{
	int result = -1;


	return result;
}



static const struct rt_uart_ops usart_ops =
{
	configure,
	control,
	putc,
	getc,
};


void rt_hw_usart_init(void)
{


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
    	rt_hw_serial_char_isr();
    }

    // Receive Character time-out
    if( iir == UART_IIR_INTID_CTI )
    {
    	rt_hw_serial_timeout_isr();
    }

    // Transmit Holding Empty
    if ( iir == UART_IIR_INTID_THRE )
    {
    	if( writeSendFifo() == 0 )
    	{
    		rt_hw_serial_dma_tx_isr();
    	}
    }

}


