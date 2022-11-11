/**
 * @file    deca_SPI.c
 *
 * @brief   Hardware abstraction layer for SPI
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */


#include <stdint.h>
#include <string.h>

#include "HAL_lock.h"
#include "HAL_SPI.h"
#include "error.h"
#include "boards.h"
#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "nrf_error.h"

static void spi_slow_rate(void);
static void spi_fast_rate(void);
static void spi_cs_low(void);
static void spi_cs_high(void);
int readfromspi(uint16_t headerLength, uint8_t *headerBuffer, uint16_t readlength, uint8_t *readBuffer);
int writetospi(uint16_t headerLength, const uint8_t *headerBuffer, uint16_t readlength, const uint8_t *readBuffer); 
int writetospiwithcrc(uint16_t  headerLength, const uint8_t *headerBuffer,
                      uint16_t  bodyLength, const uint8_t *bodyBuffer,  uint8_t crc8);

typedef struct
{
  nrf_drv_spi_t         spi_inst;
  uint32_t              frequency_slow;
  uint32_t              frequency_fast;
  uint32_t              cs_pin;
  nrf_drv_spi_config_t  spi_config;
  dw_hal_lockTypeDef    lock;
}spi_handle_t;

static spi_handle_t spi_handler = 
{
/* below will be configured in the port_init_dw_chip() */
  .spi_inst       = {0},
  .frequency_slow = 0, 
  .frequency_fast = 0,
  .spi_config     = {0},
  .cs_pin          = DW3000_CS_Pin,
  .lock           = DW_HAL_NODE_UNLOCKED
};

static volatile bool spi_xfer_done;

static const struct spi_s spim0 =
{
    .cs_high = spi_cs_high,
    .cs_low = spi_cs_low,
    .slow_rate = spi_slow_rate,
    .fast_rate = spi_fast_rate,
    .read = readfromspi,
    .write = writetospi,
    .write_with_crc = writetospiwithcrc
};

static const struct spi_s spim3 =
{
    .cs_high = spi_cs_high,
    .cs_low = spi_cs_low,
    .slow_rate = spi_slow_rate,
    .fast_rate = spi_fast_rate,
    .read = readfromspi,
    .write = writetospi,
    .write_with_crc = writetospiwithcrc
};

#define DATALEN1 200
static uint8_t idatabuf[DATALEN1]={0}; //Never define this inside the Spi read/write
static uint8_t itempbuf[DATALEN1]={0}; //As that will use the stack from the Task, which are not such long!!!!
                                     //You will face a crashes which are not expected!
uint8_t spi_init_stat = 0; // use 1 for slow, use 2 for fast;

int readfromspi_uni(uint16_t headerLength,
                    const uint8_t *headerBuffer,
                    uint16_t readlength,
                    uint8_t *readBuffer,
                    spi_handle_t *pgSpiHandler);

int writetospi_uni(uint16_t headerLength,
                   const uint8_t *headerBuffer,
                   uint32_t bodylength,
                   const uint8_t *bodyBuffer,
                   spi_handle_t *pgSpiHandler);

void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context);



/* @fn      spi_slow_rate
 * @brief   set 2MHz
 *          
 * */
void spi_slow_rate(void)
{
    //set_SPI_master();
    if(spi_init_stat == 1)
    {
        return;
    }
    else 
    {
        if(spi_init_stat == 2)
        {

            nrf_drv_spi_uninit(&spi_handler.spi_inst);
        }

        spi_handler.spi_config.frequency = spi_handler.frequency_slow;

        APP_ERROR_CHECK( nrf_drv_spi_init(&spi_handler.spi_inst, 
                                          &spi_handler.spi_config, 
                                          spi_event_handler,
                                          NULL) );
        spi_init_stat = 1;

        nrf_delay_ms(2);

    }    
}

/* @fn      spi_fast_rate
 * @brief   set 16MHz
 *          
 * */
void spi_fast_rate(void)
{
    if(spi_init_stat == 2)
    {
        return;
    }
    else 
    {
        if(spi_init_stat == 1)
        {
            nrf_drv_spi_uninit(&spi_handler.spi_inst);
        }
        spi_handler.spi_config.frequency = spi_handler.frequency_fast;

        APP_ERROR_CHECK( nrf_drv_spi_init(&spi_handler.spi_inst, 
                                          &spi_handler.spi_config, 
                                          spi_event_handler,
                                          NULL) );
        // HIGH drive is mandatory when operating @ 32MHz
        nrf_gpio_cfg(spi_handler.spi_config.sck_pin,
        NRF_GPIO_PIN_DIR_OUTPUT,
        NRF_GPIO_PIN_INPUT_CONNECT,
        NRF_GPIO_PIN_NOPULL,
        NRF_GPIO_PIN_H0H1,
        NRF_GPIO_PIN_NOSENSE);
        nrf_gpio_cfg( spi_handler.spi_config.mosi_pin,
        NRF_GPIO_PIN_DIR_OUTPUT,
        NRF_GPIO_PIN_INPUT_DISCONNECT,
        NRF_GPIO_PIN_NOPULL,
        NRF_GPIO_PIN_H0H1,
        NRF_GPIO_PIN_NOSENSE);
        spi_init_stat = 2;
        nrf_delay_ms(2);
    }
}


/**
 * @brief SPI user event handler.
 * @param event
 */
void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
    spi_xfer_done = true;
}

//==============================================================================
const struct spi_s *init_spi(int spiIdx)
{
    nrf_drv_spi_t   *spi_inst;
    nrf_drv_spi_config_t  *spi_config;
    const struct spi_s  *spi_ret = NULL;
    spi_inst   = &spi_handler.spi_inst;
    spi_config = &spi_handler.spi_config;

#if NRFX_SPIM0_ENABLED == 1
    if(spiIdx == 0)
    {
        spi_inst->inst_idx = SPI0_INSTANCE_INDEX;
        spi_inst->use_easy_dma = SPI0_USE_EASY_DMA;
        spi_inst->u.spim.p_reg = NRF_SPIM0;
        spi_inst->u.spim.drv_inst_idx = NRFX_SPIM0_INST_IDX;

        spi_handler.frequency_slow = NRF_SPIM_FREQ_4M;
        spi_handler.frequency_fast = DW3000_MAX_SPI_FREQ;
        spi_ret = &spim0;
    }
#endif
#if NRFX_SPIM3_ENABLED == 1
    if(spiIdx == 3)
    {
        spi_inst->inst_idx = SPI3_INSTANCE_INDEX;
        spi_inst->use_easy_dma = SPI3_USE_EASY_DMA;
        spi_inst->u.spim.p_reg = NRF_SPIM3;
        spi_inst->u.spim.drv_inst_idx = NRFX_SPIM3_INST_IDX;

        spi_handler.frequency_slow = NRF_SPIM_FREQ_4M;
        spi_handler.frequency_fast = DW3000_MAX_SPI_FREQ;
        spi_ret = &spim3;
    }
#endif
    spi_handler.lock = DW_HAL_NODE_UNLOCKED;

    spi_config->sck_pin = DW3000_CLK_Pin;
    spi_config->mosi_pin = DW3000_MOSI_Pin;
    spi_config->miso_pin = DW3000_MISO_Pin;
    spi_config->ss_pin = DW3000_CS_Pin;
#ifdef SOFTDEVICE_PRESENT
    spi_config->irq_priority = (APP_IRQ_PRIORITY_MID - 1);
#else
    spi_config->irq_priority = (APP_IRQ_PRIORITY_MID - 2);
#endif
    spi_config->orc = 0xFF;
    spi_config->frequency = spi_handler.frequency_slow;
    spi_config->mode = NRF_DRV_SPI_MODE_0;
    spi_config->bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
    nrf_gpio_cfg_output(spi_handler.cs_pin);
    return spi_ret;
}

void close_spi(nrf_drv_spi_t *p_instance)
{

    NRF_SPIM_Type * p_spi = p_instance->u.spim.p_reg;
    nrf_spim_disable(p_spi);
}

void open_spi(nrf_drv_spi_t *p_instance)
{

    NRF_SPIM_Type * p_spi = p_instance->u.spim.p_reg;
    nrf_spim_enable(p_spi);
}

void spi_cs_low(void)
{    
    nrf_gpio_pin_clear(spi_handler.cs_pin);
}

void spi_cs_high(void)
{  
    nrf_gpio_pin_set(spi_handler.cs_pin);
}

int readfromspi(uint16_t headerLength,
                uint8_t *headerBuffer,
                uint16_t readlength,
                uint8_t *readBuffer)
{
    int ret;    

    ret = readfromspi_uni(headerLength, headerBuffer,
                            readlength, readBuffer, &spi_handler);
    return ret;
}

int writetospi( uint16_t headerLength,
                const uint8_t *headerBuffer,
                uint16_t bodylength,
                const uint8_t *bodyBuffer)
{
    int ret;    
    ret = writetospi_uni(headerLength, headerBuffer,
                          bodylength, bodyBuffer, &spi_handler);
    return ret;
}

int writetospiwithcrc(uint16_t  headerLength,
               const uint8_t    *headerBuffer,
               uint16_t         bodyLength,
               const uint8_t    *bodyBuffer,
                           uint8_t          crc8)
{
    return _ERR;
}

int readfromspi_uni(uint16_t headerLength,
                    const uint8_t *headerBuffer,
                    uint16_t readlength,
                    uint8_t *readBuffer,
                    spi_handle_t *pgSpiHandler)
{
    uint8_t * p1;
    uint32_t idatalength = headerLength + readlength;

    if ( idatalength > DATALEN1 ) {
      return NRF_ERROR_NO_MEM;
    }

    while(spi_handler.lock);

    __HAL_LOCK(pgSpiHandler);

    open_spi(&spi_handler.spi_inst);

    p1=idatabuf;
    memcpy(p1,headerBuffer, headerLength);

    p1 += headerLength;
    memset(p1,0x00,readlength);

    idatalength= headerLength + readlength;

    spi_xfer_done = false;
    nrf_drv_spi_transfer(&spi_handler.spi_inst, idatabuf, idatalength, itempbuf, idatalength);
    while(!spi_xfer_done);

    p1=itempbuf + headerLength;
    memcpy(readBuffer, p1, readlength);

    close_spi(&spi_handler.spi_inst);

    __HAL_UNLOCK(pgSpiHandler);

    return 0;
}

int writetospi_uni(uint16_t headerLength,
                   const uint8_t *headerBuffer,
                   uint32_t bodylength,
                   const uint8_t *bodyBuffer,
                   spi_handle_t *pgSpiHandler)
{
    uint8_t * p1;
    uint32_t idatalength = headerLength + bodylength;

    if ( idatalength > DATALEN1 ) {
      return NRF_ERROR_NO_MEM;
    }

    while(spi_handler.lock);

    __HAL_LOCK(pgSpiHandler);

    open_spi(&spi_handler.spi_inst);

    p1=idatabuf;
    memcpy(p1,headerBuffer, headerLength);
    p1 += headerLength;
    memcpy(p1,bodyBuffer,bodylength);

    idatalength= headerLength + bodylength;

    spi_xfer_done = false;
    nrf_drv_spi_transfer(&spi_handler.spi_inst, idatabuf, idatalength, itempbuf, idatalength);
    while(!spi_xfer_done);

    close_spi(&spi_handler.spi_inst);

     __HAL_UNLOCK(pgSpiHandler);

    return 0;
}
