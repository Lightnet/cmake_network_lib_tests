#include <enet/enet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>

#define sleep(ms) Sleep(ms)
#define MAX_MESSAGE_SIZE 1024

void run_server() {
    ENetAddress address;
    ENetHost* server;
    ENetEvent event;

    address.host = ENET_HOST_ANY;
    address.port = 12345;

    server = enet_host_create(&address, 32, 2, 0, 0);
    if (server == NULL) {
        fprintf(stderr, "Failed to create server\n");
        return;
    }

    printf("Server started on port %u\n", address.port);
    printf("Enter messages to broadcast (or '/bye' to exit):\n");

    char input[MAX_MESSAGE_SIZE];
    while (1) {
        int service_result = enet_host_service(server, &event, 100);
        if (service_result > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    printf("Client connected from %x:%u\n",
                           event.peer->address.host,
                           event.peer->address.port);
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    printf("Client: %s\n", (char*)event.packet->data);
                    for (size_t i = 0; i < server->peerCount; i++) {
                        if (&server->peers[i] != event.peer) {
                            ENetPacket* packet = enet_packet_create(event.packet->data,
                                                                 event.packet->dataLength,
                                                                 ENET_PACKET_FLAG_RELIABLE);
                            enet_peer_send(&server->peers[i], 0, packet);
                        }
                    }
                    enet_packet_destroy(event.packet);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    printf("Client %x:%u disconnected\n",
                           event.peer->address.host,
                           event.peer->address.port);
                    break;

                default:
                    break;
            }
        }

        if (_kbhit()) {
            if (fgets(input, MAX_MESSAGE_SIZE, stdin)) {
                input[strcspn(input, "\n")] = 0;
                if (strcmp(input, "/bye") == 0) {
                    break;
                }
                if (strlen(input) > 0) {
                    ENetPacket* packet = enet_packet_create(input,
                                                          strlen(input) + 1,
                                                          ENET_PACKET_FLAG_RELIABLE);
                    enet_host_broadcast(server, 0, packet);
                    printf("Server: %s\n", input);
                }
            }
        }
    }

    enet_host_destroy(server);
}

void run_client() {
    ENetHost* client;
    ENetAddress address;
    ENetEvent event;
    ENetPeer* peer;

    client = enet_host_create(NULL, 1, 2, 0, 0);
    if (client == NULL) {
        fprintf(stderr, "Failed to create client host\n");
        return;
    }

    if (enet_address_set_host(&address, "127.0.0.1") < 0) {
        fprintf(stderr, "Failed to set host address\n");
        enet_host_destroy(client);
        return;
    }
    address.port = 12345;

    peer = enet_host_connect(client, &address, 2, 0);
    if (peer == NULL) {
        fprintf(stderr, "No available peers for initiating connection\n");
        enet_host_destroy(client);
        return;
    }

    printf("Connecting to server...\n");
    int connected = 0;
    char input[MAX_MESSAGE_SIZE];

    while (1) {
        int service_result = enet_host_service(client, &event, 100);
        if (service_result > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    printf("Connected to server\n");
                    printf("Enter messages to send (or '/bye' to exit):\n");
                    connected = 1;
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    printf("Server: %s\n", (char*)event.packet->data);
                    enet_packet_destroy(event.packet);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    printf("Disconnected from server\n");
                    enet_host_destroy(client);
                    return;

                default:
                    break;
            }
        }

        if (connected && _kbhit()) {
            if (fgets(input, MAX_MESSAGE_SIZE, stdin)) {
                input[strcspn(input, "\n")] = 0;
                if (strcmp(input, "/bye") == 0) {
                    enet_peer_disconnect(peer, 0);
                    sleep(1000);
                    break;
                }
                if (strlen(input) > 0) {
                    ENetPacket* packet = enet_packet_create(input,
                                                          strlen(input) + 1,
                                                          ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(peer, 0, packet);
                    printf("Me: %s\n", input);
                }
            }
        }
    }

    enet_host_destroy(client);
}

int main(int argc, char* argv[]) {
    if (enet_initialize() != 0) {
        fprintf(stderr, "ENet initialization failed\n");
        return 1;
    }

    printf("ENet initialized successfully!\n");

    if (argc < 2) {
        printf("Usage: %s [server|client]\n", argv[0]);
        enet_deinitialize();
        return 1;
    }

    if (_stricmp(argv[1], "server") == 0) {
        run_server();
    }
    else if (_stricmp(argv[1], "client") == 0) {
        run_client();
    }
    else {
        printf("Invalid argument. Use 'server' or 'client'\n");
        enet_deinitialize();
        return 1;
    }

    enet_deinitialize();
    printf("Program terminated\n");
    return 0;
}