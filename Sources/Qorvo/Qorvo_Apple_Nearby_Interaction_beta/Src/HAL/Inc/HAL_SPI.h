/**
 * @file    HAL_SPI.h
 *
 * @brief   Header for HAL_SPI
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#ifndef HAL_SPI_H
#define HAL_SPI_H
#include <stdint.h>

const struct spi_s *init_spi(int spiIdx);

struct spi_s
{
    void (*cs_low)(void);
    void (*cs_high)(void);
    void (*slow_rate)(void);
    void (*fast_rate)(void);
    int (*read)(uint16_t headerLength, uint8_t *headerBuffer, 
                uint16_t readlength,  uint8_t *readBuffer);
    int (*write)(uint16_t headerLength, const uint8_t *headerBuffer, 
                 uint16_t readlength,  const uint8_t *readBuffer);
    int (*write_with_crc)(uint16_t  headerLength, const uint8_t *headerBuffer,
                          uint16_t bodyLength,  const uint8_t *bodyBuffer,
                          uint8_t crc8);
};
#endif
