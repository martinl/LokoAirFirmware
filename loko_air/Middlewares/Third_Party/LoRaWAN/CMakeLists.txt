project(LoRaWAN)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        .
)

target_sources(${PROJECT_NAME}
    INTERFACE
        Crypto/cmac.c
        Crypto/lorawan_aes.c
        Crypto/soft-se.c
        LmHandler/LmHandler.c
        LmHandler/NvmDataMgmt.c
        LmHandler/Packages/LmhpCompliance.c
        Mac/LoRaMac.c
        Mac/LoRaMacAdr.c
        Mac/LoRaMacClassB.c
        Mac/LoRaMacCommands.c
        Mac/LoRaMacConfirmQueue.c
        Mac/LoRaMacCrypto.c
        Mac/LoRaMacParser.c
        Mac/LoRaMacSerializer.c
        Mac/Region/Region.c
        Mac/Region/RegionAS923.c
        Mac/Region/RegionAU915.c
        Mac/Region/RegionBaseUS.c
        Mac/Region/RegionCN470.c
        Mac/Region/RegionCN470A20.c
        Mac/Region/RegionCN470A26.c
        Mac/Region/RegionCN470B20.c
        Mac/Region/RegionCN470B26.c
        Mac/Region/RegionCN779.c
        Mac/Region/RegionCommon.c
        Mac/Region/RegionEU433.c
        Mac/Region/RegionEU868.c
        Mac/Region/RegionIN865.c
        Mac/Region/RegionKR920.c
        Mac/Region/RegionRU864.c
        Mac/Region/RegionUS915.c
        Utilities/utilities.c
)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        LmHandler
        LmHandler/Packages
        Mac
        Mac/Region
        Utilities
)
