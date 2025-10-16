# Atividades de Sistemas Embarcados

## Atividade 3

Comunicação entre Threads com Produtores, Filtro Intermediário e Consumidor

Contexto:

Em uma estufa inteligente, dois sensores capturam informações ambientais:

    Produtor 1: Sensor de Temperatura
    Produtor 2: Sensor de Umidade


Os dados de ambos os sensores são enviados para uma fila de entrada.

Uma thread de Filtro lê da fila de entrada, aplica as regras de validação e então envia os dados para outra fila de saída, que será consumida por uma thread Consumidora.

Fluxo:

Produtores (Temperatura e Umidade)

    Capturam os dados dos sensores.
    Enviam para a fila de entrada (msg_queue).
    Filtro (thread intermediária)
    Lê dados da fila de entrada.
    Aplica regras de validação.
    Encaminha dados válidos para a fila de saída.
    Encaminha dados inválidos para um canal separado de log/erro.


Consumidor

    Lê dados apenas da fila de saída.
    Armazena ou exibe os dados finais considerados corretos.


Regras do Filtro:

    Temperatura: só aceita valores entre 18 °C e 30 °C.
    Umidade: só aceita valores entre 40% e 70%.
    Fora desses intervalos → encaminhar como inconsistentes para log.


Critérios de Avaliação:

    Precisa estar compilando.
    Separação clara das responsabilidades (produção, filtragem e consumo).
    Comunicação entre threads usando duas filas (msg_queue).
    Implementação correta da lógica de validação na thread intermediária.
    Organização do fluxo de dados válidos e inválidos
