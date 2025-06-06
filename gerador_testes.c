#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

time_t converter_para_timestamp(int dia, int mes, int ano, int hora, int min, int seg) {
    struct tm t;
    t.tm_year = ano - 1900;
    t.tm_mon = mes - 1;
    t.tm_mday = dia;
    t.tm_hour = hora;
    t.tm_min = min;
    t.tm_sec = seg;
    t.tm_isdst = -1;
    time_t timestamp = mktime(&t);
    return timestamp;
}

time_t gerar_timestamp_aleatorio(struct tm *inicial_tm_ptr, struct tm *final_tm_ptr) {
    time_t timestamp_inicial, timestamp_final;
    struct tm t_inicial = *inicial_tm_ptr;
    struct tm t_final = *final_tm_ptr;
    timestamp_inicial = mktime(&t_inicial);
    timestamp_final = mktime(&t_final);
    time_t diff = timestamp_final - timestamp_inicial;
    if (diff < 0) return -1;
    if (diff == 0) return timestamp_inicial;
    time_t random_offset = rand() % (diff + 1);
    time_t timestamp_aleatorio = timestamp_inicial + random_offset;
    return timestamp_aleatorio;
}

// Le argumentos, valida e gera o arquivo de teste.
int main(int argc, char *argv[]) {
    if (argc < 14) {
        printf("Erro: Faltam argumentos de data ou hora.\n");
        printf("O correto seria: %s <arquivo_saida> <ano_i> <mes_i> <dia_i> <h_i> <m_i> <s_i> <ano_f> <mes_f> <dia_f> <h_f> <m_f> <s_f> <ID_SENSOR_1> <TIPO_DADO_1> ...\n", argv[0]);
        return 1;
    }
    int arg_sensores_inicio = 14;
    if ((argc - arg_sensores_inicio) % 2 != 0) {
        printf("Erro: Cada sensor deve ter um ID e um TIPO correspondente.\n");
        return 1;
    }
    if (argc < 16) {
        printf("Erro: E necessario fornecer no minimo um par de SENSOR e TIPO.\n");
        return 1;
    }

    // Valida os valores de data e hora
    struct tm tm_inicio = {0};
    int ano_i = atoi(argv[2]), mes_i = atoi(argv[3]), dia_i = atoi(argv[4]);
    int hora_i = atoi(argv[5]), min_i = atoi(argv[6]), seg_i = atoi(argv[7]);

    if (hora_i < 0 || hora_i > 23 || min_i < 0 || min_i > 59 || seg_i < 0 || seg_i > 59) {
        printf("Erro: Hora, minuto ou segundo de INICIO fora do intervalo valido (H:0-23, M/S:0-59).\n");
        return 1;
    }
    tm_inicio.tm_year = ano_i - 1900; tm_inicio.tm_mon = mes_i - 1; tm_inicio.tm_mday = dia_i;
    tm_inicio.tm_hour = hora_i; tm_inicio.tm_min = min_i; tm_inicio.tm_sec = seg_i;
    tm_inicio.tm_isdst = -1;
    time_t ts_check_inicio = mktime(&tm_inicio);
    if (ts_check_inicio == -1 || tm_inicio.tm_mday != dia_i || tm_inicio.tm_mon != mes_i - 1) {
        printf("Erro: Data de INICIO invalida ou nao existente (ex: 31 de Abril).\n");
        return 1;
    }

    struct tm tm_fim = {0};
    int ano_f = atoi(argv[8]), mes_f = atoi(argv[9]), dia_f = atoi(argv[10]);
    int hora_f = atoi(argv[11]), min_f = atoi(argv[12]), seg_f = atoi(argv[13]);

    if (hora_f < 0 || hora_f > 23 || min_f < 0 || min_f > 59 || seg_f < 0 || seg_f > 59) {
        printf("Erro: Hora, minuto ou segundo de FIM fora do intervalo valido (H:0-23, M/S:0-59).\n");
        return 1;
    }
    tm_fim.tm_year = ano_f - 1900; tm_fim.tm_mon = mes_f - 1; tm_fim.tm_mday = dia_f;
    tm_fim.tm_hour = hora_f; tm_fim.tm_min = min_f; tm_fim.tm_sec = seg_f;
    tm_fim.tm_isdst = -1;
    time_t ts_check_fim = mktime(&tm_fim);
    if (ts_check_fim == -1 || tm_fim.tm_mday != dia_f || tm_fim.tm_mon != mes_f - 1) {
        printf("Erro: Data de FIM invalida ou nao existente (ex: 31 de Abril).\n");
        return 1;
    }

    if (ts_check_fim < ts_check_inicio) {
        printf("Erro: A data de fim nao pode ser anterior a data de inicio.\n");
        return 1;
    }

    char *output_filename = argv[1];
    FILE *outfile = fopen(output_filename, "w");
    if (!outfile) {
        perror("Erro ao abrir arquivo de saida");
        return 1;
    }

    srand(time(NULL));
    int num_leituras_por_sensor = 2000;
    
    for (int i = arg_sensores_inicio; i < argc; i += 2) {
        char *sensor_id = argv[i];
        char *sensor_type = argv[i+1];

        // Valida o tipo do sensor antes do loop de 2000 repeticoes
        int tipo_valido = 0;
        if (strcmp(sensor_type, "int") == 0 || strcmp(sensor_type, "float") == 0 ||
            strcmp(sensor_type, "bool") == 0 || strcmp(sensor_type, "string") == 0) {
            tipo_valido = 1;
        }

        if (!tipo_valido) {
            printf("Aviso: Tipo de sensor desconhecido '%s' para o sensor '%s'. Nenhuma leitura sera gerada para ele.\n", sensor_type, sensor_id);
            continue; 
        }

        for (int j = 0; j < num_leituras_por_sensor; j++) {
            time_t random_ts = gerar_timestamp_aleatorio(&tm_inicio, &tm_fim);
            
            if (strcmp(sensor_type, "int") == 0) {
                fprintf(outfile, "%ld %s %d\n", (long)random_ts, sensor_id, rand() % 1000);
            } else if (strcmp(sensor_type, "float") == 0) {
                fprintf(outfile, "%ld %s %.2f\n", (long)random_ts, sensor_id, (float)rand() / RAND_MAX * 100.0f);
            } else if (strcmp(sensor_type, "bool") == 0) {
                fprintf(outfile, "%ld %s %d\n", (long)random_ts, sensor_id, rand() % 2);
            } else if (strcmp(sensor_type, "string") == 0) {
                fprintf(outfile, "%ld %s MSG_%d\n", (long)random_ts, sensor_id, rand() % 100);
            }
        }
    }

    fclose(outfile);
    printf("Arquivo de teste '%s' gerado com sucesso.\n", output_filename);

    return 0;
}