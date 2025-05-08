#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>

#define PORT 8080
#define MAX_INVENTORY 10
#define MAX_PLAYERS 10

typedef struct {
    char name[32];
    int damage;
    int price;
    char passive[64];
} Weapon;

typedef struct {
    char name[32];
    int gold;
    char *equipped_weapon;
    int base_damage;
    int kills;
    char *inventory[MAX_INVENTORY];
    int inventory_count;
    int socket_fd;
} Player;

Weapon weapons[] = {
    {"Wooden Stick", 10, 50, "None"},
    {"Flame Dagger", 15, 100, "None"},
    {"Ice Mace", 20, 150, "25% Insta-Kill Chance"},
    {"Venom Sword", 25, 200, "+30% Crit Chance"},
    {"Dragon Slayer", 40, 400, "None"}
};

int weapon_count = sizeof(weapons) / sizeof(weapons[0]);
Player players[MAX_PLAYERS];
int player_count = 0;

void show_weapon_shop(int socket_fd) {
    char shop_list[1024] = "";
    strcat(shop_list, "\n====== Weapon Shop ======\n");
    for (int i = 0; i < weapon_count; i++) {
        char line[256];
        snprintf(line, sizeof(line),
            "%d. %s\n   Damage: %d\n   Price: %d gold\n   Passive: %s\n\n",
            i + 1, weapons[i].name, weapons[i].damage,
            weapons[i].price, weapons[i].passive);
        strcat(shop_list, line);
    }
    send(socket_fd, shop_list, strlen(shop_list), 0);
}

int buy_weapon(int id, int *gold, char **equipped_weapon, int *base_damage, char *response, size_t size) {
    if (id < 1 || id > weapon_count) {
        snprintf(response, size, "Senjata tidak valid.\n");
        return 0;
    }

    Weapon selected = weapons[id - 1];
    if (*gold < selected.price) {
        snprintf(response, size, "Gold tidak cukup untuk membeli %s. Harga: %d, Gold kamu: %d\n",
                selected.name, selected.price, *gold);
        return 0;
    }

    *gold -= selected.price;
    *equipped_weapon = strdup(selected.name);
    *base_damage = selected.damage;

    snprintf(response, size,
        "Berhasil membeli %s!\nDamage meningkat jadi %d.\nSisa gold: %d\n",
        selected.name, *base_damage, *gold);
    return 1;
}

void battle_mode(int socket_fd, int *gold, int base_damage, int *kills, char *equipped_weapon) {
    char buffer[1024];
    const char *current_passive = "None";
    for (int i = 0; i < weapon_count; i++) {
        if (strcmp(equipped_weapon, weapons[i].name) == 0) {
            current_passive = weapons[i].passive;
            break;
        }
    }

    snprintf(buffer, sizeof(buffer), "\n====== BATTLE MODE ======\n");
    send(socket_fd, buffer, strlen(buffer), 0);

    while (1) {
        int enemy_hp = rand() % 151 + 50;
        int max_enemy_hp = enemy_hp;
        int gold_reward = rand() % 41 + 10;

        snprintf(buffer, sizeof(buffer),
            "\n======= BATTLE MODE =======\n"
            "Musuh muncul dengan %d HP!\n", enemy_hp);
        send(socket_fd, buffer, strlen(buffer), 0);

        while (enemy_hp > 0) {
            char health_bar[64] = "";
            int bar_length = 20;
            int filled = (enemy_hp * bar_length) / max_enemy_hp;
            for (int i = 0; i < filled; i++) strcat(health_bar, "█");
            for (int i = filled; i < bar_length; i++) strcat(health_bar, " ");

            snprintf(buffer, sizeof(buffer),
                "\nEnemy HP: [%s] %d/%d\n"
                "1. Attack\n"
                "2. Exit Battle Mode\n"
                "Pilih aksi(1 atau 2): ",
                health_bar, enemy_hp, max_enemy_hp);
            send(socket_fd, buffer, strlen(buffer), 0);

            memset(buffer, 0, sizeof(buffer));
            read(socket_fd, buffer, sizeof(buffer));
            int action = atoi(buffer);

            if (action == 1) {
                int damage = base_damage + (rand() % 11);
                int is_critical = (rand() % 100) < 20;
                int is_insta_kill = 0;

                if (strstr(current_passive, "Insta-Kill")) {
                    is_insta_kill = (rand() % 100) < 25;
                    if (is_insta_kill) {
                        damage = enemy_hp;
                        snprintf(buffer, sizeof(buffer),
                            "\n=== PASSIVE AKTIF ===\n"
                            "Passive '%s' berhasil diaktifkan!\n"
                            "Musuh langsung dikalahkan!\n", current_passive);
                        send(socket_fd, buffer, strlen(buffer), 0);
                    }
                } else if (strstr(current_passive, "Crit Chance")) {
                    is_critical = (rand() % 100) < 30;
                }

                if (!is_insta_kill) {
                    if (is_critical) {
                        damage *= 2;
                        snprintf(buffer, sizeof(buffer), "\nCRITICAL HIT! %d damage!\n", damage);
                    } else {
                        snprintf(buffer, sizeof(buffer), "Anda menyerang! %d damage!\n", damage);
                    }
                    send(socket_fd, buffer, strlen(buffer), 0);
                    enemy_hp -= damage;
                    if (enemy_hp < 0) enemy_hp = 0;
                    usleep(500000);
                }
            } else if (action == 2) {
                snprintf(buffer, sizeof(buffer),
                    "\nAnda keluar dari Battle Mode.\n"
                    "Total musuh yang dikalahkan: %d\n"
                    "Kembali ke menu utama...\n", *kills);
                send(socket_fd, buffer, strlen(buffer), 0);
                return;
            }
        }

        *gold += gold_reward;
        (*kills)++;
        snprintf(buffer, sizeof(buffer),
            "\nMusuh telah dikalahkan!\n"
            "Anda mendapatkan %d gold!\n"
            "Total gold: %d\n"
            "Musuh yang dikalahkan: %d\n"
            "Musuh baru muncul...\n",
            gold_reward, *gold, *kills);
        send(socket_fd, buffer, strlen(buffer), 0);
        usleep(1000000);
    }
}

int create_new_player(const char *name, int socket_fd) {
    if (player_count >= MAX_PLAYERS) return -1;

    Player *p = &players[player_count];
    strncpy(p->name, name, sizeof(p->name) - 1);
    p->gold = 500;
    p->equipped_weapon = "Fists";
    p->base_damage = 5;
    p->kills = 0;
    p->inventory[0] = "Fists";
    p->inventory_count = 1;
    p->socket_fd = socket_fd;

    return player_count++;
}

void *handle_player(void *arg) {
    int socket_fd = *((int *)arg);
    char buffer[1024];
    int player_idx = -1;

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(socket_fd, buffer, sizeof(buffer));
        if (bytes_read <= 0) break;

        if (strncmp(buffer, "LOGIN:", 6) == 0) {
            char name[32];
            sscanf(buffer + 6, "%31[^\n]", name);
            player_idx = create_new_player(name, socket_fd);
            if (player_idx >= 0)
                snprintf(buffer, sizeof(buffer), "Player %s berhasil login sebagai player #%d", name, player_idx + 1);
            else
                snprintf(buffer, sizeof(buffer), "Jumlah player penuh!!!!!");
            send(socket_fd, buffer, strlen(buffer), 0);
            continue;
        }

        if (player_idx == -1) {
            char msg[] = "Silakan login dulu dengan format: LOGIN:<namamu>";
            send(socket_fd, msg, strlen(msg), 0);
            continue;
        }

        Player *p = &players[player_idx];

        if (strcmp(buffer, "SHOW_STATS") == 0) {
		printf("Menampilkan statistik player.\n");                                                   
            printf("====================================\n");
            const char *passive = "None";
            for (int i = 0; i < weapon_count; i++) {
                if (strcmp(p->equipped_weapon, weapons[i].name) == 0) {
                    passive = weapons[i].passive;
                    break;
                }
            }

            snprintf(buffer, sizeof(buffer),
                "\n------ Player Stats ------\n"
                "Gold: %d\n"
                "Equipped Weapon: %s\n"
                "Base Damage: %d\n"
                "Kills: %d\n"
                "Passive: %s\n",
                p->gold, p->equipped_weapon, p->base_damage, p->kills, passive);
            send(socket_fd, buffer, strlen(buffer), 0);
        } else if (strcmp(buffer, "SHOP") == 0) {
		printf("Menampilkan senjata di toko.\n");                                                   
            printf("====================================\n");
            show_weapon_shop(socket_fd);
            char prompt[] = "\nKetik nomor senjata yang ingin dibeli: ";
            send(socket_fd, prompt, strlen(prompt), 0);

            char input[32] = {0};
            read(socket_fd, input, sizeof(input));
            int pilihan = atoi(input);

            char response[256];
            if (buy_weapon(pilihan, &p->gold, &p->equipped_weapon, &p->base_damage, response, sizeof(response))) {
                int already_in_inventory = 0;
                for (int i = 0; i < p->inventory_count; i++) {
                    if (strcmp(p->inventory[i], p->equipped_weapon) == 0) {
                        already_in_inventory = 1;
                        break;
                    }
                }
                if (!already_in_inventory && p->inventory_count < MAX_INVENTORY) {
                    p->inventory[p->inventory_count++] = p->equipped_weapon;
                }
            }
            send(socket_fd, response, strlen(response), 0);
        } else if (strcmp(buffer, "VIEW_INVENTORY") == 0) {
		printf("Menampilkan inventory player.\n");                                                   
            printf("====================================\n");
            char inv[1024] = "\n------ Inventory ------\n";
            for (int i = 0; i < p->inventory_count; i++) {
                const char *passive = "None";
                for (int j = 0; j < weapon_count; j++) {
                    if (strcmp(p->inventory[i], weapons[j].name) == 0) {
                        passive = weapons[j].passive;
                        break;
                    }
                }
                char weapon_info[256];
                snprintf(weapon_info, sizeof(weapon_info), "%d. %s (Passive: %s)\n", i + 1, p->inventory[i], passive);
                strcat(inv, weapon_info);
            }

            strcat(inv, "\nKetik nomor senjata yang ingin dipakai (0 untuk batal): ");
            send(socket_fd, inv, strlen(inv), 0);

            char input[32] = {0};
            read(socket_fd, input, sizeof(input));
            int pilihan = atoi(input);

            if (pilihan > 0 && pilihan <= p->inventory_count) {
                p->equipped_weapon = strdup(p->inventory[pilihan - 1]);
                for (int i = 0; i < weapon_count; i++) {
                    if (strcmp(p->equipped_weapon, weapons[i].name) == 0) {
                        p->base_damage = weapons[i].damage;
                        break;
                    }
                }
                char response[256];
                snprintf(response, sizeof(response), "Senjata %s telah berhasil dipakai!\n", p->equipped_weapon);
                send(socket_fd, response, strlen(response), 0);
            }
        } else if (strcmp(buffer, "BATTLE_MODE") == 0) {
		printf("Memulai battle mode.\n"); 
                printf("====================================\n");
            battle_mode(socket_fd, &p->gold, p->base_damage, &p->kills, p->equipped_weapon);
        } else if (strcmp(buffer, "EXIT") == 0) {
printf("Keluar dari dungeon game.\n");
printf("====================================\n");
            break;
        }
    }

    close(socket_fd);
    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    srand(time(NULL));

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server berjalan di port %d...\n", PORT);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket < 0) {
            perror("accept");
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, handle_player, (void *)&new_socket);
        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}
