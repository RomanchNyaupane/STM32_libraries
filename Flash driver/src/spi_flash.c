#include "spi_flash.h"

static SPI_HandleTypeDef* g_p_hspi;

volatile uint8_t rx_buffer, rx_buffer_2[3], rx_buffer_3[3], rx_buffer_4[4];

void flash_spi_init(SPI_HandleTypeDef *hspi_type_t){
    g_p_hspi = hspi_type_t;
}
void flash_get_mfd_id(void){
    uint8_t tx_buffer[3] = {FLASH_INSTR_MFD_ID, 0xDE, 0xDE};
    uint8_t rx_buffer;
    NSS_LOW();
    HAL_SPI_Transmit(g_p_hspi, tx_buffer, sizeof(tx_buffer), HAL_MAX_DELAY);
    HAL_SPI_Receive(g_p_hspi, &rx_buffer_2, sizeof(rx_buffer_2), HAL_MAX_DELAY);
    NSS_HIGH();
}

void flash_get_jedec_id(void){
    uint8_t tx_buffer = FLASH_INSTR_JEDEC_ID;
    uint8_t rx_buffer[3];
    NSS_LOW();
    HAL_SPI_Transmit(g_p_hspi, &tx_buffer, sizeof(tx_buffer), HAL_MAX_DELAY);
    HAL_SPI_Receive(g_p_hspi, &rx_buffer_3, sizeof(rx_buffer_3), HAL_MAX_DELAY);
    NSS_HIGH();
}

void flash_get_unique_id(void){
  uint8_t tx_buffer[5] = {FLASH_INSTR_UNIQUE_ID, 0xAB, 0xBC, 0xCD, 0xDE};
  uint8_t rx_buffer[4];
  NSS_LOW();
  HAL_SPI_Transmit(g_p_hspi, tx_buffer, sizeof(tx_buffer), HAL_MAX_DELAY);
  HAL_SPI_Receive(g_p_hspi, rx_buffer_4, sizeof(rx_buffer_4), HAL_MAX_DELAY);
  NSS_HIGH();
}

void flash_get_device_id(void){
  uint8_t tx_buffer[4] = {FLASH_INSTR_DEVICE_ID, 0xCD, 0xDE, 0xEF};
  //uint8_t rx_buffer;
  NSS_LOW();
  HAL_SPI_Transmit(g_p_hspi, tx_buffer, sizeof(tx_buffer), HAL_MAX_DELAY);
  HAL_SPI_Receive(g_p_hspi, &rx_buffer, sizeof(rx_buffer), HAL_MAX_DELAY);
  NSS_HIGH();
	
}

void flash_write_en(){
    uint8_t tx_buffer = FLASH_INSTR_WRITE_ENABLE;
    NSS_LOW();
    HAL_SPI_Transmit(&g_p_hspi, &tx_buffer, sizeof(tx_buffer), HAL_MAX_DELAY);
    NSS_HIGH();
}

void flash_write_dis(){
  uint8_t tx_buffer = FLASH_INSTR_WRITE_DISABLE;
  NSS_LOW();
  HAL_SPI_Transmit(g_p_hspi, &tx_buffer, sizeof(tx_buffer), HAL_MAX_DELAY);
  NSS_HIGH();
}

uint8_t flash_read_data(uint32_t address){
    uint8_t rx_buffer;
    NSS_LOW();
    HAL_SPI_Transmit(g_p_hspi, &address, BYTE * 3, HAL_MAX_DELAY);
    HAL_SPI_Receive(g_p_hspi, &rx_buffer, sizeof(rx_buffer), HAL_MAX_DELAY);
    NSS_HIGH();
    return rx_buffer;
}

void flash_write_data(uint32_t address, uint8_t *data){
    
}