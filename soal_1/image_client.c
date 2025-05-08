#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>


#define SERVER_PORT 8080
#define MAX_BUFFER 4096

int establish_server_connection()
{
    int connection_fd;
    struct sockaddr_in server_details;

    if ((connection_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\nFailed to create socket\n");
        return -1;
    }

    server_details.sin_family = AF_INET;
    server_details.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_details.sin_addr) <= 0)
    {
        printf("\nInvalid server address\n");
        return -1;
    }

    if (connect(connection_fd, (struct sockaddr *)&server_details, sizeof(server_details)) < 0)
    {
        printf("\nServer connection failed\n");
        return -1;
    }

    return connection_fd;
}

void fetch_server_file(int connection_fd, const char *file_name)
{
    char download_request[MAX_BUFFER];
    snprintf(download_request, sizeof(download_request), "DOWNLOAD %s", file_name);
    send(connection_fd, download_request, strlen(download_request), 0);

    long file_size;
    read(connection_fd, &file_size, sizeof(file_size));

    char *file_data = malloc(file_size);
    read(connection_fd, file_data, file_size);

    char destination_path[100];
    snprintf(destination_path, sizeof(destination_path), "client/%s", file_name);

    FILE *output_file = fopen(destination_path, "wb");
    if (output_file == NULL)
    {
        printf("Error: Could not create output file\n");
        free(file_data);
        return;
    }

    fwrite(file_data, 1, file_size, output_file);
    fclose(output_file);
    free(file_data);

    printf("Success! File saved as %s\n", file_name);
}

void upload_for_processing(int connection_fd, const char *file_name)
{
    char complete_path[100];
    snprintf(complete_path, sizeof(complete_path), "client/secrets/%s", file_name);

    FILE *input_file = fopen(complete_path, "r");
    if (input_file == NULL)
    {
        printf("Error: Could not locate file\n");
        return;
    }

    char request[MAX_BUFFER];
    snprintf(request, sizeof(request), "DECRYPT %s", complete_path);
    send(connection_fd, request, strlen(request), 0);

    char response[MAX_BUFFER] = {0};
    read(connection_fd, response, MAX_BUFFER);
    printf("%s\n", response);
}

void show_client_options()
{
    printf("\nThe Legend Of Rootkid\n");
    printf("----------------------------\n");
    printf("1. Upload file for decryption\n");
    printf("2. Retrieve file from server\n");
    printf("3. Quit application\n");
    printf(">> ");
}

int run_client_program()
{
    int user_choice;
    int server_connection;
    char input_filename[100];

    while (1)
    {
        show_client_options();
        scanf("%d", &user_choice);
        getchar(); // Clear input buffer

        switch (user_choice)
        {
        case 1:
            printf("Enter filename to process: ");
            fgets(input_filename, sizeof(input_filename), stdin);
            input_filename[strcspn(input_filename, "\n")] = 0;

            server_connection = establish_server_connection();
            if (server_connection < 0)
            {
                printf("Could not connect to server\n");
                break;
            }

            upload_for_processing(server_connection, input_filename);
            close(server_connection);
            break;

        case 2:
            printf("Enter filename to download: ");
            fgets(input_filename, sizeof(input_filename), stdin);
            input_filename[strcspn(input_filename, "\n")] = 0;

            server_connection = establish_server_connection();
            if (server_connection < 0)
            {
                printf("Could not connect to server\n");
                break;
            }

            fetch_server_file(server_connection, input_filename);
            close(server_connection);
            break;

        case 3:
            server_connection = establish_server_connection();
            if (server_connection >= 0)
            {
                send(server_connection, "EXIT", 4, 0);
                close(server_connection);
            }
            printf("Closing client...\n");
            exit(0);

        default:
            printf("Invalid selection\n");
        }
    }

    return 0;
}

int main()
{
    return run_client_program();
}
