#ifndef __HCF_ULTRA_H
    #define __HCF_ULTRA_H
   // #include "esp_err.h"
    //void Enviar_lcd595(uint8_t dado);

    void iniciar_ultrassonico(int Trigger, int Echo);
    float medir_distancia(void);
#endif