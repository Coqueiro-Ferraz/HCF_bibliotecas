#ifndef __HCF_SOFT_H
    #define __HCF_SOFT_H
   // #include "esp_err.h"

    // Retorna 0 se o nível do pino não for atingido antes do estouro em us
    int espera_nivel(int pino, int nivel, uint32_t estouro_us);
    void piscar_LED(int vezes, int pino, int tempo_alto, int tempo_baixo);
    void delay_ms (int ms);
    void delay_us (int us);


    
#endif