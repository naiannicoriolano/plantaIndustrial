#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ID_LEN 50
#define MAX_VAL_LEN 100
#define MAX_SENSORS 20 
#define INITIAL_CAPACITY 2000 

typedef struct {
    time_t timestamp;
    char id_sensor[MAX_ID_LEN];
    char valor[MAX_VAL_LEN];
} Leitura;

typedef struct {
    char id_sensor[MAX_ID_LEN];
    Leitura *leituras;
    int count;
    int capacity;
} SensorData;

int comparar_leituras(const void *a, const void *b) {
    Leitura *leituraA = (Leitura *)a;
    Leitura *leituraB = (Leitura *)b;
    if (leituraA->timestamp < leituraB->timestamp) return -1;
    if (leituraA->timestamp > leituraB->timestamp) return 1;
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <arquivo_bruto>\n", argv[0]);
        return 1;
    }

    FILE *infile = fopen(argv[1], "r");
    if (!infile) {
        perror("Erro ao abrir arquivo de entrada");
        return 1;
    }

    SensorData sensores[MAX_SENSORS];
    int num_sensores_unicos = 0;

    long ts_long; 
    char current_id[MAX_ID_LEN];
    char current_val[MAX_VAL_LEN];

    for(int i=0; i<MAX_SENSORS; ++i) {
        sensores[i].leituras = NULL;
        sensores[i].count = 0;
        sensores[i].capacity = 0;
        strcpy(sensores[i].id_sensor, "");
    }

    while (fscanf(infile, "%ld %49s %99[^\n]", &ts_long, current_id, current_val) == 3) {
        int sensor_idx = -1;
        for (int i = 0; i < num_sensores_unicos; i++) {
            if (strcmp(sensores[i].id_sensor, current_id) == 0) {
                sensor_idx = i;
                break;
            }
        }

        if (sensor_idx == -1) { 
            if (num_sensores_unicos >= MAX_SENSORS) {
                printf("Numero maximo de sensores unicos (%d) atingido. Ignorando %s.\n", MAX_SENSORS, current_id);
                continue;
            }
            sensor_idx = num_sensores_unicos;
            strcpy(sensores[sensor_idx].id_sensor, current_id);
            sensores[sensor_idx].leituras = malloc(INITIAL_CAPACITY * sizeof(Leitura));
            if (!sensores[sensor_idx].leituras) {
                perror("Falha ao alocar memoria para leituras");
                for(int k=0; k < num_sensores_unicos; ++k) free(sensores[k].leituras);
                fclose(infile);
                return 1;
            }
            sensores[sensor_idx].capacity = INITIAL_CAPACITY;
            sensores[sensor_idx].count = 0;
            num_sensores_unicos++;
        }

        if (sensores[sensor_idx].count >= sensores[sensor_idx].capacity) {
            int new_capacity = sensores[sensor_idx].capacity * 2;
            Leitura *temp = realloc(sensores[sensor_idx].leituras, new_capacity * sizeof(Leitura));
            if (!temp) {
                perror("Falha ao realocar memoria para leituras");
                printf("Pulando leitura para sensor %s devido a falha de memoria.\n", current_id);
                continue; 
            }
            sensores[sensor_idx].leituras = temp;
            sensores[sensor_idx].capacity = new_capacity;
        }
        
        Leitura nova_leitura;
        nova_leitura.timestamp = (time_t)ts_long;
        strcpy(nova_leitura.id_sensor, current_id); 
        strcpy(nova_leitura.valor, current_val);
        
        sensores[sensor_idx].leituras[sensores[sensor_idx].count++] = nova_leitura;
    }
    fclose(infile);

    for (int i = 0; i < num_sensores_unicos; i++) {
        qsort(sensores[i].leituras, sensores[i].count, sizeof(Leitura), comparar_leituras);

        char out_filename[MAX_ID_LEN + 4]; 
        sprintf(out_filename, "%s.txt", sensores[i].id_sensor);
        
        FILE *outfile = fopen(out_filename, "w");
        if (!outfile) {
            perror("Erro ao criar arquivo de saida para sensor");
            printf("Nao foi possivel criar o arquivo para o sensor: %s\n", sensores[i].id_sensor);
            continue; 
        }

        for (int j = 0; j < sensores[i].count; j++) {
            fprintf(outfile, "%ld %s %s\n",
                    (long)sensores[i].leituras[j].timestamp,
                    sensores[i].leituras[j].id_sensor, 
                    sensores[i].leituras[j].valor);
        }
        fclose(outfile);
        printf("Dados do sensor %s salvos em %s\n", sensores[i].id_sensor, out_filename);
        free(sensores[i].leituras); 
    }

    printf("Processamento concluido.\n");
    return 0;
}