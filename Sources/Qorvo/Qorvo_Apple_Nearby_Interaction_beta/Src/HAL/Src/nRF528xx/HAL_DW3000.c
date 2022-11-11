/**
 * @file    HAL_DW3000.c
 * 
 * @brief   Hardware abstaction layer for DW30000
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#include <stdint.h>
#include "nrf_drv_gpiote.h"
#include "boards.h"
#include "deca_device_api.h"
#include "deca_interface.h"
#include "HAL_error.h"
#include "HAL_DW3000.h"
#include "HAL_SPI.h"
#include "nrf_delay.h"
#include "sdk_config.h"
#include "cmsis_os.h"

struct dw_s
{
    uint16_t        irqPin;
    IRQn_Type       irqN;
    uint16_t        rstPin;
    uint16_t        wakeUpPin;
    IRQn_Type       rstIrqN;
};

typedef struct dw_s dw_t;

const dw_t dw_chip =
{
    .irqPin    = DW3000_IRQ_Pin,
    .irqN      = GPIOTE_IRQn,  /* NRF chip has only 1 IRQ for all GPIO */
    .rstPin    = DW3000_RST_Pin,
    .rstIrqN   = GPIOTE_IRQn,  /* NRF chip has only 1 IRQ for all GPIO */
    .wakeUpPin = DW3000_WUP_Pin,
};

const  dw_t *pDwChip = &dw_chip;
static const struct spi_s *spi;

static int port_init_device(void);

void disable_dw3000_irq(void)
{
    /* Unfortunately NRF chip has only 1 IRQ for all GPIO pins */
    nrf_drv_gpiote_in_event_disable(pDwChip->irqPin);
}

void enable_dw3000_irq(void)
{
    nrf_drv_gpiote_in_event_enable(pDwChip->irqPin, true);
}


/* @fn      reset_DW3000
 * @brief   DW_RESET pin on DW1000 has 2 functions
 *          In general it is output, but it also can be used to reset the
 *          digital part of DW1000 by driving this pin low.
 *          Note, the DW_RESET pin should not be driven high externally.
 * */
void reset_DW3000(void)
{
    nrf_gpio_cfg_output(DW3000_RST_Pin);
    nrf_gpio_pin_clear(DW3000_RST_Pin);
    nrf_delay_us(500);
    nrf_gpio_cfg_input(DW3000_RST_Pin, NRF_GPIO_PIN_NOPULL);
    nrf_delay_us(1500);
}



/* @fn      port_wakeup_dw3000_fast
 * @brief   waking up of DW3000 using DW_CS pin
 *
 *          the fast wakeup takes ~1ms:
 *          500us to hold the CS  - TODO: this time can be reduced
 *          500us to the crystal to startup
 *          + ~various time 100us...10ms
 * */
error_e port_wakeup_dw3000_fast(void)
{
    spi->cs_low();
    nrf_delay_us(500);
    spi->cs_high();

    return _NO_ERR;
}

/* @fn      port_wakeup_dw3000_deterministic
 * @brief   waking up of DW3000 using DW_CS pin
 *
 * */
error_e port_wakeup_dw3000_deterministic(void)
{
    spi->cs_low();
    nrf_delay_us(500);
    spi->cs_high();
    nrf_delay_us(1500);

    return _NO_ERR;
}

void wakeup_device_with_io(void)
{
    port_wakeup_dw3000_deterministic();
}

void port_wakeup_dw3000_nowait(void)
{
    spi->cs_low();
    nrf_delay_us(500);
    spi->cs_high();
}

void port_disable_wake_init_dw(void)
{
    taskENTER_CRITICAL();         

    disable_dw3000_irq();             /**< disable NVIC IRQ until we configure the device */

    //this is called here to wake up the device (i.e. if it was in sleep mode before the restart)
    port_wakeup_dw3000_deterministic();  

//    if(port_init_device() != 0x00)
//    {
//        error_handler(1,  _ERR_INIT);
//    }

    taskEXIT_CRITICAL();
}

void port_reinit_dw_chips(void)
{
    nrf_gpio_pin_clear(DW3000_RST_Pin);
    nrf_delay_ms(1);
    nrf_gpio_cfg_input(DW3000_RST_Pin, NRF_GPIO_PIN_NOPULL);
    nrf_delay_ms(2);
}

void port_init_dw_chip(int spiIdx)
{
    dwt_setlocaldataptr(0);

    spi = init_spi(spiIdx);
    if(spi == NULL)
    {
        error_handler(1,_ERR_SPI);
    }
    spi->slow_rate();   
}

/**
 *  @brief     Bare-metal level
 *          initialise master/slave DW1000 (check if can talk to device and wake up and reset)
 */
int port_init_device(void)
{

    spi->slow_rate();

    if (dwt_check_dev_id()!=DWT_SUCCESS)
    {
        return DWT_ERROR;
    }

    //clear the sleep bit in case it is set - so that after the hard reset below the DW does not go into sleep
    dwt_softreset(0);

    return 0;
}

void port_stop_all_UWB(void)
{
    port_disable_dw_irq_and_reset(1);
    dwt_setcallbacks(NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

/*! ----------------------------------------------------------------------------
 * Function: decamutexon()
 *
 * Description: This function should disable interrupts.
 *
 *
 * input parameters: void
 *
 * output parameters: uint16
 * returns the state of the DW1000 interrupt
 */
decaIrqStatus_t decamutexon(void)
{
/* NRF chip has only 1 IRQ for all GPIO pins.
 * Disablin of the NVIC would not be of the best ideas.
 */
    uint32_t s = nrf_drv_gpiote_in_is_set(pDwChip->irqPin);
    if(s)
    {
        nrf_drv_gpiote_in_event_disable(pDwChip->irqPin);
    }
    return 0;
}
/*! ----------------------------------------------------------------------------
 * Function: decamutexoff()
 *
 * Description: This function should re-enable interrupts, or at least restore
 *              their state as returned(&saved) by decamutexon
 * This is called at the end of a critical section
 *
 * input parameters:
 * @param s - the state of the DW1000 interrupt as returned by decamutexon
 *
 * output parameters
 *
 * returns the state of the DW1000 interrupt
 */
void decamutexoff(decaIrqStatus_t s)
{
    if(s)
    {
        nrf_drv_gpiote_in_event_enable(pDwChip->irqPin, true);
    }
}


/* @fn      port_CheckEXT_IRQ
 * @brief   wrapper to read DW_IRQ input pin state
 * */
uint32_t port_CheckEXT_IRQ(void)
{
    return nrf_gpio_pin_read(pDwChip->irqPin);
}

/* @fn      process_deca_irq
 * @brief   main call-back for processing of DW3000 IRQ
 *          it re-enters the IRQ routing and processes all events.
 *          After processing of all events, DW3000 will clear the IRQ line.
 * */
void process_deca_irq(void)
{
    while(port_CheckEXT_IRQ() != 0)
    {
         dwt_isr();

    } //while DW3000 IRQ line active

    if (port_get_CanSleepInIRQ() == DW_CAN_SLEEP)
    {
        port_set_DwEnterSleep(DW_IS_SLEEPING_IRQ);
        dwt_entersleep(DWT_DW_IDLE_RC);   //manual sleep after successful Final TX or RX timeout
        port_set_DwSpiReady(DW_SPI_SLEEPING);
    }
}

/*
 * @brief disable DW_IRQ, reset DW3000
 *        and set
 *        app.DwCanSleep = DW_CANNOT_SLEEP;
 *        app.DwEnterSleep = DW_NOT_SLEEPING;
 * */
error_e port_disable_dw_irq_and_reset(int reset)
{
    taskENTER_CRITICAL();

    disable_dw3000_irq(); /**< disable NVIC IRQ until we configure the device */

    //this is called to reset the DW device
    if (reset)
    {
        reset_DW3000();
    }

    port_set_CanSleepInIRQ(DW_CANNOT_SLEEP);
    port_set_DwEnterSleep(DW_NOT_SLEEPING);

    taskEXIT_CRITICAL();

    return _NO_ERR;
}


static void deca_irq_handler(nrf_drv_gpiote_pin_t irqPin, nrf_gpiote_polarity_t irq_action)
{
    process_deca_irq();
}

void init_dw3000_irq(void)
{
    ret_code_t err_code;

    if(!nrf_drv_gpiote_is_init())
    {
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    in_config.pull = NRF_GPIO_PIN_PULLDOWN;

    err_code = nrf_drv_gpiote_in_init(DW3000_IRQ_Pin, &in_config, deca_irq_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(DW3000_IRQ_Pin, false);
    }
}

DwCanSleepInIRQ dwCanSleepInIRQ;
void port_set_CanSleepInIRQ(DwCanSleepInIRQ val)
{
    dwCanSleepInIRQ = val;
}
DwCanSleepInIRQ port_get_CanSleepInIRQ(void)
{
    return  dwCanSleepInIRQ;
}

DwEnterSleep dwEnterSleep;
void port_set_DwEnterSleep(DwEnterSleep val)
{
    dwEnterSleep = val;
}
DwEnterSleep port_get_DwEnterSleep(void)
{
    return  dwEnterSleep;
}
DwSpiReady dwSpiReady;
void port_set_DwSpiReady(DwSpiReady val)
{
    dwSpiReady = val;
}
DwSpiReady port_get_DwSpiReady(void)
{
    return  dwSpiReady;
}

void set_dw_spi_slow_rate(void)
{
    spi->slow_rate();
}

void set_dw_spi_fast_rate(void)
{
    spi->fast_rate();
}

int readfromspi(uint16_t headerLength, uint8_t *headerBuffer, uint16_t readlength, uint8_t *readBuffer);
int writetospi(uint16_t headerLength, const uint8_t *headerBuffer, uint16_t readlength, const uint8_t *readBuffer); 
int writetospiwithcrc(uint16_t  headerLength, const uint8_t *headerBuffer, uint16_t  bodyLength, const uint8_t *bodyBuffer,  uint8_t crc8);
                
static const struct dwt_spi_s dw3000_spi_fct = {
    .readfromspi = readfromspi,
    .writetospi = writetospi,
    .writetospiwithcrc = writetospiwithcrc,
    .setfastrate = set_dw_spi_slow_rate,
    .setfastrate = set_dw_spi_fast_rate
};

const struct dwt_probe_s dw3000_probe_interf = 
{
    .dw = NULL,
    .spi = (void*)&dw3000_spi_fct,
    .wakeup_device_with_io = wakeup_device_with_io
};
