idf_component_register(SRCS "HCF_RFID.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver
                    REQUIRES esp_timer
                    )