#include <limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

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

#define CACHE_EXPIRY 86400

// ascii-art related
char ascii[][70] = {
    "      \x1B[90m___    ",
    "     \x1B[90m(\x1B[0m.. \x1B[90m\\\x1B[0m   ",
    "     \x1B[90m(\x1B\x1B\x1B[33m<> \x1B[90m\x1B[0m\x1B[90m|\x1B[0m   ",
    "    \x1B[90m/\x1B[0m/  \x1B[0m\\ \x1B[90m\\\x1B[0m  ",
    "   \x1B[90m(\x1B[0m \x1B[0m\x1B[90m\x1B[0m|  |\x1B[0m \x1B[90m/|\x1B[0m ",
    "  \x1B[33m_\x1B[0m\x1B[90m/\\\x1B[0m "
    "\x1B[0m__)\x1B[90m/\x1B[0m\x1B[33m_\x1B[0m\x1B[90m)\x1B[0m ",
    "  \x1B[33m\\/\x1B[0m\x1B[90m-____\x1B[0m\x1B[33m\\/  ",
};

int line = 0;

// run a terminal command
const char *exec_command(const char *text)
{
    FILE *cmd;
    char *outputPtr = NULL;

    // Dynamically allocate initial buffer for output
    outputPtr = malloc(1024); // Initial size of 1024 bytes
    if (!outputPtr)
    {
        return NULL; // Handle allocation failure
    }
    outputPtr[0] = '\0';

    cmd = popen(text, "r");
    if (!cmd)
    {
        free(outputPtr); // Clean up in case of error
        return NULL;
    }

    // Read stream and append to outputPtr
    char buffer[128]; // Temporary buffer for reading each line
    while (fgets(buffer, sizeof(buffer), cmd))
    {
        strncat(outputPtr, buffer, strlen(buffer)); // Append without exceeding buffer size
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

// todo: improve this using getenv("TERM")
char *get_shell()
{
    struct passwd *pw = getpwuid(getuid());

    char *shell_path = pw->pw_shell;
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

    return shell_name;
}

const char *get_os()
{
    FILE *file = fopen("/etc/os-release", "r");
    if (!file)
    {
        perror("fopen");
        return "unknown";
    }

    char line[256];
    char *osname = NULL;

    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "PRETTY_NAME=", 5) == 0)
        {
            osname = strdup(line + 13);
            if (osname)
            {
                osname[strlen(osname) - 2] = '\0';
            }
            break;
        }
    }

    fclose(file);

    if (!osname)
    {
        return "unknown";
    }

    return osname;
}

// print_* functions
void print_os()
{
    char *osname = (char *)get_os();
    printInfo("", osname);

    free(osname);
}

void print_mem()
{
    FILE *file = fopen("/proc/meminfo", "r");
    if (!file)
    {
        perror("fopen");
        return;
    }

    char line[256];
    int total_mem = 0;
    int free_mem = 0;
    int buffers = 0;
    int cached = 0;

    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "MemTotal:", 9) == 0)
        {
            total_mem = atoi(line + 9) / 1024; // Convert from kB to MiB
        }
        else if (strncmp(line, "MemFree:", 8) == 0)
        {
            free_mem = atoi(line + 8) / 1024; // Convert from kB to MiB
        }
        else if (strncmp(line, "Buffers:", 8) == 0)
        {
            buffers = atoi(line + 8) / 1024; // Convert from kB to MiB
        }
        else if (strncmp(line, "Cached:", 7) == 0)
        {
            cached = atoi(line + 7) / 1024; // Convert from kB to MiB
        }
        if (total_mem > 0 && free_mem > 0 && buffers > 0 && cached > 0)
        {
            break; // We have all needed values, no need to read further
        }
    }

    fclose(file);

    int used_mem = total_mem - (free_mem + buffers + cached);

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

    int uptime_hours = info.uptime / 3600;
    int uptime_mins = (info.uptime % 3600) / 60;
    int uptime_secs = info.uptime % 60;

    // return result
    char *result;
    asprintf(&result, "%dh %dm %ds", uptime_hours, uptime_mins, uptime_secs);
    printInfo("󰅶", result);

    free(result);
}

// Function to get the user's cache directory
char *get_cache_file_path()
{
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;

    size_t path_len = strlen(homedir) + strlen("/.cache/num_packages_cache.txt") + 1;
    char *cache_file_path = (char *)malloc(path_len);
    snprintf(cache_file_path, path_len, "%s/.cache/num_packages_cache.txt", homedir);

    return cache_file_path;
}

// Function to check if the cache is valid
bool is_cache_valid(const char *cache_file_path)
{
    struct stat st;
    if (stat(cache_file_path, &st) != 0)
    {
        return false; // Cache file doesn't exist
    }

    time_t current_time = time(NULL);
    return (current_time - st.st_mtime) < CACHE_EXPIRY;
}

// Function to read the cache file
char *read_cache(const char *cache_file_path)
{
    FILE *file = fopen(cache_file_path, "r");
    if (!file)
    {
        return NULL;
    }

    char *line = (char *)malloc(64);
    if (fgets(line, 64, file) == NULL)
    {
        free(line);
        fclose(file);
        return NULL;
    }

    fclose(file);

    // Remove newline character
    line[strcspn(line, "\n")] = '\0';
    return line;
}

// Function to write to the cache file
void write_cache(const char *cache_file_path, const char *data)
{
    FILE *file = fopen(cache_file_path, "w");
    if (!file)
    {
        return;
    }

    fprintf(file, "%s\n", data);
    fclose(file);
}

void print_num_packages()
{
    char *cache_file_path = get_cache_file_path();
    char *cached_result = NULL;

    if (is_cache_valid(cache_file_path))
    {
        cached_result = read_cache(cache_file_path);
    }

    if (cached_result)
    {
        printInfo("󰏔", cached_result);
        free(cached_result);
    }
    else
    {
        char *package_command = NULL;
        const char *osname = get_os();

        if (strstr(osname, "Arch") != NULL)
        {
            package_command = "pacman -Q | wc -l";
        }
        else if (strstr(osname, "Fedora") != NULL)
        {
            package_command = "rpm -qa | wc -l";
        }
        else if (strstr(osname, "Debian") != NULL)
        {
            package_command = "dpkg -l | wc -l";
        }
        else if (strstr(osname, "NixOS") != NULL)
        {
            package_command = "nix-store -q --requisites /run/current-system ~/.nix-profile | wc -l";
        }
        else // unsupported os
        {
            printInfo("󰏔", "unknown");
            return;
        }

        char *num_packages_output = (char *)exec_command(package_command);
        num_packages_output[strlen(num_packages_output) - 1] = '\0'; // Remove last newline

        char result[64];
        snprintf(result, sizeof(result), "%s pkgs", num_packages_output);

        free(num_packages_output);

        printInfo("󰏔", result);
        write_cache(cache_file_path, result);
    }

    free(cache_file_path);
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

    printf("\n");
    printf("\n");

    // reset colors
    printColored(NORMAL, "");

    return 0;
}
