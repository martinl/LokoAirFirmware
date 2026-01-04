#include "radio_board_if.h"

#define USE_BSP_DRIVER 1
#include "stm32wlxx_LoRa_E5_mini_radio.h"

/* Exported functions --------------------------------------------------------*/
int32_t RBI_Init(void) {
#if defined(USE_BSP_DRIVER)
    /* Important note: BSP code is board dependent
     * STM32WL_Nucleo code can be found
     *       either in STM32CubeWL package under Drivers/BSP/STM32WLxx_Nucleo/
     *       or at https://github.com/STMicroelectronics/STM32CubeWL/tree/main/Drivers/BSP/STM32WLxx_Nucleo/
     * 1/ For User boards, the BSP/STM32WLxx_Nucleo/ directory can be copied and replaced in the project. The copy must
     * then be updated depending: on board RF switch configuration (pin control, number of port etc) on TCXO
     * configuration on DC/DC configuration on maximum output power that the board can deliver*/
    return bsp_radio_init();
#else
    /* 2/ Or implement RBI_Init here */
    int32_t retcode = 0;

#    warning user to provide its board code or to call his board driver functions

    return retcode;
#endif /* USE_BSP_DRIVER  */
}

int32_t RBI_DeInit(void) {
#if defined(USE_BSP_DRIVER)
    /* Important note: BSP code is board dependent
     * STM32WL_Nucleo code can be found
     *       either in STM32CubeWL package under Drivers/BSP/STM32WLxx_Nucleo/
     *       or at https://github.com/STMicroelectronics/STM32CubeWL/tree/main/Drivers/BSP/STM32WLxx_Nucleo/
     * 1/ For User boards, the BSP/STM32WLxx_Nucleo/ directory can be copied and replaced in the project. The copy must
     * then be updated depending: on board RF switch configuration (pin control, number of port etc) on TCXO
     * configuration on DC/DC configuration on maximum output power that the board can deliver*/
    return bsp_radio_de_init();
#else
    /* 2/ Or implement RBI_DeInit here */
    int32_t retcode = 0;

#    warning user to provide its board code or to call his board driver functions

    return retcode;
#endif /* USE_BSP_DRIVER */
}

int32_t RBI_ConfigRFSwitch(RBI_Switch_TypeDef config) {
#if defined(USE_BSP_DRIVER)

    /* Important note: BSP code is board dependent
     * STM32WL_Nucleo code can be found
     *       either in STM32CubeWL package under Drivers/BSP/STM32WLxx_Nucleo/
     *       or at https://github.com/STMicroelectronics/STM32CubeWL/tree/main/Drivers/BSP/STM32WLxx_Nucleo/
     * 1/ For User boards, the BSP/STM32WLxx_Nucleo/ directory can be copied and replaced in the project. The copy must
     * then be updated depending: on board RF switch configuration (pin control, number of port etc) on TCXO
     * configuration on DC/DC configuration on maximum output power that the board can deliver*/
    return bsp_radio_config_rf_switch((BSP_RADIO_Switch_TypeDef)config);
#else
    /* 2/ Or implement RBI_ConfigRFSwitch here */
    int32_t retcode = 0;

#    warning user to provide its board code or to call his board driver functions

    return retcode;
#endif /* USE_BSP_DRIVER */
}

int32_t RBI_GetTxConfig(void) {
#if defined(USE_BSP_DRIVER)
    /* Important note: BSP code is board dependent
     * STM32WL_Nucleo code can be found
     *       either in STM32CubeWL package under Drivers/BSP/STM32WLxx_Nucleo/
     *       or at https://github.com/STMicroelectronics/STM32CubeWL/tree/main/Drivers/BSP/STM32WLxx_Nucleo/
     * 1/ For User boards, the BSP/STM32WLxx_Nucleo/ directory can be copied and replaced in the project. The copy must
     * then be updated depending: on board RF switch configuration (pin control, number of port etc) on TCXO
     * configuration on DC/DC configuration on maximum output power that the board can deliver*/
    return bsp_radio_get_tx_config();
#else
    /* 2/ Or implement RBI_GetTxConfig here */
    int32_t retcode = RBI_CONF_RFO;

#    warning user to provide its board code or to call his board driver functions

    return retcode;
#endif /* USE_BSP_DRIVER */
}

int32_t RBI_IsTCXO(void) {
#if defined(USE_BSP_DRIVER)
    /* Important note: BSP code is board dependent
     * STM32WL_Nucleo code can be found
     *       either in STM32CubeWL package under Drivers/BSP/STM32WLxx_Nucleo/
     *       or at https://github.com/STMicroelectronics/STM32CubeWL/tree/main/Drivers/BSP/STM32WLxx_Nucleo/
     * 1/ For User boards, the BSP/STM32WLxx_Nucleo/ directory can be copied and replaced in the project. The copy must
     * then be updated depending: on board RF switch configuration (pin control, number of port etc) on TCXO
     * configuration on DC/DC configuration on maximum output power that the board can deliver*/
    return bsp_radio_is_tcxo();
#else
    /* 2/ Or implement RBI_IsTCXO here */
    int32_t retcode = IS_TCXO_SUPPORTED;

#    warning user to provide its board code or to call his board driver functions

    return retcode;
#endif /* USE_BSP_DRIVER  */
}

int32_t RBI_IsDCDC(void) {
#if defined(USE_BSP_DRIVER)
    /* Important note: BSP code is board dependent
     * STM32WL_Nucleo code can be found
     *       either in STM32CubeWL package under Drivers/BSP/STM32WLxx_Nucleo/
     *       or at https://github.com/STMicroelectronics/STM32CubeWL/tree/main/Drivers/BSP/STM32WLxx_Nucleo/
     * 1/ For User boards, the BSP/STM32WLxx_Nucleo/ directory can be copied and replaced in the project. The copy must
     * then be updated depending: on board RF switch configuration (pin control, number of port etc) on TCXO
     * configuration on DC/DC configuration on maximum output power that the board can deliver*/
    return bsp_radio_is_dcdc();
#else
    /* 2/ Or implement RBI_IsDCDC here */
    int32_t retcode = IS_DCDC_SUPPORTED;

#    warning user to provide its board code or to call his board driver functions

    return retcode;
#endif /* USE_BSP_DRIVER  */
}

int32_t RBI_GetRFOMaxPowerConfig(RBI_RFOMaxPowerConfig_TypeDef config) {
#if defined(USE_BSP_DRIVER)
    /* Important note: BSP code is board dependent
     * STM32WL_Nucleo code can be found
     *       either in STM32CubeWL package under Drivers/BSP/STM32WLxx_Nucleo/
     *       or at https://github.com/STMicroelectronics/STM32CubeWL/tree/main/Drivers/BSP/STM32WLxx_Nucleo/
     * 1/ For User boards, the BSP/STM32WLxx_Nucleo/ directory can be copied and replaced in the project. The copy must
     * then be updated depending: on board RF switch configuration (pin control, number of port etc) on TCXO
     * configuration on DC/DC configuration on maximum output power that the board can deliver*/
    return 15;  // BSP_RADIO_GetRFOMaxPowerConfig((BSP_RADIO_RFOMaxPowerConfig_TypeDef) Config);
#else
    /* 2/ Or implement RBI_RBI_GetRFOMaxPowerConfig here */
    int32_t ret = 0;

#    warning user to provide its board code or to call his board driver functions
    if (Config == RBI_RFO_LP_MAXPOWER) {
        ret = 15; /*dBm*/
    } else {
        ret = 22; /*dBm*/
    }

    return ret;
#endif /* USE_BSP_DRIVER  */
}
