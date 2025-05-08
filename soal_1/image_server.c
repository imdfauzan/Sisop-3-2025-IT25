#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <arpa/inet.h>

#define SERVER_PORT 8080
#define DATA_BUFFER_SIZE 4096
#define LOG_PATH "server/server.log"

void reverse_string(const char *input, char *result)
{
    int length = strlen(input);
    for (int i = 0; i < length; i++)
    {
        result[i] = input[length - 1 - i];
    }
    result[length] = '\0';
}

void store_jpeg_file(const char *content, const char *filepath)
{
    FILE *output_file = fopen(filepath, "wb");
    if (output_file == NULL)
    {
        perror("Failed creating output file");
        return;
    }

    fwrite(content, 1, strlen(content), output_file);
    fclose(output_file);
}

void send_error_response(int socket_fd, const char *message)
{
    send(socket_fd, message, strlen(message), 0);
}

void record_event(const char *origin, const char *operation, const char *details)
{
    time_t current;
    time(&current);
    struct tm *time_data = localtime(&current);

    char time_str[20];
    strftime(time_str, 20, "%Y-%m-%d %H:%M:%S", time_data);

    FILE *log_file = fopen(LOG_PATH, "a");
    if (log_file == NULL)
    {
        perror("Unable to access log file");
        return;
    }

    fprintf(log_file, "[%s][%s]: [%s] [%s]\n", origin, time_str, operation, details);
    fclose(log_file);
}

void process_client_request(int connection_fd)
{
    char data_buffer[DATA_BUFFER_SIZE] = {0};
    char ip_address[INET_ADDRSTRLEN];
    struct sockaddr_in client_info;
    socklen_t info_size = sizeof(client_info);

    getpeername(connection_fd, (struct sockaddr *)&client_info, &info_size);
    inet_ntop(AF_INET, &client_info.sin_addr, ip_address, INET_ADDRSTRLEN);

    int bytes_read = read(connection_fd, data_buffer, DATA_BUFFER_SIZE);
    if (bytes_read <= 0)
    {
        perror("Reading data failed");
        close(connection_fd);
        return;
    }

    if (strncmp(data_buffer, "DECRYPT", 7) == 0)
    {
        char *input_file = data_buffer + 8;
        FILE *input_fp = fopen(input_file, "r");
        if (input_fp == NULL)
        {
            send_error_response(connection_fd, "Error: File not found");
            record_event("Server", "ERROR", "File not found");
            close(connection_fd);
            return;
        }

        fseek(input_fp, 0, SEEK_END);
        long file_size = ftell(input_fp);
        fseek(input_fp, 0, SEEK_SET);

        char *text_content = malloc(file_size + 1);
        fread(text_content, 1, file_size, input_fp);
        fclose(input_fp);

        char reversed_text[file_size * 2];
        reverse_string(text_content, reversed_text);
        free(text_content);

        time_t current_time = time(NULL);
        char output_filename[50];
        snprintf(output_filename, sizeof(output_filename), "server/database/%ld.jpeg", current_time);

        store_jpeg_file(reversed_text, output_filename);

        char reply[100];
        snprintf(reply, sizeof(reply), "Server: Text decrypted and saved as %ld.jpeg", current_time);
        send(connection_fd, reply, strlen(reply), 0);

        record_event("Client", "DECRYPT", "Text data");
        record_event("Server", "SAVE", output_filename);
    }
    else if (strncmp(data_buffer, "DOWNLOAD", 8) == 0)
    {
        char *requested_file = data_buffer + 9;
        char complete_path[100];
        snprintf(complete_path, sizeof(complete_path), "server/database/%s", requested_file);

        FILE *download_fp = fopen(complete_path, "rb");
        if (download_fp == NULL)
        {
            send_error_response(connection_fd, "Error: File not found for download");
            record_event("Server", "ERROR", "File not found for download");
            close(connection_fd);
            return;
        }

        fseek(download_fp, 0, SEEK_END);
        long download_size = ftell(download_fp);
        fseek(download_fp, 0, SEEK_SET);

        char *file_contents = malloc(download_size);
        fread(file_contents, 1, download_size, download_fp);
        fclose(download_fp);

        send(connection_fd, &download_size, sizeof(download_size), 0);
        send(connection_fd, file_contents, download_size, 0);
        free(file_contents);

        record_event("Client", "DOWNLOAD", requested_file);
        record_event("Server", "UPLOAD", requested_file);
    }
    else if (strncmp(data_buffer, "EXIT", 4) == 0)
    {
        record_event("Client", "EXIT", "Client requested to exit");
    }

    close(connection_fd);
}

int initialize_server()
{
    mkdir("server", 0755);
    mkdir("server/database", 0755);

    pid_t process_id = fork();
    if (process_id < 0)
    {
        exit(EXIT_FAILURE);
    }
    if (process_id > 0)
    {
        exit(EXIT_SUCCESS);
    }

    umask(0);

    int server_socket, client_connection;
    struct sockaddr_in server_address;
    int option = 1;
    int address_length = sizeof(server_address);

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)))
    {
        perror("Socket options failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 3) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    record_event("Server", "START", "Server initialized");

    while (1)
    {
        if ((client_connection = accept(server_socket, (struct sockaddr *)&server_address, (socklen_t *)&address_length)) < 0)
        {
            perror("Connection acceptance failed");
            continue;
        }

        process_client_request(client_connection);
    }

    return 0;
}

int main()
{
    return initialize_server();
}
