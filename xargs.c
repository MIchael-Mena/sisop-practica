#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

// interfaz: $ ./xargs <comando>
// donde comando es un binario que no recibe argumentos extras (como por ejemplo ls o echo).

// NARGS es el número de argumentos que se pasan a la función execvp
#ifndef NARGS
#define NARGS 4
#endif
// Si no se define NARGS, se asume que el número de argumentos es 4
// Se puede cambiar el valor de NARGS al compilar con -DNARGS=valor (es una macro)

int main(int argc, char *argv[])
{
  char *args[NARGS];

  char *line = NULL; // Puntero inicializado a NULL
  size_t len = 0;    // Tamaño inicial del buffer
  size_t read;       // Número de caracteres leídos

  char *newline = strchr(argv[1], '\n');
  if (newline)
  {
    *newline = '\0';
  }

  int i = 0;
  args[i] = argv[1];
  i++;
  while (i < NARGS && getline(&line, &len, stdin) != -1)
  {
    // line es puntero a un char que se va reasignando en cada iteración
    newline = strchr(line, '\n'); // Devuelve un puntero al primer caracter '\n' encontrado en la cadena
    if (newline)
    {
      *newline = '\0';
    }

    args[i] = strdup(line);
    i++;

    if (i == NARGS)
    {
      args[i] = NULL;

      pid_t pid = fork();

      if (pid < 0)
      {
        perror("fork");
        exit(EXIT_FAILURE);
      }
      else if (pid == 0)
      {
        // Hijo
        free(line);                    // Liberar la memoria asignada por getline
        if (execvp(argv[1], args) < 0) // reemplaza la imagen del proceso actual con una nueva imagen de proceso
        {
          // Liberar la memoria duplicada, solo si execvp falla
          // ya que si no falla, el proceso hijo se reemplaza por el nuevo proceso
          for (int j = 0; j < i; j++)
          {
            free(args[j]);
          }
          exit(EXIT_FAILURE);
        }
      }
      else
      {
        // Padre
        wait(NULL);
        // Liberar la memoria duplicada, REVISAR
        for (int j = 0; j < i; j++)
        {
          free(args[j]);
        }
        i = 0;

        // exit(EXIT_SUCCESS);
      }
    }
  }
  // args[i] = NULL;
  free(line); // Liberar la memoria asignada por getline

  /*   pid_t pid = fork();

    if (pid < 0)
    {
      perror("fork");
      exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
      // Hijo
      execvp(argv[1], args);
    }
    else
    {
      // Padre
      wait(NULL);
      exit(EXIT_SUCCESS);
    } */

  return 0;
}
