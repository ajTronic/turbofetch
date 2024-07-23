#include <limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>

// colour constants
#define NORMAL "\x1B[0m"
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"
#define WHITE "\x1B[37m"
#define NAVY "\x1B[2m"

// custom constants
#define SEPARATOR "  "
#define COLOR_CHAR " "
#define COLOR_KEY MAGENTA
#define COLOR_MAIN WHITE

// ascii-art related
char ascii[][70] = {
    "      \x1B[2m___\x1B[0m    ",
    "     \x1B[2m(\x1B[0m.. \x1B[2m\\\x1B[0m   ",
    "     \x1B[2m(\x1B\x1B[0m\x1B[33m<> \x1B[2m\x1B[0m\x1B[2m|\x1B[0m   ",
    "    \x1B[2m/\x1B[0m/  \x1B[2m\\ \x1B[2m\\\x1B[0m  ",
    "   \x1B[2m(\x1B[0m \x1B[0m\x1B[2m\x1B[0m|  |\x1B[0m \x1B[2m/|\x1B[0m ",
    "  \x1B[33m_\x1B[0m\x1B[2m/\\\x1B[0m "
    "\x1B[0m__)\x1B[2m/\x1B[0m\x1B[33m_\x1B[0m\x1B[2m)\x1B[0m ",
    "  \x1B[33m\\/\x1B[0m\x1B[2m-____\x1B[0m\x1B[33m\\/  ",
};

int line = 0;

// run a terminal command
const char *exec_command(char text[]) {
  FILE *cmd;
  char result[sizeof(char) * 128];

  // create output pointer and initialize it
  char *outputPtr = malloc(sizeof(char) * 1024);
  outputPtr[0] = '\0';

  cmd = popen(text, "r");
  while (fgets(result, sizeof(result), cmd)) { // read stream
    strcat(outputPtr, result);
  }

  pclose(cmd);

  return outputPtr;
}

// print coloured text to the terminal
void printColored(char text[], char format[]) { printf("%s%s", format, text); }

// print a newline and ascii art
void newline() {
  printf("\n");
  printColored(ascii[line], COLOR_MAIN);
  printf(SEPARATOR);
  line++;
}

// print a key and a value to the info
void printInfo(char key[], char value[]) {
  printColored(key, COLOR_KEY);
  printf(SEPARATOR);
  printColored(value, COLOR_MAIN);
  newline();
}

// get_* functions
char *get_mem() {
  char *meminfo = (char *)exec_command("grep -m 2 -Eo '[0-9]{1,16}' /proc/meminfo");
  int total_mem = atoi(strtok(meminfo, "\n")) / 1024.0;
  int free_mem = atoi(strtok(NULL, "\n")) / 1024.0;
  int used_mem = total_mem - free_mem;

  free(meminfo);

  char *result;
  asprintf(&result, "%d/%d MiB", used_mem, total_mem);

  return result;
}

char *get_uptime() {
  // get system info
  struct sysinfo info;
  sysinfo(&info);

  int uptime_hours = info.uptime / 60 / 60;
  int uptime_mins = info.uptime % 60;
  int uptime_secs = info.uptime % 60 % 60;

  // return result
  char *result;
  asprintf(&result, "%dh %dm %ds", uptime_hours, uptime_mins, uptime_secs);

  return result;
}

char* get_num_packages() {
  char* num_packages_output = (char*)exec_command("pacman -Q | wc -l");
  num_packages_output[strlen(num_packages_output)-1] = *"\0"; // remove last newline

  char *result;
  asprintf(&result, "%s pkgs", num_packages_output);

  free(num_packages_output);

  return result;
}

char* get_shell() { // todo get correct shell
  char* shell_path= (char*)exec_command("echo \"$SHELL\"");

  if (strstr(shell_path, "fish")) return "fish";
  if (strstr(shell_path, "zsh")) return "zsh";
  if (strstr(shell_path, "bash")) return "bash";
  if (strstr(shell_path, "ksh")) return "ksh";
  if (strstr(shell_path, "tcsh")) return "tcsh";
  if (strstr(shell_path, "dash")) return "dash";
  if (strstr(shell_path, "sh")) return "bash";

  free(shell_path);

  return "unknown";
}

int main() {
  newline();

  printColored(getpwuid(getuid())->pw_name, YELLOW); // username
  printColored("@", RED);

  // computer name
  char hostname[HOST_NAME_MAX];
  gethostname(hostname, HOST_NAME_MAX);
  printColored(hostname, BLUE);

  newline();

  // print info
  printInfo("󰣇", "Arch Linux");
  printInfo("", get_shell());
  printInfo("󰍛", get_mem());
  printInfo("󰏔", get_num_packages());
  printInfo("󰅶", get_uptime());

  // theme
  printColored("", MAGENTA);
  printf(SEPARATOR);

  // colors to print
  char colors[][6] = {RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, NAVY};

  // print each color
  for (int i = 0; i < sizeof(*colors); i++) {
    printColored(COLOR_CHAR, colors[i]);
  }

  // final newline that doesn't require ascii-art
  printf("\n");

  // add an extra newline for non-fish shells
  if (strcmp("fish", get_shell()) != 0) printf("\n");

  return 0;
}
