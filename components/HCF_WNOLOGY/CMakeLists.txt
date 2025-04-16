idf_component_register(SRCS "HCF_WNOLOGY.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver
                    REQUIRES HCF_WIFI
                    REQUIRES json
                    REQUIRES nvs_flash                  
                    REQUIRES mqtt
                    )
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common
    )