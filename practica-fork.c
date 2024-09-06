#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Pruebas de punteros con fork

int main(int argc, char *argv[])
{

  char *args[2] = {"a", "b"};

  pid_t pid = fork();

  if (pid < 0)
  {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  else if (pid == 0)
  {
    // Hijo
    printf("Hijo\n");

    args[0] = "c";
    args[1] = "d";

    printf("args[0]: %s\n", args[0]);
    printf("args[1]: %s\n", args[1]);
  }
  else
  {
    // Padre

    wait(NULL);

    // args[0] = "c";
    // args[1] = "d";

    printf("Padre\n");
    printf("args[0]: %s\n", args[0]);
    printf("args[1]: %s\n", args[1]);
  }

  return 0;
}