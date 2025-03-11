#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOMCX
#define NOSERVICE
#define NOUSER

#include <winsock2.h>
#include <windows.h>
#include "raylib.h"
#define ENET_IMPLEMENTATION
#include "enet.h"

#include <stdio.h>
#include <string.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MAX_CLIENTS 32
#define HASH_LENGTH 8

typedef struct {
    Vector2 position;
    bool connected;
    char playerHash[HASH_LENGTH + 1];
} Player;

typedef struct {
    char id[HASH_LENGTH + 1];
    float x, y;
    bool active;
} OtherPlayer;

int main() {
    if (enet_initialize() != 0) {
        fprintf(stderr, "ENet initialization failed.\n");
        return EXIT_FAILURE;
    }
    printf("ENet initialized.\n");

    ENetHost* client = enet_host_create(NULL, 1, 2, 0, 0);
    if (client == NULL) {
        fprintf(stderr, "Failed to create ENet client host.\n");
        enet_deinitialize();
        return EXIT_FAILURE;
    }
    printf("Client host created.\n");

    ENetAddress address = {0};
    ENetEvent event;
    ENetPeer* peer;
    enet_address_set_host(&address, "127.0.0.1");
    address.port = 7777;
    printf("Attempting to connect to 127.0.0.1:7777...\n");
    peer = enet_host_connect(client, &address, 2, 0);
    if (peer == NULL) {
        fprintf(stderr, "No available peers for connection.\n");
        enet_host_destroy(client);
        enet_deinitialize();
        return EXIT_FAILURE;
    }

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Network Test Client - Raylib 5.5");
    SetTargetFPS(60);

    Player player = {{SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2}, false, ""};
    OtherPlayer other_players[MAX_CLIENTS] = {0};
    float speed = 200.0f;
    char status[64] = "Connecting...";
    int connected_count = 0;
    int disconnected_count = 0;

    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        printf("Connection to 127.0.0.1:%d succeeded.\n", address.port);
        player.connected = true;
    } else {
        enet_peer_reset(peer);
        printf("Connection to 127.0.0.1:%d failed.\n", address.port);
        enet_host_destroy(client);
        enet_deinitialize();
        return EXIT_FAILURE;
    }

    while (!WindowShouldClose()) {
        while (enet_host_service(client, &event, 0) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_RECEIVE: {
                    char* data = (char*)event.packet->data;
                    if (strncmp(data, "POSITIONS:", 10) == 0) {
                        int count = 0;
                        char* token = strtok(data + 10, ";");
                        while (token) {
                            char id[HASH_LENGTH + 1];
                            float x, y;
                            if (sscanf(token, "%8[^:]:%f,%f", id, &x, &y) == 3) {
                                if (strcmp(id, player.playerHash) != 0) {
                                    int slot = -1;
                                    for (int i = 0; i < MAX_CLIENTS; i++) {
                                        if (other_players[i].active && strcmp(other_players[i].id, id) == 0) {
                                            slot = i;
                                            break;
                                        }
                                        if (!other_players[i].active && slot == -1) slot = i;
                                    }
                                    if (slot != -1) {
                                        strcpy(other_players[slot].id, id);
                                        other_players[slot].x = x;
                                        other_players[slot].y = y;
                                        other_players[slot].active = true;
                                        printf("Updated other player %s to (%.1f, %.1f)\n", id, x, y);
                                    }
                                } else {
                                    printf("Received own position %s: (%.1f, %.1f), ignored\n", id, x, y);
                                }
                                count++;
                            }
                            token = strtok(NULL, ";");
                        }
                        connected_count = count;
                        printf("Received positions (count: %d): %s\n", count, data);
                    } else if (strncmp(data, "JOIN:", 5) == 0) {
                        char id[HASH_LENGTH + 1];
                        sscanf(data, "JOIN:%8s", id);
                        if (strlen(player.playerHash) == 0) {
                            strcpy(player.playerHash, id);
                            connected_count = 1;
                            printf("Assigned playerHash: %s, Connected: %d\n", player.playerHash, connected_count);
                        } else if (strcmp(id, player.playerHash) != 0) {
                            int slot = -1;
                            for (int i = 0; i < MAX_CLIENTS; i++) {
                                if (!other_players[i].active) {
                                    slot = i;
                                    break;
                                }
                            }
                            if (slot != -1) {
                                strcpy(other_players[slot].id, id);
                                other_players[slot].x = SCREEN_WIDTH / 2;
                                other_players[slot].y = SCREEN_HEIGHT / 2;
                                other_players[slot].active = true;
                                connected_count++;
                                printf("Player joined: %s, Connected: %d\n", id, connected_count);
                            }
                        }
                    } else if (strncmp(data, "LEAVE:", 6) == 0) {
                        char id[HASH_LENGTH + 1];
                        sscanf(data, "LEAVE:%8s", id);
                        if (strcmp(id, player.playerHash) != 0) {
                            for (int i = 0; i < MAX_CLIENTS; i++) {
                                if (other_players[i].active && strcmp(other_players[i].id, id) == 0) {
                                    other_players[i].active = false;
                                    connected_count--;
                                    disconnected_count++;
                                    printf("Player left: %s, Connected: %d, Disconnected: %d\n", id, connected_count, disconnected_count);
                                    break;
                                }
                            }
                        }
                    } else {
                        printf("Received: %s\n", data);
                    }
                    enet_packet_destroy(event.packet);
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT:
                case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                    player.connected = false;
                    strcpy(status, "Disconnected");
                    printf("Disconnected from server.\n");
                    disconnected_count++;
                    printf("Connected: %d, Disconnected: %d\n", connected_count, disconnected_count);
                    break;
            }
        }

        if (player.connected && strlen(player.playerHash) > 0) {
            float dt = GetFrameTime();
            if (IsKeyDown(KEY_RIGHT)) player.position.x += speed * dt;
            if (IsKeyDown(KEY_LEFT)) player.position.x -= speed * dt;
            if (IsKeyDown(KEY_UP)) player.position.y -= speed * dt;
            if (IsKeyDown(KEY_DOWN)) player.position.y += speed * dt;

            char buffer[32];
            snprintf(buffer, sizeof(buffer), "POS:%s:%.1f,%.1f", player.playerHash, player.position.x, player.position.y);
            ENetPacket* packet = enet_packet_create(buffer, strlen(buffer) + 1, ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(peer, 0, packet);
            printf("Sent position for %s: (%.1f, %.1f)\n", player.playerHash, player.position.x, player.position.y);
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        if (player.connected) {
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (other_players[i].active && strcmp(other_players[i].id, player.playerHash) != 0) {
                    DrawCircleV((Vector2){other_players[i].x, other_players[i].y}, 20, BLUE);
                }
            }
            DrawCircleV(player.position, 20, RED);
        }
        DrawText(status, 10, 10, 20, player.connected ? GREEN : RED);
        char count_text[64];
        snprintf(count_text, sizeof(count_text), "Connected: %d, Disconnected: %d", connected_count, disconnected_count);
        DrawText(count_text, 10, 40, 20, BLACK);

        int y_offset = 70;
        if (player.connected && strlen(player.playerHash) > 0) {
            char self_text[64];
            snprintf(self_text, sizeof(self_text), "playerHash: %s, x:%.1f, y:%.1f", player.playerHash, player.position.x, player.position.y);
            DrawText(self_text, 10, y_offset, 20, RED);
            y_offset += 30;
        }
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (other_players[i].active) {
                char player_text[64];
                snprintf(player_text, sizeof(player_text), "playerHash: %s, x:%.1f, y:%.1f", other_players[i].id, other_players[i].x, other_players[i].y);
                DrawText(player_text, 10, y_offset, 20, BLUE);
                y_offset += 30;
            }
        }
        EndDrawing();
    }

    enet_peer_disconnect(peer, 0);
    uint8_t disconnected = false;
    while (enet_host_service(client, &event, 3000) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                puts("Disconnection succeeded.");
                disconnected = true;
                break;
        }
    }
    if (!disconnected) enet_peer_reset(peer);

    CloseWindow();
    enet_host_destroy(client);
    enet_deinitialize();
    printf("Client shut down.\n");
    return 0;
}