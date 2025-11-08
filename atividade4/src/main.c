#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_core.h>
#include <zephyr/net/sntp.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <time.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* Estrutura para mensagens de horário */
struct time_msg {
	int64_t timestamp_ms;
	struct tm time_info;
	bool sync_success;
};

/* Canal ZBus */
ZBUS_CHAN_DEFINE(time_channel,
		 struct time_msg,
		 NULL,
		 NULL,
		 ZBUS_OBSERVERS(logger_sub, app_sub),
		 ZBUS_MSG_INIT(0)
);

/* THREAD 1: SNTP CLIENT */
static void sntp_client_thread(void)
{
	struct sntp_time sntp_time;
	struct time_msg msg;
	int ret;
	int sync_count = 0;
	int fail_count = 0;

	LOG_INF("SNTP Client iniciado");
	LOG_INF("Servidor: %s", CONFIG_SNTP_SERVER);
	LOG_INF("Intervalo: %d segundos", CONFIG_SNTP_SYNC_INTERVAL);

	/* Aguarda interface de rede estar pronta */
	LOG_INF("[SNTP] Aguardando interface de rede...");
	k_sleep(K_SECONDS(3));

	while (1) {
		LOG_INF("[SNTP] Tentando sincronizar com %s...",
			CONFIG_SNTP_SERVER);

		/* Tenta sincronização SNTP real - timeout de 10 segundos */
		ret = sntp_simple(CONFIG_SNTP_SERVER, 10000, &sntp_time);

		if (ret == 0) {
			/* Sincronização bem-sucedida! */
			sync_count++;
			fail_count = 0;

			/* Converte para timestamp e struct tm */
			time_t unix_time = (time_t)sntp_time.seconds;
			msg.timestamp_ms = (int64_t)sntp_time.seconds * 1000;
			msg.sync_success = true;

			gmtime_r(&unix_time, &msg.time_info);

			LOG_INF("[SNTP] Sincronizacao REAL bem-sucedida (#%d)",
				sync_count);
			LOG_INF("[SNTP] Horario: %04d-%02d-%02d %02d:%02d:%02d UTC",
				msg.time_info.tm_year + 1900,
				msg.time_info.tm_mon + 1,
				msg.time_info.tm_mday,
				msg.time_info.tm_hour,
				msg.time_info.tm_min,
				msg.time_info.tm_sec);

			/* Publica no ZBus */
			ret = zbus_chan_pub(&time_channel, &msg, K_MSEC(100));

			if (ret == 0) {
				LOG_INF("[SNTP] Publicado no ZBus");
			} else {
				LOG_ERR("[SNTP] Erro ao publicar no ZBus: %d", ret);
			}

		} else {
			/* Falha na sincronização */
			fail_count++;
			LOG_WRN("[SNTP] Falha na sincronizacao (erro: %d)", ret);
			LOG_WRN("[SNTP] Total de falhas consecutivas: %d", fail_count);

			if (fail_count == 1) {
				LOG_WRN("[SNTP] Possíveis causas:");
				LOG_WRN("[SNTP]   - Rede SLIP não configurada");
				LOG_WRN("[SNTP]   - DNS não resolvendo");
				LOG_WRN("[SNTP]   - Servidor indisponível");
				LOG_WRN("[SNTP]   - Execute: sudo ./setup-slip.sh");
			}
		}

		LOG_INF("");

		/* Aguarda próximo ciclo */
		k_sleep(K_SECONDS(CONFIG_SNTP_SYNC_INTERVAL));
	}
}

K_THREAD_DEFINE(sntp_thread, 4096, sntp_client_thread,
		NULL, NULL, NULL, 5, 0, 0);

/* THREAD 2: LOGGER */
static void logger_listener(const struct zbus_channel *chan)
{
	const struct time_msg *msg = zbus_chan_const_msg(chan);

	LOG_INF("[LOGGER] Mensagem recebida via ZBus");
	LOG_INF("[LOGGER] Timestamp: %lld ms", msg->timestamp_ms);
	LOG_INF("[LOGGER] Data/Hora: %04d-%02d-%02d %02d:%02d:%02d UTC",
		msg->time_info.tm_year + 1900,
		msg->time_info.tm_mon + 1,
		msg->time_info.tm_mday,
		msg->time_info.tm_hour,
		msg->time_info.tm_min,
		msg->time_info.tm_sec);
	LOG_INF("");
}

ZBUS_LISTENER_DEFINE(logger_sub, logger_listener);

/* THREAD 3: APPLICATION */
static int64_t last_event_time = 0;
static int event_count = 0;

static void app_listener(const struct zbus_channel *chan)
{
	const struct time_msg *msg = zbus_chan_const_msg(chan);
	int64_t current_time = msg->timestamp_ms;

	LOG_INF("[APP] Processando evento");

	event_count++;

	if (last_event_time == 0) {
		last_event_time = current_time;
		LOG_INF("[APP] Primeiro evento registrado!");
	} else {
		int64_t elapsed_ms = current_time - last_event_time;
		int64_t elapsed_sec = elapsed_ms / 1000;

		LOG_INF("[APP] Intervalo: %lld segundos", elapsed_sec);
		last_event_time = current_time;
	}

	LOG_INF("[APP] Horario: %02d:%02d:%02d",
		msg->time_info.tm_hour,
		msg->time_info.tm_min,
		msg->time_info.tm_sec);
	LOG_INF("[APP] Total de eventos: %d", event_count);
	LOG_INF("");
}

ZBUS_LISTENER_DEFINE(app_sub, app_listener);

/* MAIN */
int main(void)
{
	LOG_INF("Configuracao:");
	LOG_INF("  - Servidor: %s", CONFIG_SNTP_SERVER);
	LOG_INF("  - Intervalo: %d segundos", CONFIG_SNTP_SYNC_INTERVAL);

	return 0;
}
