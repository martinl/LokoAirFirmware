#include "lora_info.h"
#include "LoRaMac.h"

static LoraInfo_t _lora_info = { 0, 0 };

void lora_info_init(void) {
    _lora_info.ActivationMode = 0;
    _lora_info.Region = 0;
    _lora_info.ClassB = 0;
    _lora_info.Kms = 0;

#ifdef REGION_AS923
    _lora_info.Region |= (1 << LORAMAC_REGION_AS923);
#endif /* REGION_AS923 */
#ifdef REGION_AU915
    _lora_info.Region |= (1 << LORAMAC_REGION_AU915);
#endif /* REGION_AU915 */
#ifdef REGION_CN470
    _lora_info.Region |= (1 << LORAMAC_REGION_CN470);
#endif /* REGION_CN470 */
#ifdef REGION_CN779
    _lora_info.Region |= (1 << LORAMAC_REGION_CN779);
#endif /* REGION_CN779 */
#ifdef REGION_EU433
    _lora_info.Region |= (1 << LORAMAC_REGION_EU433);
#endif /* REGION_EU433 */
#ifdef REGION_EU868
    _lora_info.Region |= (1 << LORAMAC_REGION_EU868);
#endif /* REGION_EU868 */
#ifdef REGION_KR920
    _lora_info.Region |= (1 << LORAMAC_REGION_KR920);
#endif /* REGION_KR920 */
#ifdef REGION_IN865
    _lora_info.Region |= (1 << LORAMAC_REGION_IN865);
#endif /* REGION_IN865 */
#ifdef REGION_US915
    _lora_info.Region |= (1 << LORAMAC_REGION_US915);
#endif /* REGION_US915 */
#ifdef REGION_RU864
    _lora_info.Region |= (1 << LORAMAC_REGION_RU864);
#endif /* REGION_RU864 */

    if (_lora_info.Region == 0) {
        LOG_WARNING("error: At least one region shall be defined in the MW: check lorawan_conf.h \r\n");
        while (1) {} /* At least one region shall be defined */
    }

#if (LORAMAC_CLASSB_ENABLED == 1)
    _lora_info.ClassB = 1;
#elif !defined(LORAMAC_CLASSB_ENABLED)
#    error LORAMAC_CLASSB_ENABLED not defined ( shall be <0 or 1> )
#endif /* LORAMAC_CLASSB_ENABLED */

#if (!defined(LORAWAN_KMS) || (LORAWAN_KMS == 0))
    _lora_info.Kms = 0;
    _lora_info.ActivationMode = 3;
#else  /* LORAWAN_KMS == 1 */
    _lora_info.Kms = 1;
    _lora_info.ActivationMode = ACTIVATION_BY_PERSONALIZATION + (OVER_THE_AIR_ACTIVATION << 1);
#endif /* LORAWAN_KMS */
}

LoraInfo_t *LoraInfo_GetPtr(void) {
    return &_lora_info;
}
