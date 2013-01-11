/*
 * lpc1788_usart.c
 *
 *  Created on: 2013-1-10
 *      Author: YangZhiyong
 */

#include "rtthread.h"
#include "LPC177x_8x.h"
#include "lpc177x_8x_uart.h"
#include "drivers/serial.h"
#include "lpc177x_8x_clkpwr.h"
#include "lpc177x_8x_pinsel.h"


#define RT_DEVICE_FLAG_TX	RT_DEVICE_FLAG_INT_TX	//RT_DEVICE_FLAG_DMA_TX

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


static rt_uint8_t RecvFifoLength;
static rt_uint8_t SendFifoLength;
char *SendPtr;
rt_size_t SendLength;

struct serial_ringbuffer serial1_int_rx;
struct serial_ringbuffer serial1_int_tx;

struct rt_serial_device serial1;


static rt_err_t configure(	struct rt_serial_device *serial,
							struct serial_configure *cfg)
{
	rt_err_t error = RT_EOK;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);


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

    switch( cfg->baud_rate )
    {
    case BAUD_RATE_115200:
    	UARTConfigStruct.Baud_rate = 115200;
    	break;
    case BAUD_RATE_9600:
    	UARTConfigStruct.Baud_rate = 9600;
    	break;
    case BAUD_RATE_4800:
    	UARTConfigStruct.Baud_rate = 4800;
    	break;
    default:
    	UARTConfigStruct.Baud_rate = 115200;
    	break;
    }

    switch( cfg->parity )
    {
    case PARITY_NONE:
    	UARTConfigStruct.Parity = UART_PARITY_NONE;
    	break;
    case PARITY_ODD:
    	UARTConfigStruct.Parity = UART_PARITY_ODD;
    	break;
    case PARITY_EVEN:
    	UARTConfigStruct.Parity = UART_PARITY_EVEN;
    	break;
    default:
    	UARTConfigStruct.Parity = UART_PARITY_NONE;
    	break;
    }

    switch( cfg->data_bits )
    {
    case DATA_BITS_9:
    	error = RT_EIO;
    	break;
    case DATA_BITS_8:
    	UARTConfigStruct.Databits = UART_DATABIT_8;
    	break;
    case DATA_BITS_7:
    	UARTConfigStruct.Databits = UART_DATABIT_7;
    	break;
    case DATA_BITS_6:
    	UARTConfigStruct.Databits = UART_DATABIT_6;
    	break;
    case DATA_BITS_5:
    	UARTConfigStruct.Databits = UART_DATABIT_5;
    	break;
    default:
    	UARTConfigStruct.Databits = UART_DATABIT_8;
    	break;
    }

    switch( cfg->stop_bits )
    {
    case STOP_BITS_1:
    	UARTConfigStruct.Stopbits = UART_STOPBIT_1;
    	break;
    case STOP_BITS_2:
    	UARTConfigStruct.Stopbits = UART_STOPBIT_2;
    	break;
    case STOP_BITS_3:
    	error = RT_EIO;
    	break;
    case STOP_BITS_4:
    	error = RT_EIO;
    	break;
    default:
    	UARTConfigStruct.Stopbits = UART_STOPBIT_1;
    	break;
    }

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
    UART_IntConfig(UART_1, UART_INTCFG_THRE, ENABLE);
    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(UART1_IRQn, ((0x01<<3)|0x01));

    /* Enable Interrupt for UART0 channel */
    NVIC_EnableIRQ(UART1_IRQn);

	return error;
}



static rt_err_t control(	struct rt_serial_device *serial,
							int cmd,
							void *arg	)
{
	rt_err_t error = RT_EOK;

	RT_ASSERT(serial != RT_NULL);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        /* disable rx irq */
    	NVIC_DisableIRQ(UART1_IRQn);
        break;
    case RT_DEVICE_CTRL_SET_INT:
        /* enable rx irq */
    	NVIC_EnableIRQ(UART1_IRQn);
        break;
    default:
    	error = RT_EIO;
    	break;
    }

	return error;
}



static int putchar(struct rt_serial_device *serial, char c)
{
	RT_ASSERT(serial != RT_NULL);

	if( SendFifoLength < 15 )
	{
		LPC_UART1->THR = c & UART_THR_MASKBIT;
		SendFifoLength ++;
	}

	return (15 - SendFifoLength);
}



static int getchar(struct rt_serial_device *serial)
{
	int result = -1;

	RT_ASSERT(serial != RT_NULL);

	while( ((LPC_UART1->LSR) & UART_LSR_BITMASK) & UART_LSR_RDR )
	{
		if( RecvFifoLength > 1 )		/// 留下最后一个字节触发字符超时中断。
		{
			result = LPC_UART1->RBR;
			RecvFifoLength--;
		}
		else
		{
			result = -1;
			break;
		}
	}

	return result;
}



static uint32_t writeSendFifo(void)
{
	uint8_t write_cnt = 0;

	for( ; SendFifoLength < 16; SendFifoLength ++ )
	{
		if( SendLength > 0)
		{
			LPC_UART1->THR = (*SendPtr) & UART_THR_MASKBIT;
			SendPtr ++;
			SendLength --;
			write_cnt ++;
		}
		else
		{
			break;
		}
	}

	return write_cnt;
}



rt_size_t dma_transmit(struct rt_serial_device *serial, const char *buf, rt_size_t size)
{
	RT_ASSERT(serial != RT_NULL);

	SendPtr = buf;
	SendLength = size;

	writeSendFifo();

	return size;
}



static const struct rt_uart_ops usart_ops =
{
	.configure	= configure,
	.control	= control,
	.putc		= putchar,
	.getc		= getchar,
	.dma_transmit = dma_transmit,
};


void rt_hw_usart_init(void)
{
	struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

	serial1.ops    = &usart_ops;
	serial1.int_rx = &serial1_int_rx;
	serial1.int_tx = &serial1_int_tx;
	serial1.config = config;

	configure(&serial1, &config);

    /* register USART1 device */
    rt_hw_serial_register(&serial1, "serial1",
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_TX,
                          (void*)0 );
}


////////////////////////////////////////////////////////////


void UART1_IRQHandler(void)
{
    uint32_t intsrc;
    uint32_t iir;
    uint32_t lsr;

    rt_interrupt_enter();

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
    	RecvFifoLength = FIFO_THRESHOLD;
    	rt_hw_serial_char_isr(&serial1);
    }

    // Receive Character time-out
    if( iir == UART_IIR_INTID_CTI )
    {
    	RecvFifoLength = FIFO_THRESHOLD;
    	rt_hw_serial_timeout_isr(&serial1);
    }

    // Transmit Holding Empty
    if ( iir == UART_IIR_INTID_THRE )
    {
    	SendFifoLength = 0;

#if (RT_DEVICE_FLAG_TX == RT_DEVICE_FLAG_DMA_TX)
    	if( writeSendFifo() == 0 )
    	{
    		rt_hw_serial_dma_tx_isr(&serial1);
    	}
#else
    	rt_hw_serial_int_tx_isr(&serial1);
#endif
    }

    rt_interrupt_leave();

}


