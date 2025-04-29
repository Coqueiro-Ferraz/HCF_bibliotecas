idf_component_register(SRCS "app_main.c"
                    INCLUDE_DIRS "."
                    REQUIRES HCF_ADC
                    REQUIRES HCF_IOTEC
                    REQUIRES HCF_LCD
                    REQUIRES HCF_MP
                    REQUIRES HCF_ULTRA
                    REQUIRES HCF_DHT
                    REQUIRES HCF_WNOLOGY
                    REQUIRES HCF_SOFT
                    REQUIRES esp_rom
                    )
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common
    )

