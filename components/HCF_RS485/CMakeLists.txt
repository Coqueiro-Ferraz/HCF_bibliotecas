idf_component_register(SRCS "HCF_RS485.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver
                    REQUIRES esp_timer
                    )