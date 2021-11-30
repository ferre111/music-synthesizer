/*
 * ext_flash_driver.c
 *
 *  Created on: Feb 21, 2021
 *      Author: Karol Witusik
 */


#include "ext_flash_driver.h"
#include "wavetable.h"

//----------------------------------------------------------------------

#define TIMEOUT 1000
#define MAX_DATA_TO_SAVE_PPQ 256

//----------------------------------------------------------------------

static void check_memory_busy(void);
static void enable_quad_write(void);

//----------------------------------------------------------------------

/** @brief   Commands for IS25LQ010B FLASH memory.
 */

/*
 *      .Instruction        Specifies the Instruction to be sent
 *                          This parameter can be a value (8-bit) between 0x00 and 0xFF
 *
 *      .Address            Specifies the Address to be sent (Size from 1 to 4 bytes according AddressSize)
 *                          This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF
 *
 *      .AlternateBytes     Specifies the Alternate Bytes to be sent (Size from 1 to 4 bytes according AlternateBytesSize)
 *                          This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF
 *
 *      .AddressSize        Specifies the Address Size
 *                          This parameter can be a value of @ref QSPI_AddressSize
 *
 *      .AlternateBytesSize Specifies the Alternate Bytes Size
 *                          This parameter can be a value of @ref QSPI_AlternateBytesSize
 *
 *      .DummyCycles        Specifies the Number of Dummy Cycles.
 *                          This parameter can be a number between 0 and 31
 *
 *      .InstructionMode    Specifies the Instruction Mode
 *                          This parameter can be a value of @ref QSPI_InstructionMode
 *
 *      .AddressMode        Specifies the Address Mode
 *                          This parameter can be a value of @ref QSPI_AddressMode
 *
 *      .AlternateByteMode  Specifies the Alternate Bytes Mode
 *                          This parameter can be a value of @ref QSPI_AlternateBytesMode
 *
 *      .DataMode           Specifies the Data Mode (used for dummy cycles and data phases)
 *                          This parameter can be a value of @ref QSPI_DataMode
 *
 *      .NbData             Specifies the number of data to transfer. (This is the number of bytes)
 *                          This parameter can be any value between 0 and 0xFFFFFFFF (0 means undefined length
 *                          until end of memory)
 *
 *      .DdrMode            Specifies the double data rate mode for address, alternate byte and data phase
 *                          This parameter can be a value of @ref QSPI_DdrMode
 *
 *      .DdrHoldHalfCycle   Specifies if the DDR hold is enabled. When enabled it delays the data
 *                          output by one half of system clock in DDR mode.
 *                          This parameter can be a value of @ref QSPI_DdrHoldHalfCycle
 *
 *      .SIOOMode           Specifies the send instruction only once mode
 *                          This parameter can be a value of @ref QSPI_SIOOMode
 */

//----------------------------------------------------------------------

QSPI_CommandTypeDef cmd_RDSR = {
    .Instruction = 0x05,
    .Address = 0,
    .AlternateBytes = 0,
    .AddressSize = 0,
    .AlternateBytesSize = 0,
    .DummyCycles = 0,
    .InstructionMode = QSPI_INSTRUCTION_1_LINE,
    .AddressMode = QSPI_ADDRESS_NONE,
    .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
    .DataMode = QSPI_DATA_1_LINE,
    .NbData = 1,
    .DdrMode = QSPI_DDR_MODE_DISABLE,
    .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,
    .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
};

QSPI_CommandTypeDef cmd_RDFR = {
    .Instruction = 0x48,                              /* Specifies the Instruction to be sent
                                                      This parameter can be a value (8-bit) between 0x00 and 0xFF */
    .Address = 0,                                     /* Specifies the Address to be sent (Size from 1 to 4 bytes according AddressSize)
                                                      This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF */
    .AlternateBytes = 0,                              /* Specifies the Alternate Bytes to be sent (Size from 1 to 4 bytes according AlternateBytesSize)
                                                      This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF */
    .AddressSize = 0,                                 /* Specifies the Address Size
                                                      This parameter can be a value of @ref QSPI_AddressSize */
    .AlternateBytesSize = 0,                          /* Specifies the Alternate Bytes Size
                                                      This parameter can be a value of @ref QSPI_AlternateBytesSize */
    .DummyCycles = 0,                                 /* Specifies the Number of Dummy Cycles. //todo
                                                      This parameter can be a number between 0 and 31 */
    .InstructionMode = QSPI_INSTRUCTION_1_LINE,       /* Specifies the Instruction Mode
                                                      This parameter can be a value of @ref QSPI_InstructionMode */
    .AddressMode = QSPI_ADDRESS_NONE,                 /* Specifies the Address Mode
                                                      This parameter can be a value of @ref QSPI_AddressMode */
    .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,   /* Specifies the Alternate Bytes Mode
                                                      This parameter can be a value of @ref QSPI_AlternateBytesMode */
    .DataMode = QSPI_DATA_1_LINE,                     /* Specifies the Data Mode (used for dummy cycles and data phases)
                                                      This parameter can be a value of @ref QSPI_DataMode */
    .NbData = 1,                                      /* Specifies the number of data to transfer. (This is the number of bytes)
                                                      This parameter can be any value between 0 and 0xFFFFFFFF (0 means undefined length
                                                      until end of memory)*/
    .DdrMode = QSPI_DDR_MODE_DISABLE,                 /* Specifies the double data rate mode for address, alternate byte and data phase
                                                      This parameter can be a value of @ref QSPI_DdrMode */
    .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,    /* Specifies if the DDR hold is enabled. When enabled it delays the data
                                                      output by one half of system clock in DDR mode.
                                                      This parameter can be a value of @ref QSPI_DdrHoldHalfCycle */
    .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,             /* Specifies the send instruction only once mode
                                                      This parameter can be a value of @ref QSPI_SIOOMode */
};

QSPI_CommandTypeDef cmd_RDUID = {
    .Instruction = 0x4B,                              /* Specifies the Instruction to be sent
                                                      This parameter can be a value (8-bit) between 0x00 and 0xFF */
    .Address = 0,                                     /* Specifies the Address to be sent (Size from 1 to 4 bytes according AddressSize)
                                                      This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF */
    .AlternateBytes = 0,                              /* Specifies the Alternate Bytes to be sent (Size from 1 to 4 bytes according AlternateBytesSize)
                                                      This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF */
    .AddressSize = QSPI_ADDRESS_24_BITS,              /* Specifies the Address Size
                                                      This parameter can be a value of @ref QSPI_AddressSize */
    .AlternateBytesSize = 0,                          /* Specifies the Alternate Bytes Size
                                                      This parameter can be a value of @ref QSPI_AlternateBytesSize */
    .DummyCycles = 1,                                 /* Specifies the Number of Dummy Cycles. //todo
                                                      This parameter can be a number between 0 and 31 */
    .InstructionMode = QSPI_INSTRUCTION_1_LINE,       /* Specifies the Instruction Mode
                                                      This parameter can be a value of @ref QSPI_InstructionMode */
    .AddressMode = QSPI_ADDRESS_1_LINE,               /* Specifies the Address Mode
                                                      This parameter can be a value of @ref QSPI_AddressMode */
    .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,   /* Specifies the Alternate Bytes Mode
                                                      This parameter can be a value of @ref QSPI_AlternateBytesMode */
    .DataMode = QSPI_DATA_1_LINE,                     /* Specifies the Data Mode (used for dummy cycles and data phases)
                                                      This parameter can be a value of @ref QSPI_DataMode */
    .NbData = 2,                                      /* Specifies the number of data to transfer. (This is the number of bytes)
                                                      This parameter can be any value between 0 and 0xFFFFFFFF (0 means undefined length
                                                      until end of memory)*/
    .DdrMode = QSPI_DDR_MODE_DISABLE,                 /* Specifies the double data rate mode for address, alternate byte and data phase
                                                      This parameter can be a value of @ref QSPI_DdrMode */
    .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,    /* Specifies if the DDR hold is enabled. When enabled it delays the data
                                                      output by one half of system clock in DDR mode.
                                                      This parameter can be a value of @ref QSPI_DdrHoldHalfCycle */
    .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,             /* Specifies the send instruction only once mode
                                                      This parameter can be a value of @ref QSPI_SIOOMode */
};

QSPI_CommandTypeDef cmd_WREN = {
    .Instruction = 0x06,         /* Specifies the Instruction to be sent
                            This parameter can be a value (8-bit) between 0x00 and 0xFF */
    .Address = 0,            /* Specifies the Address to be sent (Size from 1 to 4 bytes according AddressSize)
                            This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF */
    .AlternateBytes = 0,     /* Specifies the Alternate Bytes to be sent (Size from 1 to 4 bytes according AlternateBytesSize)
                            This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF */
    .AddressSize = 0,       /* Specifies the Address Size
                            This parameter can be a value of @ref QSPI_AddressSize */
    .AlternateBytesSize = 0, /* Specifies the Alternate Bytes Size
                            This parameter can be a value of @ref QSPI_AlternateBytesSize */
    .DummyCycles = 0,        /* Specifies the Number of Dummy Cycles.
                            This parameter can be a number between 0 and 31 */
    .InstructionMode = QSPI_INSTRUCTION_1_LINE,   /* Specifies the Instruction Mode
                            This parameter can be a value of @ref QSPI_InstructionMode */
    .AddressMode = QSPI_ADDRESS_NONE,       /* Specifies the Address Mode
                            This parameter can be a value of @ref QSPI_AddressMode */
    .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,  /* Specifies the Alternate Bytes Mode
                            This parameter can be a value of @ref QSPI_AlternateBytesMode */
    .DataMode = QSPI_DATA_NONE,           /* Specifies the Data Mode (used for dummy cycles and data phases)
                            This parameter can be a value of @ref QSPI_DataMode */
    .NbData = 0,             /* Specifies the number of data to transfer. (This is the number of bytes)
                            This parameter can be any value between 0 and 0xFFFFFFFF (0 means undefined length
                            until end of memory)*/
    .DdrMode = QSPI_DDR_MODE_DISABLE,            /* Specifies the double data rate mode for address, alternate byte and data phase
                            This parameter can be a value of @ref QSPI_DdrMode */
    .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,   /* Specifies if the DDR hold is enabled. When enabled it delays the data
                            output by one half of system clock in DDR mode.
                            This parameter can be a value of @ref QSPI_DdrHoldHalfCycle */
    .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,           /* Specifies the send instruction only once mode
                                    This parameter can be a value of @ref QSPI_SIOOMode */
};

QSPI_CommandTypeDef cmd_WRSR = {
    .Instruction = 0x01,                              /* Specifies the Instruction to be sent
                                                      This parameter can be a value (8-bit) between 0x00 and 0xFF */
    .Address = 0,                                     /* Specifies the Address to be sent (Size from 1 to 4 bytes according AddressSize)
                                                      This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF */
    .AlternateBytes = 0,                              /* Specifies the Alternate Bytes to be sent (Size from 1 to 4 bytes according AlternateBytesSize)
                                                      This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF */
    .AddressSize = QSPI_ADDRESS_24_BITS,              /* Specifies the Address Size
                                                      This parameter can be a value of @ref QSPI_AddressSize */
    .AlternateBytesSize = 0,                          /* Specifies the Alternate Bytes Size
                                                      This parameter can be a value of @ref QSPI_AlternateBytesSize */
    .DummyCycles = 0,                                 /* Specifies the Number of Dummy Cycles. //todo
                                                      This parameter can be a number between 0 and 31 */
    .InstructionMode = QSPI_INSTRUCTION_1_LINE,       /* Specifies the Instruction Mode
                                                      This parameter can be a value of @ref QSPI_InstructionMode */
    .AddressMode = QSPI_ADDRESS_NONE,               /* Specifies the Address Mode
                                                      This parameter can be a value of @ref QSPI_AddressMode */
    .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,   /* Specifies the Alternate Bytes Mode
                                                      This parameter can be a value of @ref QSPI_AlternateBytesMode */
    .DataMode = QSPI_DATA_1_LINE,                     /* Specifies the Data Mode (used for dummy cycles and data phases)
                                                      This parameter can be a value of @ref QSPI_DataMode */
    .NbData = 1,                                      /* Specifies the number of data to transfer. (This is the number of bytes)
                                                      This parameter can be any value between 0 and 0xFFFFFFFF (0 means undefined length
                                                      until end of memory)*/
    .DdrMode = QSPI_DDR_MODE_DISABLE,                 /* Specifies the double data rate mode for address, alternate byte and data phase
                                                      This parameter can be a value of @ref QSPI_DdrMode */
    .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,    /* Specifies if the DDR hold is enabled. When enabled it delays the data
                                                      output by one half of system clock in DDR mode.
                                                      This parameter can be a value of @ref QSPI_DdrHoldHalfCycle */
    .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,             /* Specifies the send instruction only once mode
                                                      This parameter can be a value of @ref QSPI_SIOOMode */
};


QSPI_CommandTypeDef cmd_PPQ = {
    .Instruction = 0x32,         /* Specifies the Instruction to be sent
                          This parameter can be a value (8-bit) between 0x00 and 0xFF */
    .Address = 0,            /* Specifies the Address to be sent (Size from 1 to 4 bytes according AddressSize)
                          This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF */
    .AlternateBytes = 0,     /* Specifies the Alternate Bytes to be sent (Size from 1 to 4 bytes according AlternateBytesSize)
                          This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF */
    .AddressSize = QSPI_ADDRESS_24_BITS,       /* Specifies the Address Size
                          This parameter can be a value of @ref QSPI_AddressSize */
    .AlternateBytesSize = 0, /* Specifies the Alternate Bytes Size
                          This parameter can be a value of @ref QSPI_AlternateBytesSize */
    .DummyCycles = 0,        /* Specifies the Number of Dummy Cycles.
                          This parameter can be a number between 0 and 31 */
    .InstructionMode = QSPI_INSTRUCTION_1_LINE,   /* Specifies the Instruction Mode
                          This parameter can be a value of @ref QSPI_InstructionMode */
    .AddressMode = QSPI_ADDRESS_1_LINE,       /* Specifies the Address Mode
                          This parameter can be a value of @ref QSPI_AddressMode */
    .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,  /* Specifies the Alternate Bytes Mode
                          This parameter can be a value of @ref QSPI_AlternateBytesMode */
    .DataMode = QSPI_DATA_4_LINES,           /* Specifies the Data Mode (used for dummy cycles and data phases)
                          This parameter can be a value of @ref QSPI_DataMode */
    .NbData = 2,             /* Specifies the number of data to transfer. (This is the number of bytes)
                          This parameter can be any value between 0 and 0xFFFFFFFF (0 means undefined length
                          until end of memory)*/
    .DdrMode = QSPI_DDR_MODE_DISABLE,            /* Specifies the double data rate mode for address, alternate byte and data phase
                          This parameter can be a value of @ref QSPI_DdrMode */
    .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,   /* Specifies if the DDR hold is enabled. When enabled it delays the data
                          output by one half of system clock in DDR mode.
                          This parameter can be a value of @ref QSPI_DdrHoldHalfCycle */
    .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,           /* Specifies the send instruction only once mode
                                  This parameter can be a value of @ref QSPI_SIOOMode */
};

QSPI_CommandTypeDef cmd_FRQIO = {
    .Instruction = 0xEB,                              /* Specifies the Instruction to be sent
                                                      This parameter can be a value (8-bit) between 0x00 and 0xFF */
    .Address = 0,                                     /* Specifies the Address to be sent (Size from 1 to 4 bytes according AddressSize)
                                                      This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF */
    .AlternateBytes = 0x00,                           /* Specifies the Alternate Bytes to be sent (Size from 1 to 4 bytes according AlternateBytesSize)
                                                      This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF */
    .AddressSize = QSPI_ADDRESS_24_BITS,              /* Specifies the Address Size
                                                      This parameter can be a value of @ref QSPI_AddressSize */
    .AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS,/* Specifies the Alternate Bytes Size
                                                      This parameter can be a value of @ref QSPI_AlternateBytesSize */
    .DummyCycles = 4,                                 /* Specifies the Number of Dummy Cycles. //todo
                                                      This parameter can be a number between 0 and 31 */
    .InstructionMode = QSPI_INSTRUCTION_1_LINE,       /* Specifies the Instruction Mode
                                                      This parameter can be a value of @ref QSPI_InstructionMode */
    .AddressMode = QSPI_ADDRESS_4_LINES,               /* Specifies the Address Mode
                                                      This parameter can be a value of @ref QSPI_AddressMode */
    .AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES,   /* Specifies the Alternate Bytes Mode
                                                      This parameter can be a value of @ref QSPI_AlternateBytesMode */
    .DataMode = QSPI_DATA_4_LINES,                     /* Specifies the Data Mode (used for dummy cycles and data phases)
                                                      This parameter can be a value of @ref QSPI_DataMode */
    .NbData = SAMPLE_COUNT * 2,                                      /* Specifies the number of data to transfer. (This is the number of bytes)
                                                      This parameter can be any value between 0 and 0xFFFFFFFF (0 means undefined length
                                                      until end of memory)*/
    .DdrMode = QSPI_DDR_MODE_DISABLE,                 /* Specifies the double data rate mode for address, alternate byte and data phase
                                                      This parameter can be a value of @ref QSPI_DdrMode */
    .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,    /* Specifies if the DDR hold is enabled. When enabled it delays the data
                                                      output by one half of system clock in DDR mode.
                                                      This parameter can be a value of @ref QSPI_DdrHoldHalfCycle */
    .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,        /* Specifies the send instruction only once mode
                                                      This parameter can be a value of @ref QSPI_SIOOMode */
};

QSPI_CommandTypeDef cmd_CER = {
    .Instruction = 0xC7,                              /* Specifies the Instruction to be sent
                                                      This parameter can be a value (8-bit) between 0x00 and 0xFF */
    .Address = 0,                                     /* Specifies the Address to be sent (Size from 1 to 4 bytes according AddressSize)
                                                      This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF */
    .AlternateBytes = 0x00,                           /* Specifies the Alternate Bytes to be sent (Size from 1 to 4 bytes according AlternateBytesSize)
                                                      This parameter can be a value (32-bits) between 0x0 and 0xFFFFFFFF */
    .AddressSize = 0,              /* Specifies the Address Size
                                                      This parameter can be a value of @ref QSPI_AddressSize */
    .AlternateBytesSize = 0,/* Specifies the Alternate Bytes Size
                                                      This parameter can be a value of @ref QSPI_AlternateBytesSize */
    .DummyCycles = 0,                                 /* Specifies the Number of Dummy Cycles. //todo
                                                      This parameter can be a number between 0 and 31 */
    .InstructionMode = QSPI_INSTRUCTION_1_LINE,       /* Specifies the Instruction Mode
                                                      This parameter can be a value of @ref QSPI_InstructionMode */
    .AddressMode = QSPI_ADDRESS_NONE,               /* Specifies the Address Mode
                                                      This parameter can be a value of @ref QSPI_AddressMode */
    .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,   /* Specifies the Alternate Bytes Mode
                                                      This parameter can be a value of @ref QSPI_AlternateBytesMode */
    .DataMode = QSPI_DATA_NONE,                     /* Specifies the Data Mode (used for dummy cycles and data phases)
                                                      This parameter can be a value of @ref QSPI_DataMode */
    .NbData = 0,                                      /* Specifies the number of data to transfer. (This is the number of bytes)
                                                      This parameter can be any value between 0 and 0xFFFFFFFF (0 means undefined length
                                                      until end of memory)*/
    .DdrMode = QSPI_DDR_MODE_DISABLE,                 /* Specifies the double data rate mode for address, alternate byte and data phase
                                                      This parameter can be a value of @ref QSPI_DdrMode */
    .DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY,    /* Specifies if the DDR hold is enabled. When enabled it delays the data
                                                      output by one half of system clock in DDR mode.
                                                      This parameter can be a value of @ref QSPI_DdrHoldHalfCycle */
    .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,        /* Specifies the send instruction only once mode
                                                      This parameter can be a value of @ref QSPI_SIOOMode */
};

//----------------------------------------------------------------------

void flash_save_wavetable(const int16_t *ptr, uint32_t len, uint32_t address)
{
    HAL_StatusTypeDef tmp; // todo

    tmp = HAL_QSPI_Command(&hqspi, &cmd_WREN, TIMEOUT);   //enable data write

    enable_quad_write();

    len = len * sizeof(int16_t);
    uint32_t tmp_len = 0; //todo
    uint32_t block_len = 0;

    if (len <= MAX_DATA_TO_SAVE_PPQ)
    {
        cmd_PPQ.NbData = len;
        cmd_PPQ.Address = address * 2;

        check_memory_busy();

        tmp = HAL_QSPI_Command(&hqspi, &cmd_WREN, TIMEOUT);   //enable data write
        tmp = HAL_QSPI_Command(&hqspi, &cmd_PPQ, TIMEOUT);   //enable data write
        tmp = HAL_QSPI_Transmit(&hqspi, (uint8_t *)ptr, TIMEOUT);
    }
    else
    {
        while(tmp_len < len)
        {
            if(tmp_len <= (len - MAX_DATA_TO_SAVE_PPQ))
            {
                block_len = MAX_DATA_TO_SAVE_PPQ;
            }
            else
            {
                block_len = (len - tmp_len) % MAX_DATA_TO_SAVE_PPQ;
            }
            cmd_PPQ.NbData = block_len;
            cmd_PPQ.Address = tmp_len + address * 2;

            check_memory_busy();

            tmp = HAL_QSPI_Command(&hqspi, &cmd_WREN, TIMEOUT);   //enable data write
            tmp = HAL_QSPI_Command(&hqspi, &cmd_PPQ, TIMEOUT);    //write data block
            tmp = HAL_QSPI_Transmit(&hqspi, (uint8_t *)(ptr) + tmp_len, TIMEOUT);

            tmp_len += block_len;
        }
    }

    cmd_PPQ.NbData = 0;
    cmd_PPQ.Address = 0;
}

//----------------------------------------------------------------------

void flash_read_wavetable(int16_t *ptr, uint32_t len, uint32_t address)
{
    cmd_FRQIO.Address = address * 2;
    cmd_FRQIO.NbData = len * 2;

    check_memory_busy();

    HAL_QSPI_Command(&hqspi, &cmd_FRQIO, 100);
    HAL_QSPI_Receive(&hqspi, (uint8_t *)ptr, 100);
}

//----------------------------------------------------------------------

static void check_memory_busy(void)
{
    uint8_t tmp_data = 0U;
    HAL_StatusTypeDef tmp; // todo

    do
    {
        tmp = HAL_QSPI_Command(&hqspi, &cmd_RDSR, TIMEOUT);
        tmp = HAL_QSPI_Receive(&hqspi, &tmp_data, TIMEOUT);
    } while(tmp_data & 0x01);
}

//----------------------------------------------------------------------

static void enable_quad_write(void)
{
    HAL_StatusTypeDef tmp; // todo
    uint8_t tmp_data = 0x40;

    tmp = HAL_QSPI_Command(&hqspi, &cmd_WRSR, TIMEOUT);
    tmp = HAL_QSPI_Transmit(&hqspi, &tmp_data, TIMEOUT);
}
