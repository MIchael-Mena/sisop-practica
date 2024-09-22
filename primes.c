#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define READ 0  // file descriptor de lectura
#define WRITE 1 // file descriptor de escritura

void verify_error(int res, const char *msg)
{
  if (res < 0)
  {
    perror(msg);
    exit(EXIT_FAILURE);
  }
}

void filter(int fd_in)
{
  int a = 0, res = 0;
  // int res = read(fd_in, &a, sizeof(a));
  verify_error(res = read(fd_in, &a, sizeof(a)), "read filter");
  if (res == 0)
  {
    close(fd_in);
    exit(EXIT_SUCCESS); // Termina todo la aplicación (el proceso actual y sus hijos)
  }

  printf("primo: %d\n", a);
  // fflush(stdout); // Si se ejecuta en docker, se debe usar esta función para que se imprima en pantalla

  int fds_filter[2];
  verify_error(pipe(fds_filter), "filter pipe");

  pid_t pid_filter = 0;
  verify_error(pid_filter = fork(), "filter fork");

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
        if (write(fds_filter[WRITE], &b, sizeof(b)) < 0)
        {
          perror("write filter");
          exit(EXIT_FAILURE);
        }
      }
    }
    close(fds_filter[WRITE]);
    close(fd_in);
    wait(NULL); // Espero a que el hijo termine
  }
}

long validate_and_convert_args(int argc, char *argv[])
{
  if (argc != 2)
  {
    fprintf(stderr, "Modo de uso: %s <number>\n", argv[0]); // stderr es el canal de error
    // Se usa fprintf para poder imprimir en stderr, ya que printf imprime en stdout
    // si el error fuera generado por alguna syscall lo mejor seria usar perror ya que errno tendría el
    // valor correspondiente al error generado (en este caso es mejor usar fprintf)
    // return 1; // Solo termina la función actual, si se usara despues de un fork, el proceso hijo seguiría ejecutándose
    exit(EXIT_FAILURE); // Termina el proceso actual, mas fuerte que return
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
  verify_error(pipe(fds), "main pipe");

  pid_t pid = 0;
  verify_error(pid = fork(), "main fork");

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
      if (write(fds[WRITE], &i, sizeof(i)) < 0)
      {
        perror("write main");
        exit(EXIT_FAILURE);
      }
    }
    close(fds[WRITE]); // Cierro el extremo de escritura
    wait(NULL);        // Espero a que el hijo termine
  }
  return 0; // La función main debe retornar un entero
}