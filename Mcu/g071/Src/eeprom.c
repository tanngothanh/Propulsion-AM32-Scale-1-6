/*
 * eeprom.c
 *
 *  Created on: Mar. 25, 2020
 *      Author: Alka
 *  Fixed for STM32G071 page erase & safe flash programming
 */

#include "eeprom.h"

#include <string.h>

#define page_size 0x800 // 2 kb for g071
uint32_t FLASH_FKEY1 = 0x45670123;
uint32_t FLASH_FKEY2 = 0xCDEF89AB;

void save_flash_nolib(uint8_t* data, int length, uint32_t add)
{
    uint32_t data_to_FLASH[length / 4];
    memset(data_to_FLASH, 0, sizeof(data_to_FLASH));
    for (int i = 0; i < length / 4; i++) {
        data_to_FLASH[i] = (uint32_t)data[i * 4 + 3] << 24 |
                           (uint32_t)data[i * 4 + 2] << 16 |
                           (uint32_t)data[i * 4 + 1] << 8  |
                           (uint32_t)data[i * 4];
    }
    volatile uint32_t data_length = length / 4;

    // Wait for flash not busy with timeout
    uint32_t timeout = 200000;
    while ((FLASH->SR & FLASH_SR_BSY1) != 0 && --timeout) {
    }

    // Unlock flash if locked
    if ((FLASH->CR & FLASH_CR_LOCK) != 0) {
        FLASH->KEYR = FLASH_FKEY1;
        FLASH->KEYR = FLASH_FKEY2;
    }

    // Clear all flash status / error flags before operation
    FLASH->SR = 0x0000FFFF;

    // Erase page if address is 2048-byte aligned
    if ((add % 2048) == 0) {
        uint32_t page = ((add - 0x08000000U) / 2048U) & 0x3FU;
        FLASH->CR &= ~FLASH_CR_PNB_Msk;
        FLASH->CR |= FLASH_CR_PER | (page << FLASH_CR_PNB_Pos);
        FLASH->CR |= FLASH_CR_STRT;

        timeout = 200000;
        while ((FLASH->SR & FLASH_SR_BSY1) != 0 && --timeout) {
        }
        if ((FLASH->SR & FLASH_SR_EOP) != 0) {
            FLASH->SR = FLASH_SR_EOP;
        }
        FLASH->CR &= ~FLASH_CR_PER;
        FLASH->SR = 0x0000FFFF;
    }

    // Program double-words (64 bits each)
    volatile uint32_t write_cnt = 0, index = 0;
    while (index < data_length) {
        FLASH->CR |= FLASH_CR_PG;
        *(__IO uint32_t*)(add + write_cnt) = data_to_FLASH[index];
        *(__IO uint32_t*)(add + write_cnt + 4) = data_to_FLASH[index + 1];

        timeout = 100000;
        while ((FLASH->SR & FLASH_SR_BSY1) != 0 && --timeout) {
        }
        if ((FLASH->SR & FLASH_SR_EOP) != 0) {
            FLASH->SR = FLASH_SR_EOP;
        }
        FLASH->CR &= ~FLASH_CR_PG;
        write_cnt += 8;
        index += 2;
    }

    // Lock flash
    SET_BIT(FLASH->CR, FLASH_CR_LOCK);
}

void read_flash_bin(uint8_t* data, uint32_t add, int out_buff_len)
{
    for (int i = 0; i < out_buff_len; i++) {
        data[i] = *(uint8_t*)(add + i);
    }
}
