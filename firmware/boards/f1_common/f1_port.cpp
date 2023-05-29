#include "port.h"

#include "wideband_config.h"

#include "hal.h"
#include "hal_mfs.h"

// Storage

static const MFSConfig mfscfg1 = {
    .flashp           = (BaseFlash *)&EFLD1,
    .erased           = 0xFFFFFFFFU,
    .bank_size        = 4096U,
    .bank0_start      = 120U,
    .bank0_sectors    = 4U,
    .bank1_start      = 124U,
    .bank1_sectors    = 4U
};

static MFSDriver mfs1;

// Settings
static Configuration cfg;
#define MFS_CONFIGURATION_RECORD_ID     1

#ifndef BOARD_DEFAULT_SENSOR_TYPE
#define BOARD_DEFAULT_SENSOR_TYPE SensorType::LSU49
#endif

// Configuration defaults
void Configuration::LoadDefaults()
{
    int i;

    CanIndexOffset = 0;
    sensorType = BOARD_DEFAULT_SENSOR_TYPE;

    /* default auxout curve is 0..5V for AFR 8.5 to 18.0
     * default auxout[n] input is AFR[n] */
    for (i = 0; i < 8; i++) {
        auxOutBins[0][i] = auxOutBins[1][i] = 8.5 + (18.0 - 8.5) / 7 * i;
        auxOutValues[0][i] = auxOutValues[1][i] = 0.0 + (5.0 - 0.0) / 7 * i;
    }
    auxOutputSource[0] = AuxOutputMode::Afr0;
    auxOutputSource[1] = AuxOutputMode::Afr1;

    /* Finaly */
    Tag = ExpectedTag;
}

int InitConfiguration()
{
    size_t size = GetConfigurationSize();

    /* Starting EFL driver.*/
    eflStart(&EFLD1, NULL);

    mfsObjectInit(&mfs1);

    mfs_error_t err = mfsStart(&mfs1, &mfscfg1);
    if (err != MFS_NO_ERROR) {
        return -1;
    }

    err = mfsReadRecord(&mfs1, MFS_CONFIGURATION_RECORD_ID, &size, GetConfiguratiuonPtr());
    if ((err != MFS_NO_ERROR) || (size != GetConfigurationSize() || !cfg.IsValid())) {
        /* load defaults */
        cfg.LoadDefaults();
    }

    return 0;
}

Configuration* GetConfiguration()
{
    return &cfg;
}

void SetConfiguration()
{
    SaveConfiguration();
}

/* TS stuff */
int SaveConfiguration() {
    /* TODO: handle error */
    mfs_error_t err = mfsWriteRecord(&mfs1, MFS_CONFIGURATION_RECORD_ID, GetConfigurationSize(), GetConfiguratiuonPtr());
    if (err != MFS_NO_ERROR) {
        return -1;
    }
    return 0;
}

uint8_t *GetConfiguratiuonPtr()
{
    return (uint8_t *)&cfg;
}

size_t GetConfigurationSize()
{
    return sizeof(cfg);
}

const char *getTsSignature() {
    return TS_SIGNATURE;
}

SensorType GetSensorType()
{
    return cfg.sensorType;
}

void ToggleESRDriver(SensorType sensor)
{
    switch (sensor) {
        case SensorType::LSU42:
            palTogglePad(NERNST_42_ESR_DRIVER_PORT, NERNST_42_ESR_DRIVER_PIN);
        break;
        case SensorType::LSU49:
            palTogglePad(NERNST_49_ESR_DRIVER_PORT, NERNST_49_ESR_DRIVER_PIN);
        break;
        case SensorType::LSUADV:
            palTogglePad(NERNST_ADV_ESR_DRIVER_PORT, NERNST_ADV_ESR_DRIVER_PIN);
        break;
    }
}