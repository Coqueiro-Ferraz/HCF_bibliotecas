#ifndef __HCF_DHT_H
    #define __HCF_DHT_H
   // #include "esp_err.h"
    //void Enviar_lcd595(uint8_t dado);

    void iniciar_DHT(int pin);
    int DHT_temp_umidade(float *temperatura, float *umidade);
#endif