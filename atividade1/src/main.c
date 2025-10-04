#include "zephyr/logging/log_core.h"
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

// Levels of LOG
// LOG_MODULE_REGISTER(main, NONE); //0
// LOG_MODULE_REGISTER(main, ERR);  //1
// LOG_MODULE_REGISTER(main, WRN); //2
// LOG_MODULE_REGISTER(main, INF); //3
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG); //4

// extern struct k_timer my_timer;
// extern void my_expiry_function(struct k_timer *timer_id)
// {
// 	printk("Time expired for %d times\n", timer_id->status);
// };
//
// K_TIMER_DEFINE(my_timer, my_expiry_function, NULL);

// #define TIMER_MS 1000 * CONFIG_TICKS_PER_SECOND

static struct k_timer my_timer;

void timer_callback(struct k_timer *timer) {

  if (timer == &my_timer) {
    LOG_INF("Hello World!\n");
    LOG_DBG("Hello World!\n");
    LOG_ERR("Hello World!\n");
    // printk("Tick freq: %d\n", CONFIG_SYS_CLOCK_TICKS_PER_SEC);
  }
}


int main(void)
{
  
  k_timer_init(&my_timer, timer_callback, NULL);

  k_timer_start(&my_timer, K_MSEC(CONFIG_TIMER_MS), K_MSEC(CONFIG_TIMER_MS));

  while (1) {
    k_sleep(K_FOREVER);
  }
	// k_timer_start(&my_timer, K_NO_WAIT, K_SECONDS(1));
	//
	//  while (1) {
	//    printk("Exp: %lld\n", k_timer_expires_ticks(&my_timer));
	//    k_sleep(K_SECONDS(1));
	//  }

	// while (1) {
	//    k_sleep(K_SECONDS(100));
	//    if (my_timer.status == 0){
	//      printk("Ok");
	//    }
	// 	// if ((k_timer_status_get(&my_timer)) > 0) {
	// 	// 	printk("Timer expired\n");
	// 	// } else if (k_timer_remaining_get(&my_timer) == 0) {
	// 	// 	printk("Time stopped\n");
	// 	// } else {
	// 	// 	printk("Timer still running\n");
	// 	// }
	// }

	return 0;
}
