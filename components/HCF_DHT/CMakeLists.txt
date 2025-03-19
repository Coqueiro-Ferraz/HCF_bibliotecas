idf_component_register(SRCS "HCF_DHT.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver
                    REQUIRES esp_timer
                    )