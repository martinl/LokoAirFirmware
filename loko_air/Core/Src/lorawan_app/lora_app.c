#include "lora_app.h"
#include "LmHandler.h"
#include "Region.h"
#include "lora_app_version.h"
#include "lora_info.h"
#include "lorawan_version.h"
#include "platform.h"
#include "stm32_timer.h"
#include "subghz_phy_version.h"

#include <inttypes.h>

#include <bsp.h>
#include <settings/settings.h>

static void _on_join_request(LmHandlerJoinParams_t *join_params);
static void _on_tx_data(LmHandlerTxParams_t *params);
static void _on_rx_data(LmHandlerAppData_t *app_data, LmHandlerRxParams_t *params);
static void _on_mac_process_notify(void);
static void _on_restore_context_request(void *nvm, uint32_t nvm_size);
static void _on_store_context_request(void *nvm, uint32_t nvm_size);
/* -------------------------------------------------------------------------- */

uint8_t calculate_lorawan_battery_level(uint16_t voltage_mv, uint16_t min_voltage_mv, uint16_t max_voltage_mv) {
    /* Battery level: 0: very low, 254: fully charged  */

    if (voltage_mv < min_voltage_mv) {
        return 0;
    }

    if (voltage_mv >= max_voltage_mv) {
        return 254;
    }

    return (uint8_t)(((float)(voltage_mv - min_voltage_mv) / (max_voltage_mv - min_voltage_mv)) * 254);
}

/* -------------------------------------------------------------------------- */

static uint8_t _get_battery_level(void) {
    uint32_t vbat_mv = bsp_battery_get_voltage();

    uint8_t lorawan_batery_level = calculate_lorawan_battery_level(vbat_mv, 3300, 4200);

    LOG_DEBUG("Lorawan battery level: %d (254=>100%%)", lorawan_batery_level);

    return lorawan_batery_level;
}

/* -------------------------------------------------------------------------- */

static void _get_unique_id(uint8_t *id) {

    settings_get_lora_dev_eui(id);

    LOG_DEBUG_ARRAY_GREEN("unique id", id, SETTINGS_LORA_DEV_EUI_SIZE);
}

/* -------------------------------------------------------------------------- */

static uint32_t _get_dev_addr(void) {
    return settings_get_id_1();
}

/* -------------------------------------------------------------------------- */

static LmHandlerCallbacks_t _lm_handler_callbacks = {
    .GetBatteryLevel = _get_battery_level,
    .GetTemperature = NULL,
    .GetUniqueId = _get_unique_id,
    .GetDevAddr = _get_dev_addr,
    .OnMacProcess = _on_mac_process_notify,
    .OnJoinRequest = _on_join_request,
    .OnTxData = _on_tx_data,
    .OnRxData = _on_rx_data,
    .OnRestoreContextRequest = _on_restore_context_request,
    .OnStoreContextRequest = _on_store_context_request,
};

static LmHandlerParams_t _lm_handler_params = {
    .ActiveRegion = LORAMAC_REGION_EU868, /* Will be loaded from settings */
    .DefaultClass = LORAWAN_DEFAULT_CLASS,
    .AdrEnable = LORAWAN_ADR_STATE,
    .TxDatarate = LORAWAN_DEFAULT_DATA_RATE,
    .PingSlotPeriodicity = LORAWAN_DEFAULT_PING_SLOT_PERIODICITY,
};

static uint8_t _app_data_buffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];
static LmHandlerAppData_t _app_data = {
    .Port = 0,
    .BufferSize = 0,
    .Buffer = _app_data_buffer,
};

/* -------------------------------------------------------------------------- */

static volatile bool _is_tx_complete = false;
static volatile bool _is_joined = false;

/* -------------------------------------------------------------------------- */

void lorawan_init(void) {

    _is_joined = false;

    _lm_handler_params.ActiveRegion = settings_get_lorawan_region_id();

    lora_info_init();
    LmHandlerInit(&_lm_handler_callbacks, APP_VERSION);
    LmHandlerConfigure(&_lm_handler_params);

    LmHandlerJoin(LORAWAN_DEFAULT_ACTIVATION_TYPE, LORAWAN_FORCE_REJOIN_AT_BOOT);
}

/* -------------------------------------------------------------------------- */

static void _on_rx_data(LmHandlerAppData_t *app_data, LmHandlerRxParams_t *params) {
    if ((app_data == NULL) || (params == NULL)) {
        return;
    }

#if LOG_ENABLED == 1U
    static const char *_slot_strings[] = { "1", "2", "C", "C Multicast", "B Ping-Slot", "B Multicast Ping-Slot" };

    LOG_DEBUG("###### ========== MCPS-Indication ==========");
    LOG_DEBUG("###### D/L FRAME:%04" PRIu32 " | SLOT:%s | PORT:%d | DR:%d | RSSI:%d | SNR:%d",
              params->DownlinkCounter,
              _slot_strings[params->RxSlot],
              app_data->Port,
              params->Datarate,
              params->Rssi,
              params->Snr);
    LOG_DEBUG_ARRAY_RED("RX MCPS", app_data->Buffer, app_data->BufferSize);
#endif /* LOG_ENABLED == 1U */
    switch (app_data->Port) {
        case LORAWAN_SWITCH_CLASS_PORT:

            if (app_data->BufferSize == 1) {
                switch (app_data->Buffer[0]) {
                    case 0: {
                        LmHandlerRequestClass(CLASS_A);
                        break;
                    }
                    case 1: {
                        LmHandlerRequestClass(CLASS_B);
                        break;
                    }
                    case 2: {
                        LmHandlerRequestClass(CLASS_C);
                        break;
                    }
                    default:
                        break;
                }
            }
            break;
        case LORAWAN_USER_APP_PORT:

            break;

        default:

            break;
    }
}

/* -------------------------------------------------------------------------- */

void lorawan_send(void const *data, uint8_t size) {

    _app_data.Port = LORAWAN_USER_APP_PORT;
    _app_data.BufferSize = size;
    memcpy(_app_data.Buffer, data, size);

    _is_tx_complete = false;

    TimerTime_t next_tx_in = 0;

    LOG_DEBUG_ARRAY_BLUE("TX", _app_data.Buffer, _app_data.BufferSize);

    LmHandlerErrorStatus_t status = LmHandlerSend(&_app_data, _lm_handler_params.IsTxConfirmed, false);
    if (LORAMAC_HANDLER_SUCCESS == status) {
        LOG_INFO("SEND REQUEST...");
    } else if (LORAMAC_HANDLER_DUTYCYCLE_RESTRICTED == status) {
        next_tx_in = LmHandlerGetDutyCycleWaitTime();
        if (next_tx_in > 0) {
            LOG_INFO("Next Tx in  : ~%ld second(s)", (next_tx_in / 1000));
        }
    }
}

/* -------------------------------------------------------------------------- */

static void _on_tx_data(LmHandlerTxParams_t *params) {

    if ((params == NULL)) {
        return;
    }

    /* Process Tx event only if its a mcps response to prevent some internal events (mlme) */
    if (params->IsMcpsConfirm != 0) {
        LOG_DEBUG("###### ========== MCPS-Confirm =============");
        LOG_DEBUG("###### U/L FRAME:%04" PRIu32 " | PORT:%d | DR:%d | PWR:%d",
                  params->UplinkCounter,
                  params->AppData.Port,
                  params->Datarate,
                  params->TxPower);
        LOG_DEBUG_ARRAY_BLUE("MCPS", params->AppData.Buffer, params->AppData.BufferSize);

        LOG_DEBUG(" | MSG TYPE:");
        if (params->MsgType == LORAMAC_HANDLER_CONFIRMED_MSG) {
            LOG_DEBUG("CONFIRMED [%s]", (params->AckReceived != 0) ? "ACK" : "NACK");
        } else {
            LOG_DEBUG("UNCONFIRMED");
        }
        _is_tx_complete = true;
    }
}

/* -------------------------------------------------------------------------- */

static void _on_join_request(LmHandlerJoinParams_t *join_params) {
    if (join_params == NULL) {
        return;
    }

    if (join_params->Status == LORAMAC_HANDLER_SUCCESS) {
        LOG_INFO("###### = JOINED = ");
        LmHandlerNvmDataStore();
        _is_joined = true;
        if (join_params->Mode == ACTIVATION_TYPE_ABP) {
            LOG_DEBUG("ABP ======================");
        } else {
            LOG_DEBUG("OTAA =====================");
        }
    } else {
        LOG_ERROR("###### = JOIN FAILED, restart join process =");
        LmHandlerJoin(LORAWAN_DEFAULT_ACTIVATION_TYPE, LORAWAN_FORCE_REJOIN_AT_BOOT);
    }
}

/* -------------------------------------------------------------------------- */

static void _on_restore_context_request(void *nvm, uint32_t nvm_size) {
    LOG_WARNING("Restore %" PRIu32, nvm_size);
    bsp_flash_lorawan_nvm_read(0, nvm, nvm_size);
}

/* -------------------------------------------------------------------------- */

static void _on_store_context_request(void *nvm, uint32_t nvm_size) {

    if (memcmp(bsp_flash_lorawan_nvm_get_addr(), nvm, nvm_size) == 0) {
        LOG_INFO("No need save lorawan nvm");
    } else {
        LOG_INFO("Store nvm");
        bsp_flash_lorawan_nvm_erase();
        bsp_flash_lorawan_nvm_write(0, nvm, nvm_size);
    }
}

/* -------------------------------------------------------------------------- */

static void _on_mac_process_notify(void) {
    LOG_DEBUG("MAC State changed");
}

/* -------------------------------------------------------------------------- */

bool lorawan_is_joined(void) {
    return _is_joined;
}

/* -------------------------------------------------------------------------- */

bool lorawan_is_tx_complete(void) {
    return _is_tx_complete;
}

/* -------------------------------------------------------------------------- */

bool lorawan_deinit(void) {

    if (LmHandlerStop() == LORAMAC_HANDLER_SUCCESS) {
        LmHandlerNvmDataStore();
        return true;
    }

    return false;
}

/* -------------------------------------------------------------------------- */

void lorawan_process(void) {
    LmHandlerProcess();
}

/* -------------------------------------------------------------------------- */
