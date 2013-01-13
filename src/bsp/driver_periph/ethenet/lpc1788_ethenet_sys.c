/*
 * lpc1788_ethenet_sys.c
 *
 *  Created on: 2013-1-12
 *      Author: EYE
 */


#include "rtthread.h"
#include "netif/ethernetif.h"

static struct eth_device lpc1788_ethe_device;
static rt_uint8_t  MAC_Address[6];


static rt_err_t lpc1788_ethe_init(rt_device_t dev)
{

}



static rt_err_t lpc1788_ethe_open(rt_device_t dev, rt_uint16_t oflag)
{
	return RT_EOK;
}


static rt_err_t lpc1788_ethe_close(rt_device_t dev)
{
	return RT_EOK;
}

static rt_size_t lpc1788_ethe_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
	rt_set_errno(-RT_ENOSYS);
	return 0;
}

static rt_size_t lpc1788_ethe_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
	rt_set_errno(-RT_ENOSYS);
	return 0;
}



static rt_err_t lpc1788_ethe_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
	switch(cmd)
	{
	case NIOCTL_GADDR:
		/* get mac address */
		if(args) rt_memcpy(args, MAC_Address, 6);
		else return -RT_ERROR;
		break;

	default :
		break;
	}

	return RT_EOK;
}



rt_err_t rt_stm32_eth_tx( rt_device_t dev, struct pbuf* p)
{

}



struct pbuf *rt_stm32_eth_rx(rt_device_t dev)
{

}



void rt_hw_lpc1788_eth_init(void)
{
	/* PHY RESET */

    lpc1788_ethe_device.parent.init       = lpc1788_ethe_init;
    lpc1788_ethe_device.parent.open       = lpc1788_ethe_open;
    lpc1788_ethe_device.parent.close      = lpc1788_ethe_close;
    lpc1788_ethe_device.parent.read       = lpc1788_ethe_read;
    lpc1788_ethe_device.parent.write      = lpc1788_ethe_write;
    lpc1788_ethe_device.parent.control    = lpc1788_ethe_control;
    lpc1788_ethe_device.parent.user_data  = RT_NULL;

    /* register eth device */
    eth_device_init(&lpc1788_ethe_device, "ethe0");

    /* start phy monitor */
}




