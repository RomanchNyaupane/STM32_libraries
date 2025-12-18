#include "spi_flash.h"

static SPI_HandleTypeDef* g_p_hspi;
static volatile uint8_t flash_spi_tx_buf[8], flash_spi_rx_buf[8];

static uint8_t status = 0b00000000;
//bit_0(lsb) - byte write status. 1-failed 0-success
//bit_1 - byte read status. 1-failed 0-success

static volatile uint8_t page_number = 0x00;
static volatile uint8_t sector_number = 0x00;
static volatile uint8_t block_number = 0x00;

static uint8_t num_bytes_write = 0x00; //track no. of bytes written in batch write mode

// writes and reads are intended to be consecutive, not random
static uint8_t flash_is_addr_valid(uint8_t *block, uint8_t *sector, uint8_t *page){ 
  if(&block == block_number){
    if(&block == sector_number){
        if(&block == page_number){
          return FLASH_TRUE;
        } else{
            return FLASH_FALSE;
        }
    }else{
      return FLASH_FALSE;
    }
  }else{
    return FLASH_FALSE;
  }
}

//adjust page, sector, block address after a write or erase
static void flash_address_manager(uint8_t *block, uint8_t *sector, uint8_t *page, uint8_t command, uint8_t amount){

  switch (command)
  {
  case FLASH_ADDRESS_COMMAND_ADD:
    uint32_t address = (0x00 << 24) | (&block << 16) | (&sector << 8) | (&page);
    address = address + amount;
    block_number = address >> 16;
    sector_number = address >> 8;
    page_number = address;
  break;
  case FLASH_ADDRESS_COMMAND_ERASE:
    uint32_t address = (0x00 << 24) | (&block << 16) | (&sector << 8) | (&page);
    block_number = address >>16;
    sector_number = (address >> 8) - amount;
    page_number = 0;
  break;
  default:
    break;
  }

}


void flash_spi_init(SPI_HandleTypeDef *hspi_type_t){
    g_p_hspi = hspi_type_t;
}
void flash_get_mfd_id(void){
    flash_spi_tx_buf[0] = FLASH_INSTR_MFD_ID;
    flash_spi_tx_buf[1] = FLASH_INSTR_DUMMY_BYTE;
    flash_spi_tx_buf[2] = FLASH_INSTR_DUMMY_BYTE;
    //uint8_t tx_buffer[3] = {FLASH_INSTR_MFD_ID, 0xDE, 0xDE};
    //uint8_t rx_buffer;
    NSS_LOW();
    HAL_SPI_Transmit(g_p_hspi, flash_spi_tx_buf, FLASH_PARAM_LEN_3_BYTE, HAL_MAX_DELAY);
    HAL_SPI_Receive(g_p_hspi, flash_spi_rx_buf, FLASH_PARAM_LEN_3_BYTE, HAL_MAX_DELAY);
    NSS_HIGH();
}

void flash_get_jedec_id(void){
    //uint8_t tx_buffer = FLASH_INSTR_JEDEC_ID;
    //uint8_t rx_buffer[3];
    flash_spi_tx_buf[0] = FLASH_INSTR_JEDEC_ID;
    NSS_LOW();
    HAL_SPI_Transmit(g_p_hspi, flash_spi_tx_buf, FLASH_PARAM_LEN_1_BYTE, HAL_MAX_DELAY);
    HAL_SPI_Receive(g_p_hspi, flash_spi_rx_buf, FLASH_PARAM_LEN_3_BYTE, HAL_MAX_DELAY);
    NSS_HIGH();
}

void flash_get_unique_id(void){
  //uint8_t tx_buffer[5] = {FLASH_INSTR_UNIQUE_ID, 0xAB, 0xBC, 0xCD, 0xDE};
  //uint8_t rx_buffer[4];
  flash_spi_tx_buf[0] = FLASH_INSTR_UNIQUE_ID;
  flash_spi_tx_buf[1] = FLASH_INSTR_DUMMY_BYTE;
  flash_spi_tx_buf[2] = FLASH_INSTR_DUMMY_BYTE;
  flash_spi_tx_buf[3] = FLASH_INSTR_DUMMY_BYTE;
  flash_spi_tx_buf[4] = FLASH_INSTR_DUMMY_BYTE;
  NSS_LOW();
  HAL_SPI_Transmit(g_p_hspi, flash_spi_tx_buf, FLASH_PARAM_LEN_5_BYTE, HAL_MAX_DELAY);
  HAL_SPI_Receive(g_p_hspi, flash_spi_rx_buf, FLASH_PARAM_LEN_1_BYTE * 8, HAL_MAX_DELAY);
  NSS_HIGH();
}

void flash_get_device_id(void){
  //uint8_t tx_buffer[4] = {FLASH_INSTR_DEVICE_ID, 0xCD, 0xDE, 0xEF};
  //uint8_t rx_buffer;
  flash_spi_tx_buf[0] = FLASH_INSTR_DEVICE_ID;
  flash_spi_tx_buf[1] = FLASH_INSTR_DUMMY_BYTE;
  flash_spi_tx_buf[2] = FLASH_INSTR_DUMMY_BYTE;
  flash_spi_tx_buf[3] = FLASH_INSTR_DUMMY_BYTE;
  NSS_LOW();
  HAL_SPI_Transmit(g_p_hspi, flash_spi_tx_buf, FLASH_PARAM_LEN_4_BYTE, HAL_MAX_DELAY);
  HAL_SPI_Receive(g_p_hspi, flash_spi_rx_buf, FLASH_PARAM_LEN_1_BYTE, HAL_MAX_DELAY);
  NSS_HIGH();
}

void flash_write_en(){
    // uint8_t tx_buffer = FLASH_INSTR_WRITE_ENABLE;
    uint8_t write_en = FLASH_INSTR_WRITE_ENABLE;
    NSS_LOW();
    HAL_SPI_Transmit(g_p_hspi, &write_en, FLASH_PARAM_LEN_1_BYTE, HAL_MAX_DELAY);
    NSS_HIGH();
}

void flash_write_dis(){
    uint8_t write_dis = FLASH_INSTR_WRITE_DISABLE;
    NSS_LOW();
    HAL_SPI_Transmit(g_p_hspi, &write_dis, FLASH_PARAM_LEN_1_BYTE, HAL_MAX_DELAY);
    NSS_HIGH();
}

uint8_t flash_read_byte(uint32_t address){
    //uint8_t rx_buffer;
		//uint8_t tx_address[4];
    uint8_t page, block, sector;
    page = (address) & 0xFF;
    sector = (address >> 8) & 0xFF;
    block = (address >> 16) & 0xFF;
		flash_spi_tx_buf[0] = FLASH_INSTR_READ_DATA;
		flash_spi_tx_buf[1] = block;
		flash_spi_tx_buf[2] = sector;
		flash_spi_tx_buf[3] = page;

    if(!flash_is_addr_valid(&block, &sector, &page)){
      NSS_LOW();
      HAL_SPI_Transmit(g_p_hspi, flash_spi_tx_buf, FLASH_PARAM_LEN_4_BYTE, HAL_MAX_DELAY);
      HAL_SPI_Receive(g_p_hspi, flash_spi_rx_buf, FLASH_PARAM_LEN_1_BYTE, HAL_MAX_DELAY);
      NSS_HIGH();
      status &= 0b11111101;
    }else{
      status |= 0b00000010;
    }
    return flash_spi_rx_buf[0];
}

void flash_write_byte(uint32_t address, uint8_t data){
    //uint8_t tx_address[5];

    uint8_t page, block, sector;
    page = (address) & 0xFF;
    sector = (address >> 8) & 0xFF;
    block = (address >> 16) & 0xFF;
		flash_spi_tx_buf[0] = FLASH_INSTR_READ_DATA;
		flash_spi_tx_buf[1] = block;
		flash_spi_tx_buf[2] = sector;
		flash_spi_tx_buf[3] = page;
    flash_spi_tx_buf[4] = data;

    if(!flash_is_addr_valid(&block, &sector, &page)){
      flash_write_en();
      NSS_LOW();
      HAL_SPI_Transmit(g_p_hspi, flash_spi_tx_buf, FLASH_PARAM_LEN_5_BYTE, HAL_MAX_DELAY);
      NSS_HIGH();
      flash_write_dis();
      flash_address_manager(&block, &sector, &page, FLASH_ADDRESS_COMMAND_ADD, 1);
      status &= 0b11111110;
    } else{
      status |= 0b00000001;
    }
    //uint8_t status = flash_read_status_1();
}
//only consecutive writes are allowed. So the page, sectors and blocks are wrapped automatically as reqired.
//if we are on 53rd byte of a page and 250 bytes is to be written, the address should automatically wrap to the next sector and begin from there
uint8_t flash_write_batch(uint32_t address, uint8_t *buffer, uint16_t len){
  uint8_t i = 0;
  uint8_t page, block, sector;
  page = (address) & 0xFF;
  sector = (address >> 8) & 0xFF;
  block = (address >> 16) & 0xFF;
  flash_spi_tx_buf[0] = FLASH_INSTR_WRITE_DATA;
  flash_spi_tx_buf[1] = block;
  flash_spi_tx_buf[2] = sector;
  flash_spi_tx_buf[3] = page;

  //page alignment logic
  if(!flash_is_addr_valid(&block, &sector, &page)){
    if(len <= FLASH_PARAM_SIZE_PAGE - page_number){ //if page boundary is not crossing
      NSS_LOW();
      HAL_SPI_Transmit(g_p_hspi, flash_spi_tx_buf, FLASH_PARAM_LEN_4_BYTE, HAL_MAX_DELAY);
      HAL_SPI_Transmit(g_p_hspi, buffer, len, HAL_MAX_DELAY);
      NSS_HIGH();
      flash_address_manager(&block, &sector, &page, FLASH_ADDRESS_COMMAND_ADD, len);
    }else{  //if page boundary is crossing
      if(len <= FLASH_PARAM_SIZE_PAGE ){ //page boundary crossing but data size is less than or equal to 256 bytes
        //fill existing page
        NSS_LOW();
        HAL_SPI_Transmit(g_p_hspi, flash_spi_tx_buf, FLASH_PARAM_LEN_4_BYTE, HAL_MAX_DELAY);
        for(i = 0; i <= ((FLASH_PARAM_SIZE_PAGE - 1) - page_number); i++){
          HAL_SPI_Transmit(g_p_hspi, &buffer[i], FLASH_PARAM_LEN_1_BYTE, HAL_MAX_DELAY);
        }
        NSS_HIGH();
        //change address
        flash_spi_tx_buf[2] = flash_spi_tx_buf[2] + 1; //change sector
        flash_spi_tx_buf[3] = 0;//start from new page
        //change page
        NSS_LOW();
        HAL_SPI_Transmit(g_p_hspi, flash_spi_tx_buf, FLASH_PARAM_LEN_4_BYTE, HAL_MAX_DELAY);
        for(uint8_t j = i; j <= len - i; j++){
          HAL_SPI_Transmit(g_p_hspi, &buffer[i], FLASH_PARAM_LEN_1_BYTE, HAL_MAX_DELAY);
        }
        NSS_HIGH();
        flash_address_manager(&block, &sector, &page, FLASH_ADDRESS_COMMAND_ADD, len);
      }else{//page boundary is crossing and data size is more than 256 bytes
          //fill existing page
          NSS_LOW();
          HAL_SPI_Transmit(g_p_hspi, flash_spi_tx_buf, FLASH_PARAM_LEN_4_BYTE, HAL_MAX_DELAY);
          for(i = 0; i <= ((FLASH_PARAM_SIZE_PAGE - 1) - page_number); i++){
            HAL_SPI_Transmit(g_p_hspi, &buffer[i], FLASH_PARAM_LEN_1_BYTE, HAL_MAX_DELAY);
          }
          NSS_HIGH();
          //
          for(uint8_t j = 0; j <= ((len - i) - ((len - i) % FLASH_PARAM_SIZE_PAGE))/FLASH_PARAM_SIZE_PAGE; j++){
            NSS_LOW();
          }
      }
    }
  }
}

uint8_t flash_read_status_1(){
    // uint8_t tx_data = FLASH_INSTR_READ_STATUS_1;
    flash_spi_tx_buf[0] = FLASH_INSTR_READ_STATUS_1;
    NSS_LOW();
    HAL_SPI_Transmit(g_p_hspi, flash_spi_tx_buf, FLASH_PARAM_LEN_1_BYTE, HAL_MAX_DELAY);
    HAL_SPI_Receive(g_p_hspi, flash_spi_rx_buf, FLASH_PARAM_LEN_1_BYTE, HAL_MAX_DELAY);
    NSS_HIGH();
    return flash_spi_rx_buf[0];
}

void flash_chip_erase(void){
  flash_write_en();
  flash_spi_tx_buf[0] = FLASH_INSTR_CHIP_ERASE;
  NSS_LOW();
  HAL_SPI_Transmit(g_p_hspi, flash_spi_tx_buf, FLASH_PARAM_LEN_1_BYTE, HAL_MAX_DELAY);
  NSS_HIGH();
  flash_write_dis();
}

void flash_sector_erase(uint32_t address){
    flash_write_en();
    flash_spi_tx_buf[0] = FLASH_INSTR_SECTOR_ERASE;
    flash_spi_tx_buf[1] = (address >> 16) & 0xFF;
    flash_spi_tx_buf[2] = (address >> 8) & 0xFF;
    flash_spi_tx_buf[3] = (address) & 0xFF;
    NSS_LOW();
    HAL_SPI_Transmit(g_p_hspi, flash_spi_tx_buf, FLASH_PARAM_LEN_4_BYTE, HAL_MAX_DELAY);
    NSS_HIGH();
    flash_write_dis();
}

