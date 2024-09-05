#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  char *line = NULL; // Puntero inicializado a NULL
  size_t len = 0;    // Tamaño inicial del buffer
  size_t read;       // Número de caracteres leídos

  printf("Introduce una línea (Ctrl+D para terminar):\n");

  while ((read = getline(&line, &len, stdin)) != -1) // devuelve -1 si se llega al final del archivo o si hay un error
  {
    // line contiene la línea leída y read el número de caracteres leídos (sin contar el '\0')
    // ejemplo para "hola mundo": read = 11 (10 caracteres + '\n') y line = "hola mundo\n"
    printf("Leído %zd caracteres: %s", read, line);
  }

  // getline reutiliza el buffer pasado por parámetro, por lo que alcanza con liberar la memoria una vez
  free(line); // Liberar la memoria asignada por getline

  return 0;
}
