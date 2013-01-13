/*
 * lpc1788_ethenet_mac.c
 *
 *  Created on: 2013-1-12
 *      Author: EYE
 */

#include "rtthread.h"
#include "LPC177x_8x.h"
#include "lpc1788_ethenet_mac.h"
#include "lpc177x_8x_emac.h"
#include "arch/cc.h"
#include "lwip/debug.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "netif/etharp.h"
#include "lwip/def.h"
#include "lwip/sys.h"

#ifndef LPC_EMAC_RMII
#error LPC_EMAC_RMII is not defined!
#endif

#if LPC_NUM_BUFF_TXDESCS < 2
#error LPC_NUM_BUFF_TXDESCS must be at least 2
#endif

#if LPC_NUM_BUFF_RXDESCS < 3
#error LPC_NUM_BUFF_RXDESCS must be at least 3
#endif

/** \brief  Receive group interrupts
 */
#define RXINTGROUP (EMAC_INT_RX_OVERRUN | EMAC_INT_RX_ERR | EMAC_INT_RX_DONE)

/** \brief  Transmit group interrupts
 */
#define TXINTGROUP (EMAC_INT_TX_UNDERRUN | EMAC_INT_TX_ERR | EMAC_INT_TX_DONE)

 /** \brief  Structure of a TX/RX descriptor
 */
typedef struct
{
	volatile void* 	packet;        /**< Pointer to buffer */
	volatile uint32_t 	control;       /**< Control word */
} LPC_TXRX_DESC_T;


/** \brief  Structure of a RX status entry
 */
typedef struct
{
	volatile uint32_t statusinfo;   /**< RX status word */
	volatile uint32_t statushashcrc; /**< RX hash CRC */
} LPC_TXRX_STATUS_T;


/* LPC EMAC driver data structure */
struct lpc_enetdata {
    /* prxs must be 8 byte aligned! */
	LPC_TXRX_STATUS_T prxs[LPC_NUM_BUFF_RXDESCS]; /**< Pointer to RX statuses */
	struct netif *netif;        /**< Reference back to LWIP parent netif */
	LPC_TXRX_DESC_T ptxd[LPC_NUM_BUFF_TXDESCS];   /**< Pointer to TX descriptor list */
	LPC_TXRX_STATUS_T ptxs[LPC_NUM_BUFF_TXDESCS]; /**< Pointer to TX statuses */
	LPC_TXRX_DESC_T prxd[LPC_NUM_BUFF_RXDESCS];   /**< Pointer to RX descriptor list */
	struct pbuf *rxb[LPC_NUM_BUFF_RXDESCS]; /**< RX pbuf pointer list, zero-copy mode */
	uint32_t rx_fill_desc_index; /**< RX descriptor next available index */
	volatile uint32_t rx_free_descs; /**< Count of free RX descriptors */
	struct pbuf *txb[LPC_NUM_BUFF_TXDESCS]; /**< TX pbuf pointer list, zero-copy mode */
	uint32_t lpc_last_tx_idx; /**< TX last descriptor index, zero-copy mode */

	rt_sem_t RxSem; /**< RX receive thread wakeup semaphore */
	rt_sem_t TxCleanSem; /**< TX cleanup thread wakeup semaphore */
	rt_mutex_t TXLockMutex; /**< TX critical section mutex */
	rt_sem_t xTXDCountSem; /**< TX free buffer counting semaphore */
};


/** \brief  LPC EMAC driver work data
 */
#if defined (__IAR_SYSTEMS_ICC__)
#pragma data_alignment=8
struct lpc_enetdata lpc_enetdata;
#else
ALIGN(8)
struct lpc_enetdata lpc1788_enetdata;
#endif


/* Write a value via the MII link (non-blocking) */
void lpc_mii_write_noblock(uint32_t PhyReg, uint32_t Value)
{
	/* Write value at PHY address and register */
	LPC_EMAC->MADR = (LPC_PHYDEF_PHYADDR << 8) | PhyReg;
	LPC_EMAC->MWTD = Value;
}

/* Write a value via the MII link (blocking) */
rt_err_t lpc_mii_write(uint32_t PhyReg, uint32_t Value)
{
	u32_t mst = 250;
	rt_err_t sts = RT_EOK;

	/* Write value at PHY address and register */
	lpc_mii_write_noblock(PhyReg, Value);

	/* Wait for unbusy status */
	while (mst > 0) {
		sts = LPC_EMAC->MIND;
		if ((sts & EMAC_MIND_BUSY) == 0)
			mst = 0;
		else {
			mst--;
			msDelay(1);
		}
	}

	if (sts != 0)
		sts = RT_ETIMEOUT;

	return sts;
}

/* Reads current MII link busy status */
uint32_t lpc_mii_is_busy(void)
{
	return (uint32_t) (LPC_EMAC->MIND & EMAC_MIND_BUSY);
}

/* Starts a read operation via the MII link (non-blocking) */
uint32_t lpc_mii_read_data(void)
{
	uint32_t data = LPC_EMAC->MRDD;
	LPC_EMAC->MCMD = 0;

	return data;
}

/* Starts a read operation via the MII link (non-blocking) */
void lpc_mii_read_noblock(u32_t PhyReg)
{
	/* Read value at PHY address and register */
	LPC_EMAC->MADR = (LPC_PHYDEF_PHYADDR << 8) | PhyReg;
	LPC_EMAC->MCMD = EMAC_MCMD_READ;
}

/* Read a value via the MII link (blocking) */
rt_err_t lpc_mii_read(uint32_t PhyReg, uint32_t *data)
{
	u32_t mst = 250;
	rt_err_t sts = RT_EOK;

	/* Read value at PHY address and register */
	lpc_mii_read_noblock(PhyReg);

	/* Wait for unbusy status */
	while (mst > 0) {
		sts = LPC_EMAC->MIND & ~EMAC_MIND_MII_LINK_FAIL;
		if ((sts & EMAC_MIND_BUSY) == 0) {
			mst = 0;
			*data = LPC_EMAC->MRDD;
		} else {
			mst--;
			msDelay(1);
		}
	}

	LPC_EMAC->MCMD = 0;

	if (sts != 0)
		sts = RT_ETIMEOUT;

	return sts;
}

/** \brief  Queues a pbuf into the RX descriptor list
 *
 *  \param[in] lpc_enetif Pointer to the drvier data structure
 *  \param[in] p            Pointer to pbuf to queue
 */
static void lpc_rxqueue_pbuf(struct lpc_enetdata *lpc_enetif, struct pbuf *p)
{
	u32_t idx;

	/* Get next free descriptor index */
	idx = lpc_enetif->rx_fill_desc_index;

	/* Setup descriptor and clear statuses */
	lpc_enetif->prxd[idx].control = EMAC_RCTRL_INT | ((uint32_t) (p->len - 1));
	lpc_enetif->prxd[idx].packet = p->payload;
	lpc_enetif->prxs[idx].statusinfo = 0xFFFFFFFF;
	lpc_enetif->prxs[idx].statushashcrc = 0xFFFFFFFF;

	/* Save pbuf pointer for push to network layer later */
	lpc_enetif->rxb[idx] = p;

	/* Wrap at end of descriptor list */
	idx++;
	if (idx >= LPC_NUM_BUFF_RXDESCS)
		idx = 0;

	/* Queue descriptor(s) */
	lpc_enetif->rx_free_descs -= 1;
	lpc_enetif->rx_fill_desc_index = idx;
	LPC_EMAC->RxConsumeIndex = idx;

	LWIP_DEBUGF(LWIP_DBG_TRACE,
		("lpc_rxqueue_pbuf: pbuf packet queued: %p (free desc=%d)\n", p,
			lpc_enetif->rx_free_descs));
}

/** \brief  Attempt to allocate and requeue a new pbuf for RX
 *
 *  \param[in]     netif Pointer to the netif structure
 *  \returns         1 if a packet was allocated and requeued, otherwise 0
 */
int32_t lpc_rx_queue(struct netif *netif)
{
	struct lpc_enetdata *lpc_enetif = netif->state;
	struct pbuf *p;
	s32_t queued = 0;

	/* Attempt to requeue as many packets as possible */
	while (lpc_enetif->rx_free_descs > 0) {
		/* Allocate a pbuf from the pool. We need to allocate at the
		   maximum size as we don't know the size of the yet to be
		   received packet. */
		p = pbuf_alloc(PBUF_RAW, (uint16_t) EMAC_ETH_MAX_FLEN, PBUF_RAM);
		if (p == NULL) {
			LWIP_DEBUGF(LWIP_DBG_TRACE,
				("lpc_rx_queue: could not allocate RX pbuf (free desc=%d)\n",
				lpc_enetif->rx_free_descs));
			return queued;
		}

		/* pbufs allocated from the RAM pool should be non-chained. */
		LWIP_ASSERT("lpc_rx_queue: pbuf is not contiguous (chained)",
			pbuf_clen(p) <= 1);

		/* Queue packet */
		lpc_rxqueue_pbuf(lpc_enetif, p);

		/* Update queued count */
		queued++;
	}

	return queued;
}

/** \brief  Sets up the RX descriptor ring buffers.
 *
 *  This function sets up the descriptor list used for receive packets.
 *
 *  \param[in]  lpc_enetif  Pointer to driver data structure
 *  \returns                   Always returns ERR_OK
 */
static rt_err_t lpc_rx_setup(struct lpc_enetdata *lpc_enetif)
{

	/* Setup pointers to RX structures */
	LPC_EMAC->RxDescriptor = (uint32_t) &lpc_enetif->prxd[0];
	LPC_EMAC->RxStatus = (uint32_t) &lpc_enetif->prxs[0];
	LPC_EMAC->RxDescriptorNumber = LPC_NUM_BUFF_RXDESCS - 1;

	lpc_enetif->rx_free_descs = LPC_NUM_BUFF_RXDESCS;
	lpc_enetif->rx_fill_desc_index = 0;

	/* Build RX buffer and descriptors */
	lpc_rx_queue(lpc_enetif->netif);

	return RT_EOK;
}


/** \brief  Determine if the passed address is usable for the ethernet
 *          DMA controller.
 *
 *  \param[in] addr Address of packet to check for DMA safe operation
 *  \return          1 if the packet address is not safe, otherwise 0
 */
static int32_t lpc_packet_addr_notsafe(void *addr)
{
	/* Check for legal address ranges */
	if ((((u32_t) addr >= 0x20000000) && ((u32_t) addr < 0x20008000)) ||
		(((u32_t) addr >= 0x80000000) && ((u32_t) addr < 0xF0000000)))
		return 0;

	return 1;
}



/** \brief  Allocates a pbuf and returns the data from the incoming packet.
 *
 *  \param[in] netif the lwip network interface structure for this lpc_enetif
 *  \return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *lpc_low_level_input(struct netif *netif)
{
	struct lpc_enetdata *lpc_enetif = netif->state;
	struct pbuf *p = NULL, *q;
	u32_t idx, length;
	u8_t *src;

	sys_mutex_lock(&lpc_enetif->TXLockMutex);


	/* Monitor RX overrun status. This should never happen unless
	   (possibly) the internal bus is behing held up by something.
	   Unless your system is running at a very low clock speed or
	   there are possibilities that the internal buses may be held
	   up for a long time, this can probably safely be removed. */
	if (LPC_EMAC->IntStatus & EMAC_INT_RX_OVERRUN) {
//		LINK_STATS_INC(link.err);
//		LINK_STATS_INC(link.drop);

		/* Temporarily disable RX */
		LPC_EMAC->MAC1 &= ~EMAC_MAC1_REC_EN;

		/* Reset the RX side */
		LPC_EMAC->MAC1 |= EMAC_MAC1_RES_RX;
		LPC_EMAC->IntClear = EMAC_INT_RX_OVERRUN;

		/* De-allocate all queued RX pbufs */
		for (idx = 0; idx < LPC_NUM_BUFF_RXDESCS; idx++) {
			if (lpc_enetif->rxb[idx] != NULL) {
				pbuf_free(lpc_enetif->rxb[idx]);
				lpc_enetif->rxb[idx] = NULL;
			}
		}

		/* Start RX side again */
		lpc_rx_setup(lpc_enetif);

		/* Re-enable RX */
		LPC_EMAC->MAC1 |= EMAC_MAC1_REC_EN;

		sys_mutex_unlock(&lpc_enetif->TXLockMutex);

		return NULL;
	}

	/* Determine if a frame has been received */
	length = 0;
	idx = LPC_EMAC->RxConsumeIndex;
	if (LPC_EMAC->RxProduceIndex != idx) {
		/* Handle errors */
		if (lpc_enetif->prxs[idx].statusinfo & (EMAC_RINFO_CRC_ERR |
			EMAC_RINFO_SYM_ERR | EMAC_RINFO_ALIGN_ERR | EMAC_RINFO_LEN_ERR))
		{
//#if LINK_STATS
//			if (lpc_enetif->prxs[idx].statusinfo & (EMAC_RINFO_CRC_ERR |
//				EMAC_RINFO_SYM_ERR | EMAC_RINFO_ALIGN_ERR))
//				LINK_STATS_INC(link.chkerr);
//			if (lpc_enetif->prxs[idx].statusinfo & EMAC_RINFO_LEN_ERR)
//				LINK_STATS_INC(link.lenerr);
//#endif

			/* Drop the frame */
//			LINK_STATS_INC(link.drop);

			/* Re-queue the pbuf for receive */
			lpc_enetif->rx_free_descs++;
			p = lpc_enetif->rxb[idx];
			lpc_enetif->rxb[idx] = NULL;
			lpc_rxqueue_pbuf(lpc_enetif, p);

			LWIP_DEBUGF(UDP_LPC_EMAC | LWIP_DBG_TRACE,
				("lpc_low_level_input: Packet dropped with errors (0x%x)\n",
				lpc_enetif->prxs[idx].statusinfo));

			p = NULL;
		}
		else
		{
			/* A packet is waiting, get length */
			length = (lpc_enetif->prxs[idx].statusinfo & 0x7FF) + 1;

			/* Zero-copy */
			p = lpc_enetif->rxb[idx];
			p->len = (u16_t) length;

			/* Free pbuf from desriptor */
			lpc_enetif->rxb[idx] = NULL;
			lpc_enetif->rx_free_descs++;

			LWIP_DEBUGF(LWIP_DBG_TRACE,
				("lpc_low_level_input: Packet received: %p, size %d (index=%d)\n",
				p, length, idx));

			/* Save size */
			p->tot_len = (u16_t) length;
//			LINK_STATS_INC(link.recv);

			/* Queue new buffer(s) */
			lpc_rx_queue(lpc_enetif->netif);
		}
	}

	sys_mutex_unlock(&lpc_enetif->TXLockMutex);

	return p;
}



/** \brief  Attempt to read a packet from the EMAC interface.
 *
 *  \param[in] netif the lwip network interface structure for this lpc_enetif
 */
void lpc_enetif_input(struct netif *netif)
{
	struct eth_hdr *ethhdr;
	struct pbuf *p;

	/* move received packet into a new pbuf */
	p = lpc_low_level_input(netif);
	if (p == NULL)
		return;

	/* points to packet payload, which starts with an Ethernet header */
	ethhdr = p->payload;

	switch (htons(ethhdr->type)) {
		case ETHTYPE_IP:
		case ETHTYPE_ARP:
#if PPPOE_SUPPORT
		case ETHTYPE_PPPOEDISC:
		case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
			/* full packet send to tcpip_thread to process */
			if (netif->input(p, netif) != ERR_OK) {
				LWIP_DEBUGF(NETIF_DEBUG, ("lpc_enetif_input: IP input error\n"));
				/* Free buffer */
				pbuf_free(p);
			}
			break;

		default:
			/* Return buffer */
			pbuf_free(p);
			break;
	}
}



/** \brief  Polls if an available TX descriptor is ready. Can be used to
*           determine if the low level transmit function will block.
*
*  \param[in] netif the lwip network interface structure for this lpc_enetif
*  \return 0 if no descriptors are read, or >0
*/
int32_t lpc_tx_ready(struct netif *netif)
{
	int32_t fb;
	uint32_t idx, cidx;

	cidx = LPC_EMAC->TxConsumeIndex;
	idx = LPC_EMAC->TxProduceIndex;

	/* Determine number of free buffers */
	if (idx == cidx)
		fb = LPC_NUM_BUFF_TXDESCS;
	else if (cidx > idx)
		fb = (LPC_NUM_BUFF_TXDESCS - 1) -
			((idx + LPC_NUM_BUFF_TXDESCS) - cidx);
	else
		fb = (LPC_NUM_BUFF_TXDESCS - 1) - (cidx - idx);

	return fb;
}



/** \brief  Low level output of a packet. Never call this from an
 *          interrupt context, as it may block until TX descriptors
 *          become available.
 *
 *  \param[in] netif the lwip network interface structure for this lpc_enetif
 *  \param[in] p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 *  \return ERR_OK if the packet could be sent or an err_t value if the packet couldn't be sent
 */
static err_t lpc_low_level_output(struct netif *netif, struct pbuf *p)
{
	struct lpc_enetdata *lpc_enetif = netif->state;
	struct pbuf *q;
	u8_t *dst;
	u32_t idx, sz = 0;
	err_t err = ERR_OK;
	struct pbuf *np;
	u32_t dn, notdmasafe = 0;

	/* Zero-copy TX buffers may be fragmented across mutliple payload
	   chains. Determine the number of descriptors needed for the
	   transfer. The pbuf chaining can be a mess! */
	dn = (u32_t) pbuf_clen(p);

	/* Test to make sure packet addresses are DMA safe. A DMA safe
	   address is once that uses external memory or periphheral RAM.
	   IRAM and FLASH are not safe! */
	for (q = p; q != NULL; q = q->next)
		notdmasafe += lpc_packet_addr_notsafe(q->payload);

#if LPC_TX_PBUF_BOUNCE_EN==1
	/* If the pbuf is not DMA safe, a new bounce buffer (pbuf) will be
	   created that will be used instead. This requires an copy from the
	   non-safe DMA region to the new pbuf */
	if (notdmasafe) {
		/* Allocate a pbuf in DMA memory */
		np = pbuf_alloc(PBUF_RAW, p->tot_len, PBUF_RAM);
		if (np == NULL)
			return ERR_MEM;

		/* This buffer better be contiguous! */
		LWIP_ASSERT("lpc_low_level_output: New transmit pbuf is chained",
			(pbuf_clen(np) == 1));

		/* Copy to DMA safe pbuf */
		dst = (u8_t *) np->payload;
	 	for(q = p; q != NULL; q = q->next) {
			/* Copy the buffer to the descriptor's buffer */
	  		MEMCPY(dst, (u8_t *) q->payload, q->len);
		  dst += q->len;
		}
		np->len = p->tot_len;

		LWIP_DEBUGF(UDP_LPC_EMAC | LWIP_DBG_TRACE,
			("lpc_low_level_output: Switched to DMA safe buffer, old=%p, new=%p\n",
			q, np));

		/* use the new buffer for descrptor queueing. The original pbuf will
		   be de-allocated outsuide this driver. */
		p = np;
		dn = 1;
	}
#else
	if (notdmasafe)
		LWIP_ASSERT("lpc_low_level_output: Not a DMA safe pbuf",
			(notdmasafe == 0));
#endif

	/* Wait until enough descriptors are available for the transfer. */
	/* THIS WILL BLOCK UNTIL THERE ARE ENOUGH DESCRIPTORS AVAILABLE */
	while (dn > lpc_tx_ready(netif))
//#if NO_SYS == 0
//		xSemaphoreTake(lpc_enetif->xTXDCountSem, 0);
	sys_arch_sem_wait(&lpc_enetif->xTXDCountSem, 0);
//#else
//		msDelay(1);
//#endif

	/* Get free TX buffer index */
	idx = LPC_EMAC->TxProduceIndex;

//#if NO_SYS == 0
	/* Get exclusive access */
	sys_mutex_lock(&lpc_enetif->TXLockMutex);
//#endif

	/* Prevent LWIP from de-allocating this pbuf. The driver will
	   free it once it's been transmitted. */
	if (!notdmasafe)
		pbuf_ref(p);

	/* Setup transfers */
	q = p;
	while (dn > 0) {
		dn--;

		/* Only save pointer to free on last descriptor */
		if (dn == 0) {
			/* Save size of packet and signal it's ready */
			lpc_enetif->ptxd[idx].control = (q->len - 1) | EMAC_TCTRL_INT |
				EMAC_TCTRL_LAST;
			lpc_enetif->txb[idx] = p;
		}
		else {
			/* Save size of packet, descriptor is not last */
			lpc_enetif->ptxd[idx].control = (q->len - 1) | EMAC_TCTRL_INT;
			lpc_enetif->txb[idx] = NULL;
		}

		LWIP_DEBUGF(UDP_LPC_EMAC | LWIP_DBG_TRACE,
			("lpc_low_level_output: pbuf packet(%p) sent, chain#=%d,"
			" size = %d (index=%d)\n", q->payload, dn, q->len, idx));

		lpc_enetif->ptxd[idx].packet = (u32_t) q->payload;

		q = q->next;

		idx++;
		if (idx >= LPC_NUM_BUFF_TXDESCS)
			idx = 0;
	}

	LPC_EMAC->TxProduceIndex = idx;

//	LINK_STATS_INC(link.xmit);

//#if NO_SYS == 0
	/* Restore access */
	sys_mutex_unlock(&lpc_enetif->TXLockMutex);
//#endif

	return ERR_OK;
}





/** \brief  Sets up the TX descriptor ring buffers.
 *
 *  This function sets up the descriptor list used for transmit packets.
 *
 *  \param[in]      lpc_enetif  Pointer to driver data structure
 */
static err_t lpc_tx_setup(struct lpc_enetdata *lpc_enetif)
{
	s32_t idx;

	/* Build TX descriptors for local buffers */
	for (idx = 0; idx < LPC_NUM_BUFF_TXDESCS; idx++) {
		lpc_enetif->ptxd[idx].control = 0;
		lpc_enetif->ptxs[idx].statusinfo = 0xFFFFFFFF;
	}

	/* Setup pointers to TX structures */
	LPC_EMAC->TxDescriptor = (u32_t) &lpc_enetif->ptxd[0];
	LPC_EMAC->TxStatus = (u32_t) &lpc_enetif->ptxs[0];
	LPC_EMAC->TxDescriptorNumber = LPC_NUM_BUFF_TXDESCS - 1;

	lpc_enetif->lpc_last_tx_idx = 0;

	return ERR_OK;
}

/** \brief  Free TX buffers that are complete
 *
 *  \param[in] lpc_enetif  Pointer to driver data structure
 *  \param[in] cidx  EMAC current descriptor comsumer index
 */
static void lpc_tx_reclaim_st(struct lpc_enetdata *lpc_enetif, u32_t cidx)
{
#if NO_SYS == 0
	/* Get exclusive access */
	sys_mutex_lock(&lpc_enetif->TXLockMutex);
#endif

	while (cidx != lpc_enetif->lpc_last_tx_idx) {
		if (lpc_enetif->txb[lpc_enetif->lpc_last_tx_idx] != NULL) {
			LWIP_DEBUGF(UDP_LPC_EMAC | LWIP_DBG_TRACE,
				("lpc_tx_reclaim_st: Freeing packet %p (index %d)\n",
				lpc_enetif->txb[lpc_enetif->lpc_last_tx_idx],
				lpc_enetif->lpc_last_tx_idx));
			pbuf_free(lpc_enetif->txb[lpc_enetif->lpc_last_tx_idx]);
		 	lpc_enetif->txb[lpc_enetif->lpc_last_tx_idx] = NULL;
		}

//#if NO_SYS == 0
//		xSemaphoreGive(lpc_enetif->xTXDCountSem);
		sys_sem_signal(&lpc1788_enetdata.xTXDCountSem);
//#endif
		lpc_enetif->lpc_last_tx_idx++;
		if (lpc_enetif->lpc_last_tx_idx >= LPC_NUM_BUFF_TXDESCS)
			lpc_enetif->lpc_last_tx_idx = 0;
	}

#if NO_SYS == 0
	/* Restore access */
	sys_mutex_unlock(&lpc_enetif->TXLockMutex);
#endif
}

/** \brief  User call for freeingTX buffers that are complete
 *
 *  \param[in] netif the lwip network interface structure for this lpc_enetif
 */
void lpc_tx_reclaim(struct netif *netif)
{
	lpc_tx_reclaim_st((struct lpc_enetdata *) netif->state,
		LPC_EMAC->TxConsumeIndex);
}





/** \brief  LPC EMAC interrupt handler.
 *
 *  This function handles the transmit, receive, and error interrupt of
 *  the LPC177x_8x. This is meant to be used when NO_SYS=0.
 */
void ENET_IRQHandler(void)
{
#if NO_SYS == 1
	/* Interrupts are not used without an RTOS */
    NVIC_DisableIRQ(ENET_IRQn);
#else
//	signed portBASE_TYPE xRecTaskWoken = pdFALSE, XTXTaskWoken = pdFALSE;
	uint32_t ints;

	/* Interrupts are of 2 groups - transmit or receive. Based on the
	   interrupt, kick off the receive or transmit (cleanup) task */

//	SYS_EnterInt();

	/* Get pending interrupts */
	ints = LPC_EMAC->IntStatus;

	if (ints & RXINTGROUP) {
		/* RX group interrupt(s) */
		/* Give semaphore to wakeup RX receive task. Note the FreeRTOS
		   method is used instead of the LWIP arch method. */
//        xSemaphoreGiveFromISR(lpc_enetdata.RxSem, &xRecTaskWoken);
        sys_sem_signal(&lpc1788_enetdata.RxSem);
	}

	if (ints & TXINTGROUP) {
		/* TX group interrupt(s) */
		/* Give semaphore to wakeup TX cleanup task. Note the FreeRTOS
		   method is used instead of the LWIP arch method. */
//        xSemaphoreGiveFromISR(lpc_enetdata.TxCleanSem, &XTXTaskWoken);
	    sys_sem_signal(&lpc1788_enetdata.TxCleanSem);
	}

	/* Clear pending interrupts */
	LPC_EMAC->IntClear = ints;

	/* Context switch needed? */
//	SYS_ExitInt();
//	portEND_SWITCHING_ISR( xRecTaskWoken || XTXTaskWoken );
#endif
}










