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
const char *exec_command(char text[])
{
  FILE *cmd;
  char result[sizeof(char) * 128];

  // create output pointer and initialize it
  char *outputPtr = malloc(sizeof(char) * 1024);
  outputPtr[0] = '\0';

  cmd = popen(text, "r");
  while (fgets(result, sizeof(result), cmd))
  { // read stream
    strcat(outputPtr, result);
  }

  pclose(cmd);

  return outputPtr;
}

// print coloured text to the terminal
void printColored(char text[], char format[]) { printf("%s%s", format, text); }

// print a newline and ascii art
void newline()
{
  printf("\n");
  printColored(ascii[line], COLOR_MAIN);
  printf(SEPARATOR);
  line++;
}

// print a key and a value to the info
void printInfo(char key[], char value[])
{
  printColored(key, COLOR_KEY);
  printf(SEPARATOR);
  printColored(value, COLOR_MAIN);
  newline();
}

// TODO: get correct shell
char *get_shell()
{
  char *shell_path = (char *)exec_command("echo \"$SHELL\"");
  char *shell_name;

  if (strstr(shell_path, "fish"))
    shell_name = "fish";
  else if (strstr(shell_path, "zsh"))
    shell_name = "zsh";
  else if (strstr(shell_path, "bash"))
    shell_name = "bash";
  else if (strstr(shell_path, "ksh"))
    shell_name = "ksh";
  else if (strstr(shell_path, "tcsh"))
    shell_name = "tcsh";
  else if (strstr(shell_path, "dash"))
    shell_name = "dash";
  else if (strstr(shell_path, "sh"))
    shell_name = "bash";
  else
    shell_name = "unknown";

  free(shell_path);

  return shell_name;
}

const char* get_os() {
  char *osname = (char *)exec_command("grep -Po 'NAME=\"\\K.*?(?=\")' /etc/os-release | head -1");
  osname[strlen(osname) - 1] = 0;
  
  return osname;
}

// print_* functions
void print_os()
{ 
  char* osname = (char*)get_os();
  printInfo("󰣇", osname);

  free(osname);
}

void print_mem()
{
  char *meminfo = (char *)exec_command("grep -m 2 -Eo '[0-9]{1,16}' /proc/meminfo");
  int total_mem = atoi(strtok(meminfo, "\n")) / 1024.0;
  int free_mem = atoi(strtok(NULL, "\n")) / 1024.0;
  int used_mem = total_mem - free_mem;

  free(meminfo);

  char *result;
  asprintf(&result, "%d/%d MiB", used_mem, total_mem);
  printInfo("󰍛", result);

  free(result);
}

void print_uptime()
{
  // get system info
  struct sysinfo info;
  sysinfo(&info);

  int uptime_hours = info.uptime / 60 / 60;
  int uptime_mins = info.uptime % 60;
  int uptime_secs = info.uptime % 60 % 60;

  // return result
  char *result;
  asprintf(&result, "%dh %dm %ds", uptime_hours, uptime_mins, uptime_secs);
  printInfo("󰅶", result);

  free(result);
}

void print_num_packages()
{
  char *package_command;
  const char *osname = get_os();

  // printf("%s", get_package_manager());

  if (strstr(osname, "Arch") != NULL)
    package_command = "pacman -Q | wc -l";
  else if (strstr(osname, "Fedora") != NULL)
    package_command = "rpm -qa | wc -l";
  else
    package_command = "dpkg -l | wc -l";

  char *num_packages_output = (char *)exec_command(package_command);
  num_packages_output[strlen(num_packages_output) - 1] = *"\0"; // remove last newline

  char *result;
  asprintf(&result, "%s pkgs", num_packages_output);

  free(num_packages_output);
  free((char*)osname);

  printInfo("󰏔", result);

  free(result);
}

void print_shell()
{
  char *shell_name = get_shell();
  printInfo("", shell_name);
}

void print_colors()
{
  printColored("", MAGENTA);
  printf(SEPARATOR);

  char colors[][6] = {RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, NAVY};

  for (int i = 0; i < sizeof(*colors); i++)
  {
    printColored(COLOR_CHAR, colors[i]);
  }
}

int main()
{
  newline();

  printColored(getpwuid(getuid())->pw_name, YELLOW); // username
  printColored("@", RED);

  // computer name
  char hostname[HOST_NAME_MAX];
  gethostname(hostname, HOST_NAME_MAX);
  printColored(hostname, BLUE);

  newline();

  // print info
  print_os();
  print_shell();
  print_mem();
  print_num_packages();
  print_uptime();
  print_colors();

  // final newline that doesn't require ascii-art
  printf("\n");

  // reset colors
  printColored(NORMAL, "");

  return 0;
}
