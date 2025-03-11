#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define ENET_IMPLEMENTATION
#include <enet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_CLIENTS 32
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define HASH_LENGTH 8

typedef struct {
    ENetPeer* peer;
    float x, y;
    int active;
    char playerHash[HASH_LENGTH + 1]; // +1 for null terminator
} Client;

void generate_hash(char* out) {
    const char* chars = "0123456789abcdef";
    for (int i = 0; i < HASH_LENGTH; i++) {
        out[i] = chars[rand() % 16];
    }
    out[HASH_LENGTH] = '\0';
}

void print_player_list(Client* clients, int connected_count, int disconnected_count) {
    printf("\n--- Player List ---\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            printf("playerHash: %s, x:%.1f, y:%.1f\n", clients[i].playerHash, clients[i].x, clients[i].y);
        }
    }
    printf("Connected: %d, Disconnected: %d\n", connected_count, disconnected_count);
    printf("-------------------\n\n");
}

void broadcast_positions(ENetHost* server, Client* clients) {
    char buffer[1024]; // Smaller buffer since IDs are shorter
    int offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "POSITIONS:");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%s:%.1f,%.1f;", clients[i].playerHash, clients[i].x, clients[i].y);
        }
    }
    if (offset > 10) {
        ENetPacket* packet = enet_packet_create(buffer, strlen(buffer) + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_host_broadcast(server, 0, packet);
        printf("Broadcasted positions: %s\n", buffer);
    }
}

int main() {
    srand((unsigned)time(NULL));
    if (enet_initialize() != 0) {
        printf("An error occurred while initializing ENet.\n");
        return 1;
    }
    printf("ENet initialized.\n");

    ENetAddress address = {0};
    address.host = ENET_HOST_ANY;
    address.port = 7777;

    ENetHost* server = enet_host_create(&address, MAX_CLIENTS, 2, 0, 0);
    if (server == NULL) {
        printf("Failed to create ENet server host on port 7777.\n");
        enet_deinitialize();
        return 1;
    }
    printf("Server started on port 7777.\n");

    Client clients[MAX_CLIENTS] = {0};
    int connected_count = 0;
    int disconnected_count = 0;

    while (true) {
        ENetEvent event;
        while (enet_host_service(server, &event, 1000) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    printf("connect client...\n");
                    int slot = -1;
                    for (int i = 0; i < MAX_CLIENTS; i++) {
                        if (!clients[i].active) {
                            slot = i;
                            break;
                        }
                    }
                    if (slot != -1) {
                        clients[slot].peer = event.peer;
                        clients[slot].active = 1;
                        clients[slot].x = SCREEN_WIDTH / 2;
                        clients[slot].y = SCREEN_HEIGHT / 2;
                        generate_hash(clients[slot].playerHash);
                        event.peer->data = (void*)slot;
                        char host_ip[46];
                        enet_address_get_host_ip(&event.peer->address, host_ip, sizeof(host_ip));
                        printf("Client %d connected from %s:%u, playerHash: %s\n", slot, host_ip, event.peer->address.port, clients[slot].playerHash);
                        connected_count++;
                        print_player_list(clients, connected_count, disconnected_count);

                        char join_msg[32];
                        snprintf(join_msg, sizeof(join_msg), "JOIN:%s", clients[slot].playerHash);
                        ENetPacket* join_packet = enet_packet_create(join_msg, strlen(join_msg) + 1, ENET_PACKET_FLAG_RELIABLE);
                        enet_host_broadcast(server, 0, join_packet);
                        printf("Broadcasted: %s\n", join_msg);

                        broadcast_positions(server, clients);
                    } else {
                        printf("Connection rejected: max clients reached.\n");
                        enet_peer_disconnect(event.peer, 0);
                    }
                    break;
                }

                case ENET_EVENT_TYPE_RECEIVE: {
                    int slot = (intptr_t)event.peer->data;
                    if (strncmp((char*)event.packet->data, "POS:", 4) == 0) {
                        char received_hash[HASH_LENGTH + 1];
                        float x, y;
                        if (sscanf((char*)event.packet->data, "POS:%8[^:]:%f,%f", received_hash, &x, &y) == 3) {
                            if (strcmp(received_hash, clients[slot].playerHash) == 0) {
                                clients[slot].x = x;
                                clients[slot].y = y;
                                printf("Client %d (%s) position updated to: (%.1f, %.1f)\n", slot, clients[slot].playerHash, x, y);
                                print_player_list(clients, connected_count, disconnected_count);
                                broadcast_positions(server, clients);
                            } else {
                                printf("Hash mismatch for slot %d: expected %s, received %s\n", slot, clients[slot].playerHash, received_hash);
                            }
                        }
                    }
                    enet_packet_destroy(event.packet);
                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT:
                case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
                    int slot = (intptr_t)event.peer->data;
                    printf("Client %d (%s) disconnected.\n", slot, clients[slot].playerHash);
                    clients[slot].active = 0;
                    disconnected_count++;
                    print_player_list(clients, connected_count, disconnected_count);
                    
                    char leave_msg[32];
                    snprintf(leave_msg, sizeof(leave_msg), "LEAVE:%s", clients[slot].playerHash);
                    ENetPacket* leave_packet = enet_packet_create(leave_msg, strlen(leave_msg) + 1, ENET_PACKET_FLAG_RELIABLE);
                    enet_host_broadcast(server, 0, leave_packet);
                    printf("Broadcasted: %s\n", leave_msg);

                    broadcast_positions(server, clients);
                    break;
                }
            }
        }
    }

    enet_host_destroy(server);
    enet_deinitialize();
    printf("Server shut down.\n");
    return 0;
}