#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/gpio.h>
#include "zephyr/dt-bindings/gpio/gpio.h"
#include "zephyr/logging/log_core.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define LED0_NODE DT_ALIAS(led0)
#define BUTTON0_NODE DT_ALIAS(sw0)

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(BUTTON0_NODE, gpios, {0});
static const struct pwm_dt_spec pwm_led = PWM_DT_SPEC_GET_OR(LED0_NODE, {0});

#define PWM_MAX 1000000U // 1 MHz -> duty cycle em microssegundos
#define PWM_STEP 50000U  // passo do fade 

// Variáveis globais
static bool led_on = false;
static bool pwm_mode = false;

int main(void)
{

    // Configurar LED digital (GPIO)
    struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
    if (!device_is_ready(led.port)) {
        LOG_ERR("LED device not ready");
        return -1;
    }
    gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);

    // Configurar botão 
    if (!device_is_ready(button.port)) {
        LOG_ERR("Button device not ready");
        return -1;
    }
    gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_PULL_UP);

    uint32_t duty = 0;
    bool fade_up = true;

    while (1) {
        // Leitura do botão
        int val = gpio_pin_get_dt(&button);
        if (val == 1) {
            pwm_mode = !pwm_mode;   // alterna o modo
            k_msleep(300);          // debounce simples
        }

        if (pwm_mode) {
            // Controle PWM (fade in/out)
            if (fade_up) {
                duty += PWM_STEP;
                if (duty >= PWM_MAX) {
                    duty = PWM_MAX;
                    fade_up = false;
                }
            } else {
                if (duty >= PWM_STEP) {
                    duty -= PWM_STEP;
                } else {
                    duty = 0;
                    fade_up = true;
                }
            }
            pwm_set_dt(&pwm_led, PWM_MAX, duty);  // periodo = PWM_MAX, duty em micros
            k_msleep(50);
        } else {
            // LED digital liga/desliga
            led_on = !led_on;
            gpio_pin_set_dt(&led, led_on);
            printk("Led is %s", led_on? "On":"Off");
            k_msleep(500);
        }
    }
  return 0;
}
