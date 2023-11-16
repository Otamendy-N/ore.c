#define _CRT_SECURE_NO_DEPRECATE
#include "stdio.h"
#include "string.h"
#include <stdlib.h>
#include <direct.h>

#ifdef _WIN32
#include "Windows.h"
#include <windows.h>
#define MAX_PATH_LEN MAX_PATH
#define GET_CURRENT_DIR(cwd, len) GetCurrentDirectory((DWORD)len, cwd)
#include <io.h>
#define access _access
#define F_OK 0
#else
#include <unistd.h>
#include <limits.h>
#define MAX_PATH_LEN PATH_MAX
#define GET_CURRENT_DIR getcwd
#endif

#define MAKE_DIRECTORY(path) _mkdir(path)
#define OK 0
#define ERR 1

typedef int Result;
typedef unsigned int bool;

bool file_exists(const char *filename)
{
    return access(filename, F_OK) != -1;
}

char *concat(const char *str1, const char *str2)
{
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);

    char *result = (char *)malloc(len1 + len2 + 1); // +1 para el carácter nulo
    if (result == NULL)
    {
        fprintf(stderr, "[error] No se pudo asignar memoria.\n");
        exit(EXIT_FAILURE);
    }

    strcpy(result, str1);
    strcat(result, str2);

    return result;
}

char *get_current_path(void)
{
    static char cwd[MAX_PATH_LEN];

    if (GET_CURRENT_DIR(cwd, sizeof(cwd)) != 0)
    {
        return cwd;
    }
    else
    {
        perror("[error] Error al obtener el directorio actual\n");
        return NULL;
    }
}

Result create_src_folder(const char *current_path)
{
    if (MAKE_DIRECTORY(concat(current_path, "\\src")) == ERR)
    {
        printf("[error] No se pudo crear la carpeta ./src.");
        return ERR;
    }
    FILE *main_file = fopen(concat(current_path, "\\src\\main.c"), "w");
    if (main_file == NULL)
    {
        printf("[error] No se pudo crear el archivo main.c\n");
        return ERR;
    }
    const char *content = "#include \"stdio.h\"\n\nint main(void) {\n    printf(\"Hello world!\\n\");\n    return 0;\n}\n";
    fputs(content, main_file);

    fclose(main_file);
    return OK;
}

Result create_bin_folder(const char *current_path)
{
    return MAKE_DIRECTORY(concat(current_path, "\\bin"));
}

Result build_single_file(const char *current_path, const char *file_name)
{
    create_bin_folder(current_path);
    char* parsed_name = (char *) file_name;
    if (file_name[0] == '.')
    {
        parsed_name = parsed_name + 1;
    }
    if (parsed_name[0] == '/' || parsed_name[0] == '\\') 
    {
        parsed_name = parsed_name + 1;
    }
    const char* full_path = concat(current_path, concat("\\", parsed_name));
    if (file_exists(full_path))
    {
        Result res = system(concat("clang -Wall -Wextra -o .\\bin\\bin.exe ", file_name));
        if (res == OK)
            printf("[info] Archivo 'main.c' compilado exitosamente.\n");
        return res;
    }

    printf("[error] Source file, '%s' at '%s', does not exist.", parsed_name, full_path);
    return ERR;
}

Result build_and_run_single_file(const char *current_path, const char *file_name)
{
    Result result = build_single_file(current_path, file_name);
    if (result == ERR)
        return ERR;

    Result run_result = system(".\\bin\\bin.exe\"");
    if (run_result == OK)
        printf("[info] Programa ejecutado exitosamente.\n");
    else
        printf("[error] El programa no se pudo ejecutar.\n");
    return run_result;
}

Result build(const char *current_path)
{
    create_bin_folder(current_path);
    const char * full_path = concat(current_path, "\\src\\main.c");
    if (file_exists(full_path))
        return system("clang -Wall -Wextra -o .\\bin\\bin.exe .\\src\\main.c");

    printf("[error] Main file at '%s', does not exist.", full_path);
    return ERR;
}

Result build_and_run(const char *current_path)
{
    Result build_res = build(current_path);
    if (build_res == ERR)
        return ERR;
    printf("[info] Archivo 'main.c' compilado exitosamente.\n");

    Result run_result = system(".\\bin\\bin.exe\"");
    if (run_result == OK)
        printf("[info] Programa ejecutado exitosamente.\n");
    return run_result;
}

Result jump_to_new_dir(const char *path)
{
    Result run_result = system(concat("cd ", path));
    return run_result;
}

void print_menu(void)
{
    printf("List of available commands:\n");
    printf(" - new [name] -> this will create a folder\n");
    printf(" - build [name]? -> this will create a folder\n");
    printf(" - run [name]? -> this will build and run the provided c file and if a file is not provided this will try to run a main.c file.\n");
}

int main(int args_length, char **args)
{
    printf("\n");
    if (args_length == 0 || args_length == 1)
    {
        print_menu();
        return OK;
    }

    const char *current_path = get_current_path();
    const char *command = args[1];
    const char *parameter = args[2];

    if (!strcmp(command, "run\0"))
    {
        if (args_length == 3)
            build_and_run_single_file(current_path, parameter);
        else
            build_and_run(current_path);
        return OK;
    }

    if (!strcmp(command, "build\0"))
    {
        if (args_length == 3)
            build_single_file(current_path, parameter);
        else
            build(current_path);
        return OK;
    }

    if (!strcmp(command, "new\0") && args_length == 3)
    {
        const char *root_path = concat(concat(current_path, "\\"), parameter);
        if (MAKE_DIRECTORY(root_path) == ERR)
        {
            printf("[error] Could not create folder '%s' in path: %s\n", root_path, current_path);
            return ERR;
        }

        printf("[info] Project folder created at: %s\n", root_path);

        if (create_src_folder(root_path) == ERR || create_bin_folder(root_path) == ERR)
            return ERR;

        return OK;
    }
    else
    {
        print_menu();
        return ERR;
    }
    return OK;
}
