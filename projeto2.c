#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

const int VRX = 26;
const int VRY = 27;
const int ADC_CHANNEL_0 = 0;
const int ADC_CHANNEL_1 = 1;
const int SW = 22;

bool pause = true;
bool btn_b_status = false;

#define PWM_PIN 21         // Defina o pino de saída do PWM (exemplo: GP10 e GP21 são buzzes passivos embutidos)
#define CLOCK_PWM 12200000 // Clock da Pico em 125 MHz
#define PWM_DIVIDER 100.0  // Divisor do clock PWM

#define Do 262
#define Re 294
#define Mi 330
#define Fa 349
#define Sol 392
#define La 440
#define Si 494
#define DO 528
int frequencia = 0;

const int led_pin_blue = 12;
const int led_pin_red = 13;
const float DIVIDER_PWM = 16.0;
const uint16_t PERIOD = 4096;
uint16_t led_b_level, led_r_level = 100;
uint slice_led_b, slice_led_r;


const int button_a = 5; 
int button_b_value = 0;
const uint button_b = 6; 
static volatile uint a=1;
static volatile uint b=1;
static void gpio_irq_handler(uint gpio, uint32_t events);

uint16_t vrx_value, vry_value, sw_value;



void setup_joystick(){
    adc_init();
    adc_gpio_init(VRX);
    adc_gpio_init(VRY);
    gpio_init(SW);
    gpio_set_dir(SW, GPIO_IN);
    gpio_pull_up(SW);
}

void setup_pwm_led(uint led, uint *slice, uint16_t level){
    gpio_set_function(led, GPIO_FUNC_PWM);
    *slice = pwm_gpio_to_slice_num(led);
    pwm_set_clkdiv(*slice, DIVIDER_PWM);
    pwm_set_wrap(*slice, PERIOD),
    pwm_set_gpio_level(led, level);
    pwm_set_enabled(*slice, true);
    
    gpio_init(led_pin_red);
    gpio_set_dir(led_pin_red, GPIO_OUT);
    
    gpio_init(button_a);
    gpio_set_dir(button_a, GPIO_IN);
    gpio_pull_up(button_a);

    gpio_init(button_b);
    gpio_set_dir(button_b, GPIO_IN);
    gpio_pull_up(button_b);
    
}
void setup(){
    stdio_init_all();   
    setup_joystick();
    setup_pwm_led(led_pin_blue, &slice_led_b, led_b_level);
    setup_pwm_led(led_pin_red, &slice_led_r, led_r_level);
}

void joystick_read_axis(uint16_t *vrx_value, uint16_t *vry_value, uint16_t *sw_value){
    adc_select_input(ADC_CHANNEL_0);
        sleep_us(2);
        *vrx_value = adc_read();
        adc_select_input(ADC_CHANNEL_1);
        sleep_us(2);
        *vry_value = adc_read();
        *sw_value= !gpio_get(SW);
        //printf("X: %u, Y: %u, Botão: %d\n", *vrx_value, *vry_value, *sw_value);
        if (*sw_value==1 || a%2 ==0)
        {
            gpio_put(led_pin_red, true);
        }
        else{
            gpio_put(led_pin_red, false);
        }
}
void ledEnable(bool ativo){
    joystick_read_axis(&vrx_value, &vry_value, &sw_value);
    if (ativo)
    {
        pwm_set_gpio_level(led_pin_blue, vrx_value);
        pwm_set_gpio_level(led_pin_red, vry_value);
    }
    else
    {
        pwm_set_gpio_level(led_pin_blue, 0);
        pwm_set_gpio_level(led_pin_red, 0);
}
    }

void pressButtonB (){
    b++;
    if (b%2 ==0)
    {
        btn_b_status = true;
    }   
    else{
        pwm_set_gpio_level(led_pin_blue, 0);
        pwm_set_gpio_level(led_pin_red, 0);
        btn_b_status = false;
        printf("Pause\n");
    }
    
}
    
void tone(int nota, int time)
{
    int button_b_value = !gpio_get(button_a);
    if (pause==false)
    {   
        uint slice_Do = pwm_gpio_to_slice_num(PWM_PIN);                       // Obtém o número do slice PWM associado ao pino
        uint32_t wrap_valueDo = (uint32_t)(CLOCK_PWM / (nota * PWM_DIVIDER)); // calcula o valor do wrap manualmente
        pwm_set_wrap(slice_Do, wrap_valueDo);                                 // Configura o wrap ou top do PWM
        pwm_set_clkdiv(slice_Do, PWM_DIVIDER);                                // configura o divisor do PWM
        pwm_set_gpio_level(PWM_PIN, wrap_valueDo / 2);                        // Ciclo de trabalho do duty cycle em 50%
        pwm_set_enabled(slice_Do, true);
        sleep_ms(time);
        pwm_set_enabled(slice_Do, false);
        pwm_set_gpio_level(led_pin_red, 400);
    }
    else
    {
        gpio_put(led_pin_red, false);
    }
    if (button_b_value==1)
    {
        pressButtonB();
    }
    
    if (btn_b_status == true)
    {
        ledEnable(true);
    }
    else
    {
        ledEnable(false);
    }   
}


int main(){
    
    
    setup();
    printf("joystick PWM\n");
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM); // Configura o pino como saída PWM
    gpio_set_irq_enabled_with_callback(button_b, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    while (true) {

        for(frequencia = 200; frequencia <500; frequencia+=10){
            tone(frequencia,20);    
            printf("valor da A: %d\n", button_b_value);             
        }
        for(frequencia = 500; frequencia > 200; frequencia-=10){
            tone(frequencia,20);
            printf("valor da A: %d\n", button_b_value);
        }
        sleep_ms(100);
    }
}


void gpio_irq_handler (uint gpio, uint32_t events){
    a++;
    if (a%2==0)
    {
        printf("SOCORRO %d\n", a);
        pause = true;
    }   
    else{
        pause = false;
        gpio_put(led_pin_red, false);
        printf("Pause\n");
    }
}

