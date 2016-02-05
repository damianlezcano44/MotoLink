#include "storage.h"
#include <string.h>

const SPIConfig EEPROM_SPIDCONFIG = {
  NULL,
  PORT_SPI2_NSS,
  PAD_SPI2_NSS,
  0, // Up to 20Mhz
  SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0
};

static SPIEepromFileConfig eeSettingsCfg = {
  0,
  EEPROM_SPLIT-1,
  EEPROM_SIZE,
  EEPROM_PAGE_SIZE,
  MS2ST(EEPROM_WRITE_TIME_MS),
  &EEPROM_SPID,
  &EEPROM_SPIDCONFIG,
};

static SPIEepromFileConfig eeTablesCfg = {
  EEPROM_SPLIT,
  EEPROM_SIZE,
  EEPROM_SIZE,
  EEPROM_PAGE_SIZE,
  MS2ST(EEPROM_WRITE_TIME_MS),
  &EEPROM_SPID,
  &EEPROM_SPIDCONFIG,
};

static SPIEepromFileStream settingsFile, tablesFile;
static EepromFileStream *settingsFS, *tablesFS;

static const uint32_t magic_key = 0xABEF1289;
static tables_t tables_buf = {0xABEF1289, 0, {}, {}, 0};
static uint32_t gCrc = 0;
static uint32_t counters[EEPROM_SIZE / EEPROM_TABLES_PAGE_SIZE];

void crc_callback(CRCDriver *crcp, uint32_t crc) {
  (void)crcp;
  gCrc = crc;
}

static const CRCConfig crc32_config = {
  .poly_size         = 32,
  .poly              = 0x04C11DB7,
  .initial_val       = 0xFFFFFFFF,
  .final_val         = 0xFFFFFFFF,
  .reflect_data      = 1,
  .reflect_remainder = 1
};

/*
 * CRC32 configuration with DMA
 */
static const CRCConfig crc32_dma_config = {
  .poly_size         = 32,
  .poly              = 0x04C11DB7,
  .initial_val       = 0xFFFFFFFF,
  .final_val         = 0xFFFFFFFF,
  .reflect_data      = 1,
  .reflect_remainder = 1,
  .end_cb = crc_callback
};

static uint32_t getCrc(const CRCConfig *config, uint8_t *data, uint16_t len)
{
  uint32_t crc;

  crcAcquireUnit(&CRCD1);             /* Acquire ownership of the bus.    */
  crcStart(&CRCD1, config);           /* Activate CRC driver              */
  crcReset(&CRCD1);
  crc = crcCalc(&CRCD1, len, data);
  crcStop(&CRCD1);                    /* Deactive CRC driver);            */
  crcReleaseUnit(&CRCD1);             /* Acquire ownership of the bus.    */

  return crc;
}

static uint32_t getCrcDma(const CRCConfig *config, uint8_t *data, uint16_t len)
{
  gCrc = 0;

  crcAcquireUnit(&CRCD1);             /* Acquire ownership of the bus.    */
  crcStart(&CRCD1, config);           /* Activate CRC driver              */
  crcReset(&CRCD1);
  crcStartCalc(&CRCD1, len, &data);
  while (gCrc == 0)
      chThdSleepMilliseconds(1);                  /* Wait for callback to verify      */
  crcStop(&CRCD1);                    /* Deactive CRC driver);            */
  crcReleaseUnit(&CRCD1);             /* Acquire ownership of the bus.    */

  return gCrc;
}

void eeInit(void) {

    settingsFS = SPIEepromFileOpen(&settingsFile, &eeSettingsCfg, EepromFindDevice(EEPROM_DEV_25XX));
    tablesFS = SPIEepromFileOpen(&tablesFile, &eeTablesCfg, EepromFindDevice(EEPROM_DEV_25XX));

    fileStreamSeek(settingsFS, 0);
    fileStreamSeek(tablesFS, 0);
}

uint32_t eeFindCurrentPage(void)
{
    uint32_t addr, magic, max_counter = 0;
    uint8_t i = 0;
    const uint32_t from = 0;
    const uint32_t to = from+EEPROM_SIZE;
    msg_t status;

    // TODO: Fix

    for (addr=from; addr <= to; addr += EEPROM_TABLES_PAGE_SIZE)
	{
        fileStreamSeek(tablesFS, addr);

        status = fileStreamRead(tablesFS, (uint8_t*)&magic, sizeof magic);
        if (status != MSG_OK)
		{
            continue;
		}

        status = fileStreamRead(tablesFS, (uint8_t*)&counters[i], sizeof counters[0]);
        if (status != MSG_OK)
        {
            counters[i] = 0;
            continue;
        }

        i++;
	}

    // Find highest counter
    addr = 0;
    for (i = 0; i < (sizeof counters / sizeof counters[0]); i++)
    {
        if (counters[i] > max_counter)
        {
            max_counter = counters[i];
            addr = i * EEPROM_TABLES_PAGE_SIZE; // Move address to page with highest counter
        }
    }

    return addr;
}

uint32_t eeFindNextPage(void)
{
    uint32_t addr = eeFindCurrentPage();

	/* Check if at last page */
    if(addr >= EEPROM_SIZE - EEPROM_TABLES_PAGE_SIZE)
	{
        addr = 0;
	}
    else
    {
        addr += EEPROM_TABLES_PAGE_SIZE;
    }

	return addr;
}

uint32_t eeFindPrevPage(void)
{
    uint32_t addr = eeFindCurrentPage();

    /* Check if at first page */
    if(addr == 0)
    {
        addr = EEPROM_SIZE - EEPROM_TABLES_PAGE_SIZE;
    }
    else
    {
        addr -= EEPROM_TABLES_PAGE_SIZE;
    }

    return addr;
}

uint8_t eePushPage(uint32_t page, uint8_t *buffer, uint32_t len)
{
    if (fileStreamSeek(tablesFS, page) != MSG_OK)
        return 1;

    if (fileStreamWrite(tablesFS, buffer, len) != MSG_OK)
        return 2;

    return 0;
}

uint8_t eePullPage(uint32_t page, uint8_t *buffer, uint32_t len)
{
    if (fileStreamSeek(tablesFS, page) != MSG_OK)
        return 1;

    if (fileStreamRead(tablesFS, buffer, len) != MSG_OK)
        return 2;

    return 0;
}

void readTablesFromEE(void)
{
    uint32_t crc1, crc2 = 0;
    //eePullPage(eeFindCurrentPage(), (uint8_t*)&tables_buf, sizeof tables_buf);
    eePullPage(2048, (uint8_t*)&tables_buf, sizeof tables_buf);

    crc1 = tables_buf.crc;
    crc2 = getCrc(&crc32_config, (uint8_t*)&tables_buf, (sizeof tables_buf - sizeof tables_buf.crc));

    if (crc1 != crc2)
    {
        // Try previous page
        /*
        eePullPage(eeFindPrevPage(), (uint8_t*)&tables_buf, sizeof tables_buf);
        crc1 = tables_buf.crc;
        crc2 = getCrc(&crc32_config, (uint8_t*)&tables_buf, (sizeof tables_buf - sizeof tables_buf.crc));
        if (crc1 != crc2)*/
            return;
    }

    memcpy(tables_buf.afr, tableAFR, sizeof tableAFR);
    memcpy(tables_buf.knock, tableKnock, sizeof tableKnock);
}

void writeTablesToEE(void)
{
    memcpy(tableAFR, tables_buf.afr, sizeof tableAFR);
    memcpy(tableKnock, tables_buf.knock, sizeof tableKnock);

    tables_buf.crc = getCrc(&crc32_config, (uint8_t*)&tables_buf, (sizeof tables_buf - sizeof tables_buf.crc));

    //eePushPage(eeFindNextPage(), (uint8_t*)&tables_buf, sizeof tables_buf);
    eePushPage(2048, (uint8_t*)&tables_buf, sizeof tables_buf);
}
