idf_component_register(SRCS "HCF_SOFT.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver
                    REQUIRES esp_rom
                    REQUIRES esp_timer
                    )