#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

void finalizar_padre(void) {
    printf("\n[atexit] Proceso padre (PID: %d) finalizando correctamente.\n", getpid());
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "ERROR: Uso correcto: %s <N>\n", argv[0]);
        fprintf(stderr, "Donde N es la cantidad de procesos hijos a crear.\n");
        return 1;
    }

    int N = atoi(argv[1]);
    
    if (N <= 0) {
        fprintf(stderr, "ERROR: N debe ser un entero positivo (mayor a 0).\n");
        return 1;
    }

    printf("=== INICIO DEL ORQUESTADOR ===\n");
    printf("Proceso padre PID: %d\n", getpid());
    printf("Creando %d procesos hijos...\n\n", N);

    if (atexit(finalizar_padre) != 0) {
        perror("ERROR: atexit");
        return 1;
    }

    srand(time(NULL));

    pid_t hijos[N];

    for (int i = 0; i < N; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("ERROR: fork falló");
            return 1;
        }

        if (pid == 0) {
            printf("[HIJO %d] Mi PID es: %d, mi padre es: %d\n", i + 1, getpid(), getppid());
            
            int tiempo_sleep = (rand() % 5) + 1;
            printf("[HIJO %d] Simulando tarea por %d segundos...\n", i + 1, tiempo_sleep);
            sleep(tiempo_sleep);
            
            printf("[HIJO %d] Tarea completada. Finalizando con exit(%d)\n", i + 1, i + 1);
            exit(i + 1);
        } else {
            hijos[i] = pid;
            printf("[PADRE] Hijo %d creado con PID: %d\n", i + 1, pid);
        }
    }

    printf("\n=== ESPERANDO FINALIZACIÓN DE HIJOS ===\n");
    
    for (int i = 0; i < N; i++) {
        int status;
        pid_t pid_finalizado;
        
        pid_finalizado = waitpid(hijos[i], &status, 0);
        
        if (pid_finalizado == -1) {
            perror("ERROR: waitpid falló");
            continue;
        }
        
        if (WIFEXITED(status)) {
            int codigo_salida = WEXITSTATUS(status);
            printf("[PADRE] waitpid(): Hijo con PID %d finalizó con código %d\n", pid_finalizado, codigo_salida);
        } else {
            printf("[PADRE] waitpid(): Hijo con PID %d terminó anormalmente\n", pid_finalizado);
        }
    }

    printf("\n=== TODOS LOS HIJOS HAN FINALIZADO ===\n");
    return 0;
}