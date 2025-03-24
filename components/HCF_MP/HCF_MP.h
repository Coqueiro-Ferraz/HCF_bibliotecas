#ifndef __HCF_MP_H
    #define __HCF_MP_H
   // #include "esp_err.h"
 

    void iniciar_MP(int modo);
    void rotacionar_MP(int horario, float graus);
    void rotacionar_DRV8825(bool horario, float angulo, int espera_us);
    void iniciar_driver(int Fim_de_curso_inicial, int Fim_de_curso_final);
    void rotacionar_driver(int abrir, float graus, uint8_t saidas_atual);
#endif