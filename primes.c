#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define READ 0  // file descriptor de lectura
#define WRITE 1 // file descriptor de escritura

void filter(int fd_in)
{
  int a;
  int res = read(fd_in, &a, sizeof(a));
  if (res == 0)
  {
    close(fd_in);
    exit(EXIT_SUCCESS); // Termina todo la aplicación (el proceso actual y sus hijos)
  }
  else if (res < 0)
  {
    perror("read");
    exit(EXIT_FAILURE);
  }

  printf("primo: %d\n", a);
  // fflush(stdout); // Si se ejecuta en docker, se debe usar esta función para que se imprima en pantalla

  int fds_filter[2];
  if (pipe(fds_filter) < 0)
  {
    perror("filter pipe");
    exit(EXIT_FAILURE);
  }

  pid_t pid_filter = fork();

  if (pid_filter == 0)
  {
    // Hijo
    close(fd_in);
    close(fds_filter[WRITE]);
    filter(fds_filter[READ]);
  }
  else if (pid_filter > 0)
  {
    // Padre
    int b;
    close(fds_filter[READ]);
    while (read(fd_in, &b, sizeof(b)) > 0)
    { // Si read retorna 0 significa que el pipe está cerrado
      if (b % a != 0)
      { // Si el número no es divisible por el primo actual lo envío al siguiente pipe
        write(fds_filter[WRITE], &b, sizeof(b));
      }
    }
    close(fds_filter[WRITE]);
    close(fd_in);
    wait(NULL); // Espero a que el hijo termine
  }
  else
  {
    perror("filter fork");
    exit(EXIT_FAILURE);
  }
}

long validate_and_convert_args(int argc, char *argv[])
{
  if (argc < 2)
  {
    fprintf(stderr, "Modo de uso: %s <number>\n", argv[0]); // stderr es el canal de error
    // return 1; // Solo termina la función actual, si se usara despues de un fork, el proceso hijo seguiría ejecutándose
    exit(EXIT_FAILURE);
  }
  // int n = atoi(argv[1]); // atoi no maneja errores
  char *endptr;
  errno = 0; // Para distinguir errores de conversión 'strtol', de errores de lectura
  long n = strtol(argv[1], &endptr, 10);
  if (errno != 0 || *endptr != '\0')
  {
    fprintf(stderr, "El argumento debe ser un número entero\n");
    exit(EXIT_FAILURE);
  }

  if (n < 2)
  {
    fprintf(stderr, "El número debe ser mayor a 1\n");
    exit(EXIT_FAILURE);
  }

  return n;
}

int main(int argc, char *argv[])
{
  long n = validate_and_convert_args(argc, argv);

  int fds[2];
  if (pipe(fds) < 0)
  {
    perror("main pipe");
    exit(EXIT_FAILURE);
  }

  pid_t pid = fork();

  if (pid == 0)
  {
    // Hijo
    close(fds[WRITE]);
    filter(fds[READ]);
  }
  else if (pid > 0)
  {
    // Padre crea los números
    close(fds[READ]); // Cierro el extremo de lectura
    for (int i = 2; i <= n; i++)
    {
      // Los pipe son bloqueantes por lo que no se pasara al siguiente número hasta que el hijo haya leído el anterior
      write(fds[WRITE], &i, sizeof(i));
    }
    close(fds[WRITE]); // Cierro el extremo de escritura
    wait(NULL);        // Espero a que el hijo termine
  }
  else
  {
    perror("main fork");
    exit(EXIT_FAILURE);
  }
  return 0; // La función main debe retornar un entero
}
