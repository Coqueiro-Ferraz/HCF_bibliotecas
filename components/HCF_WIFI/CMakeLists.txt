idf_component_register(SRCS "HCF_WIFI.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver
                    REQUIRES esp_wifi
                    REQUIRES esp_netif
                    )