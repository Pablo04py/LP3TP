#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t contador_sigusr1 = 0;

void manejador_sigint(int sig) {
    printf("\n[SIGINT] Ctrl+C presionado, pero el programa continua...\n");
}

void manejador_sigterm(int sig) {
    printf("\n[SIGTERM] Liberando recursos y finalizando de forma segura.\n");
    exit(0); 
}

void manejador_sigusr1(int sig) {
    contador_sigusr1++;
    printf("\n[SIGUSR1] Contador incrementado a: %d\n", contador_sigusr1);
}

int main() {
    struct sigaction sa_int, sa_term, sa_usr;
    printf("--- Iniciando Monitor de Eventos (PID: %d) ---\n", getpid());        
    
    sa_int.sa_handler = manejador_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    sa_term.sa_handler = manejador_sigterm;
    sigemptyset(&sa_term.sa_mask);
    sa_term.sa_flags = 0;
    sigaction(SIGTERM, &sa_term, NULL);

    sa_usr.sa_handler = manejador_sigusr1;
    sigemptyset(&sa_usr.sa_mask);
    sa_usr.sa_flags = 0;
    sigaction(SIGUSR1, &sa_usr, NULL);

    sigset_t conjunto_bloqueo;
    sigemptyset(&conjunto_bloqueo);
    sigaddset(&conjunto_bloqueo, SIGINT);
    sigaddset(&conjunto_bloqueo, SIGUSR1);

    printf("\n--- Bloqueando SIGINT y SIGUSR1 temporalmente ---\n");
    sigprocmask(SIG_BLOCK, &conjunto_bloqueo, NULL);

    printf("Realizando trabajo critico por 5 segundos...\n");
    printf("--> TIP: Intenta presionar Ctrl+C ahora mismo para ver que sucede\n");
    sleep(5);

    printf("\n--- Desbloqueando señales ---\n");
    sigprocmask(SIG_UNBLOCK, &conjunto_bloqueo, NULL);

    printf("\n--- Generando señal interna con raise() ---\n");
    raise(SIGUSR1); 

    printf("\n--- Ignorando SIGINT temporalmente ---\n");
    struct sigaction sa_ign, sa_old;
    sa_ign.sa_handler = SIG_IGN;
    sigemptyset(&sa_ign.sa_mask);
    sa_ign.sa_flags = 0;
    sigaction(SIGINT, &sa_ign, &sa_old);

    printf("Intenta presionar Ctrl+C ahora (no pasara nada). Tienes 3 segundos...\n");
    sleep(3);

    printf("Restaurando el comportamiento de SIGINT...\n");
    sigaction(SIGINT, &sa_old, NULL);

    printf("\n--- Prueba 1: Esperando con pause() ---\n");
    printf("Presiona Ctrl+C para continuar...\n");
    pause(); 

    printf("\n--- Prueba 2: Esperando con sigsuspend() ---\n");
    printf("Presiona Ctrl+C de nuevo para continuar...\n");
    sigset_t mascara_vacia;
    sigemptyset(&mascara_vacia);
    sigsuspend(&mascara_vacia);

    printf("\n--- Prueba 3: Esperando con sigwait() ---\n");
    sigset_t conjunto_wait;
    sigemptyset(&conjunto_wait);
    sigaddset(&conjunto_wait, SIGUSR1);
    sigprocmask(SIG_BLOCK, &conjunto_wait, NULL);

    printf("Generando otra SIGUSR1 interna para que sigwait la atrape en silencio...\n");
    raise(SIGUSR1);

    int senal_atrapada;
    sigwait(&conjunto_wait, &senal_atrapada);
    printf("sigwait() atrapo la señal numero %d exitosamente\n", senal_atrapada);

    printf("\n--- Enviando SIGTERM para finalizar el programa ---\n");
    raise(SIGTERM); 
    
    return 0;
}
