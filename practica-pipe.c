#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  // printf("La cantidad de argumentos es %d y el primer argumento es %s\n", argc, argv[1]);
  // Cuando se ejecute el archivo compilado ej: ./my_program 42
  // argc = 2, argv[0] = "./my_program", argv[1] = "42"
  int fds[2];
  int msg = 42;

  int r = pipe(fds);

  if (r < 0)
  {
    perror("Error en pipe");
    exit(-1);
  }

  printf("Lectura: %d, Escritura: %d\n", fds[0], fds[1]);

  // read(fds[0], &msg, sizeof(msg)); // Detiene la ejecucion hasta que halla algo para leer del pipe

  // Escribo en el pipe
  write(fds[1], &msg, sizeof(msg));

  int recibido = 0;
  // Lee lo que este en el pipe y lo guarda en la direccion de memoria de 'recibido'
  read(fds[0], &recibido, sizeof(recibido));
  printf("Recibi: %d\n", recibido);

  close(fds[0]);
  close(fds[1]);
}
