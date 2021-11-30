/*
 * ext_flash_driver.h
 *
 *  Created on: Nov 29, 2021
 *      Author: karol
 */

#ifndef CORE_INC_EXT_FLASH_DRIVER_H_
#define CORE_INC_EXT_FLASH_DRIVER_H_

#include "main.h"
#include "quadspi.h"

//----------------------------------------------------------------------

/*
 * @brief   Read Status Register command
 */
extern QSPI_CommandTypeDef cmd_RDSR;

/*
 * @brief  Read Unique ID Number command
 */
extern QSPI_CommandTypeDef cmd_RDUID;

/*
 * @brief  Write Enable command
 */
extern QSPI_CommandTypeDef cmd_WREN;

/*
 * @brief  Write Status Register command
 */
extern QSPI_CommandTypeDef cmd_WRSR;

/*
 * @brief  Quad Input Page Program command
 */
extern QSPI_CommandTypeDef cmd_PPQ;

/*
 * @brief  Fast Read Quad I/O command
 */
extern QSPI_CommandTypeDef cmd_FRQIO;

/*
 * @brief  Read Function Register command
 */
extern QSPI_CommandTypeDef cmd_RDFR;

/*
 * @brief  Chip Erase command
 */
extern QSPI_CommandTypeDef cmd_CER;

//----------------------------------------------------------------------

/*
 * @brief   Save wavetable in external flash memory.
 * @param   *ptr - pointer to array with samples of wavetable
 * @param   len - defines length of array (in half word)
 * @param   address - determines where function will start saving data (in half word), this value should be multiplication of 128 (?)
 */
void flash_save_wavetable(const int16_t *ptr, uint32_t len, uint32_t address);

//----------------------------------------------------------------------

/*
 * @brief   Read wavetable from external flash memory.
 * @param   *ptr - pointer to array where samples will be save
 * @param   len - defines length of array (in half word)
 * @param   address - determines where function will start read data (in half word)
 */
void flash_read_wavetable(int16_t *ptr, uint32_t len, uint32_t address);


#endif /* CORE_INC_EXT_FLASH_DRIVER_H_ */
