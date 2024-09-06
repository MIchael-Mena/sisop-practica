#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

// interfaz: $ ./xargs <comando>
// Donde comando es un binario que no recibe argumentos extras (como por ejemplo ls o echo).
// Ejemplo de uso: $ seq 9 | ./xargs echo
// salida esperada: 1 2 3 4
//                  5 6 7 8
//                  9

// NARGS es el número de argumentos que se pasan a la función execvp
#ifndef NARGS
#define NARGS 4
#endif
// Si no se define NARGS, se asume que el número de argumentos es 4
// Se puede cambiar el valor de NARGS al compilar con -DNARGS=valor (es una macro)

void remove_newline(char *str)
{
  char *newline = strchr(str, '\n');
  if (newline)
  {
    *newline = '\0';
  }
}

void free_args(char *args[])
{
  for (int i = 1; args[i] != NULL; i++)
  {
    free(args[i]);
    args[i] = NULL;
  }
}

void execute_command(char *args[], char *line)
{
  pid_t pid = fork(); // Crear un nuevo proceso

  if (pid < 0)
  {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  else if (pid == 0)
  {
    // Hijo
    // Se llama a execvp que cambia la imagen del proceso actual con una nueva imagen de proceso
    // execvp recibe el nombre del programa a ejecutar y un arreglo de strings con los argumentos
    if (execvp(args[0], args) < 0)
    {
      // Libero la memoria pedida solo si execvp falla ya que el proceso hijo se reemplaza
      // por el nuevo proceso (incluyendo el stack, heap, etc, excepto el PID y los file descriptors)
      free_args(args);
      free(line);
      perror("execvp");
      exit(EXIT_FAILURE);
    }
  }
  else
  {
    // Padre
    wait(NULL);
    free_args(args);
  }
}

void read_and_execute_commands(char *args[])
{
  char *line = NULL;
  size_t len = 0;
  int i = 1; // El primer argumento ya está ocupado por el comando

  // Itero mientras no se llegue al final del archivo
  while (getline(&line, &len, stdin) != -1)
  {
    remove_newline(line); // Eliminar el carácter de nueva línea de la línea leída

    args[i] = strdup(line); // Duplica la cadena line y la asigna a args[i]

    // Empaquetar los NARGS argumentos y ejecutar el comando
    if (i == NARGS)
    {
      i++;
      args[i] = NULL; // Marcar el final de los argumentos
      execute_command(args, line);
      i = 1;
    }
    else
    {
      i++;
    }
  }

  // Si quedan argumentos por ejecutar
  if (i > 1)
  {
    args[i] = NULL;
    execute_command(args, line);
  }

  free(line); // Liberar la memoria asignada por getline (solo el padre llega a este punto)
}

int main(int argc, char *argv[])
{
  // Verificar que se haya pasado un argumento
  if (argc < 2)
  {
    fprintf(stderr, "Uso: %s <comando>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  remove_newline(argv[1]); // Eliminar el carácter de nueva línea del comando

  char *args[NARGS + 2]; // +2 espacios, uno para el comando y otro el NULL final
  args[0] = argv[1];

  read_and_execute_commands(args);

  return 0;
}