#include "gpio.h"

uint8_t gpio_set(uint8_t port, uint8_t pin_number, uint8_t pin_mode){

    if(pin_number > 15u) return 1u;

    if(port == 'A'){
        RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
        (void)RCC->AHBENR;

        GPIOA->MODER  |=  (pin_mode << (2u * pin_number));
        
    }
    else if (port == 'B'){
        RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
        (void)RCC->AHBENR;

        GPIOB->MODER  |=  (pin_mode << (2u * pin_number));
    }
    else if (port == 'C'){
        RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
        (void)RCC->AHBENR;

        GPIOC->MODER  |=  (pin_mode << (2u * pin_number));
    }
    else if (port == 'D'){
        RCC->AHBENR |= RCC_AHBENR_GPIODEN;
        (void)RCC->AHBENR;

        GPIOD->MODER  |=  (pin_mode << (2u * pin_number));
    }
    else if (port == 'E'){
        RCC->AHBENR |= RCC_AHBENR_GPIOEEN;
        (void)RCC->AHBENR;

        GPIOE->MODER  |=  (pin_mode << (2u * pin_number));
    }
    else if (port == 'F'){
        RCC->AHBENR |= RCC_AHBENR_GPIOFEN;
        (void)RCC->AHBENR;

        GPIOF->MODER  |=  (pin_mode << (2u * pin_number));
    }
    else{
        return 1u;
    }

    return 0u;
}



uint8_t gpio_write(uint8_t port, uint8_t pin_number, uint8_t state){

    if(pin_number > 15u) return 1;

    if(port == 'A'){
        if(state == HIGH)   GPIOA->BSRR = 1u << pin_number;
        if(state == LOW)    GPIOA->BSRR = 1u << (16 + pin_number);
        else return 1u;
    }
    
    else if (port == 'B'){
        if(state == HIGH)   GPIOB->BSRR = 1u << pin_number;
        if(state == LOW)    GPIOB->BSRR = 1u << (16 + pin_number);
        else return 1u;
    }
    else if (port == 'C'){
        if(state == HIGH)   GPIOC->BSRR = 1u << pin_number;
        if(state == LOW)    GPIOC->BSRR = 1u << (16 + pin_number);
        else return 1u;
    }
    else if (port == 'D'){
        if(state == HIGH)   GPIOD->BSRR = 1u << pin_number;
        if(state == LOW)    GPIOD->BSRR = 1u << (16 + pin_number);
        else return 1u;
    }
    else if (port == 'E'){
        if(state == HIGH)   GPIOE->BSRR = 1u << pin_number;
        if(state == LOW)    GPIOE->BSRR = 1u << (16 + pin_number);
        else return 1u;
    }
    else if (port == 'F'){
        if(state == HIGH)   GPIOF->BSRR = 1u << pin_number;
        if(state == LOW)    GPIOF->BSRR = 1u << (16 + pin_number);
        else return 1u;
    }
    else{
        return 1u;
    }

    return 0u;
}


uint8_t gpio_digital_read(uint8_t port, uint8_t pin_number){

    uint16_t idr_value, mask;
    mask = 1u << pin_number;

    if(port == 'A'){
        idr_value = (uint16_t)GPIOA->IDR;
    }
    else if(port == 'B'){
        idr_value = (uint16_t)GPIOB->IDR;
    }
    else if(port == 'B'){
        idr_value = (uint16_t)GPIOC->IDR;
    }
    else if(port == 'B'){
        idr_value = (uint16_t)GPIOD->IDR;
    }
    else if(port == 'B'){
        idr_value = (uint16_t)GPIOE->IDR;
    }
    else if(port == 'B'){
        idr_value = (uint16_t)GPIOF->IDR;
    }
    else{
        return 11u;
    }

    return (uint8_t)(idr_value & mask);

}