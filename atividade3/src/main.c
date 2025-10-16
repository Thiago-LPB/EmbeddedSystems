#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/printk.h>

#define STACK_SIZE 1024
#define MSG_SIZE sizeof(struct sensor_data)
#define QUEUE_LEN 10

struct sensor_data {
	char type;
	float value;
};

K_MSGQ_DEFINE(queue_in, MSG_SIZE, QUEUE_LEN, 4);
K_MSGQ_DEFINE(queue_out, MSG_SIZE, QUEUE_LEN, 4);


static void float_to_str(char *buf, size_t size, float val)
{
	int int_part = (int)val;
	int frac_part = (int)((val - int_part) * 100.0f);
	if (frac_part < 0) {
		frac_part = -frac_part;
	}
	snprintk(buf, size, "%d.%02d", int_part, frac_part);
}

void temp_producer(void)
{
	struct sensor_data msg;
	char val_str[16];

	while (1) {
		uint32_t r = sys_rand32_get();
		msg.type = 'T';
		msg.value = 10.0f + (r % 3000) / 100.0f; // Valor ente 10 e 40

		k_msgq_put(&queue_in, &msg, K_NO_WAIT);

		float_to_str(val_str, sizeof(val_str), msg.value);
		printk("[TEMP] Sent %s°C\n", val_str);

		k_msleep(1500);
	}
}

void humid_producer(void)
{
	struct sensor_data msg;
	char val_str[16];

	while (1) {
		uint32_t r = sys_rand32_get();
		msg.type = 'H';
		msg.value = 10.0f + (r % 7000) / 100.0f; // Valor entre 10 e 80

		k_msgq_put(&queue_in, &msg, K_NO_WAIT);

		float_to_str(val_str, sizeof(val_str), msg.value);
		printk("[HUM] Sent %s%%\n", val_str);

		k_msleep(2000);
	}
}

void filter_thread(void)
{
	struct sensor_data msg;
	char val_str[16];

	while (1) {
		if (k_msgq_get(&queue_in, &msg, K_FOREVER) == 0) {
			bool valid = false;

			if (msg.type == 'T' && msg.value >= 18.0f && msg.value <= 30.0f) {
				valid = true;
			} else if (msg.type == 'H' && msg.value >= 40.0f && msg.value <= 70.0f) {
				valid = true;
			}

			float_to_str(val_str, sizeof(val_str), msg.value);

			if (valid) {
				k_msgq_put(&queue_out, &msg, K_NO_WAIT);
			} else {
				printk("[ERROR] Out of range: %c = %s\n", msg.type, val_str);
			}
		}
	}
}

void consumer_thread(void)
{
	struct sensor_data msg;
	char val_str[16];

	while (1) {
		if (k_msgq_get(&queue_out, &msg, K_FOREVER) == 0) {
			float_to_str(val_str, sizeof(val_str), msg.value);

			if (msg.type == 'T') {
				printk("[OK] Valid temperature: %s°C\n", val_str);
			} else {
				printk("[OK] Valid humidity: %s%%\n", val_str);
			}
		}
	}
}

K_THREAD_DEFINE(th_temp, STACK_SIZE, temp_producer, NULL, NULL, NULL, 2, 0, 0);
K_THREAD_DEFINE(th_humid, STACK_SIZE, humid_producer, NULL, NULL, NULL, 2, 0, 0);
K_THREAD_DEFINE(th_filter, STACK_SIZE, filter_thread, NULL, NULL, NULL, 3, 0, 0);
K_THREAD_DEFINE(th_consumer, STACK_SIZE, consumer_thread, NULL, NULL, NULL, 4, 0, 0);
