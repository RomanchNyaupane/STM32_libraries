#ifndef SPI_FLASH_LIB_H
#define SPI_FLASH_LIB_H
#include "stm32wbxx_hal.h"
#include "stm32wbxx_hal_spi.h"

#define FLASH_INSTR_MFD_ID 0x90
#define FLASH_INSTR_DEVICE_ID 0xAB
#define FLASH_INSTR_JEDEC_ID 0x9F
#define FLASH_INSTR_UNIQUE_ID 0x4B

#define FLASH_INSTR_WRITE_ENABLE 0x06
#define FLASH_INSTR_WRITE_DISABLE 0x04

#define BYTE 1

#define FLASH_INSTR_READ_DATA 0x03

#define NSS_LOW() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET)
#define NSS_HIGH() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET)

typedef SPI_HandleTypeDef hspi_type_t;

//uint8_t device_id;

void flash_spi_init(SPI_HandleTypeDef*);
void flash_get_mfd_id(void); //manufacture id
void flash_get_jedec_id(void);
void flash_get_unique_id(void);
void flash_get_device_id(void);

void flash_write_en(void);
void flash_write_dis(void);

uint8_t flash_read_data(uint32_t);

void flash_write_data(uint32_t, uint8_t*);

#endif /*SPI_FLASH_LIB_H*/