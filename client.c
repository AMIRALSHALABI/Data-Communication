#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 3000
#define BUFFER_SIZE 2048

void url_encode(const char *src, char *dest, size_t dest_size) {
	const char *hex = "0123456789ABCDEF";
	size_t i, j = 0;
	for (i = 0; src[i] != '\0' && j < dest_size - 1; i++) {
		if (isalnum((unsigned char)src[i]) || src[i] == '-' || src[i] == '_' || src[i] == '.' || src[i] == '~') {
			dest[j++] = src[i];
		} else {
			if (j + 3 > dest_size - 1) break;
			dest[j++] = '%';
			dest[j++] = hex[(unsigned char)src[i] >> 4];
			dest[j++] = hex[(unsigned char)src[i] & 15];
		}
	}
	dest[j] = '\0';
}

void send_get_request(SOCKET sock) {
	char send_buffer[BUFFER_SIZE];
	char recv_buffer[BUFFER_SIZE];
	snprintf(send_buffer, sizeof(send_buffer), "GET / HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n", SERVER_IP, SERVER_PORT);

	send(sock, send_buffer, strlen(send_buffer), 0);

	printf("Response:\n");
	int bytes_received;
	while ((bytes_received = recv(sock, recv_buffer, sizeof(recv_buffer) - 1, 0)) > 0) {
		recv_buffer[bytes_received] = '\0';
		printf("%s", recv_buffer);
	}
	printf("\n");
}

void send_put_request(SOCKET sock, const char *name, const char *price) {
	char send_buffer[BUFFER_SIZE];
	char recv_buffer[BUFFER_SIZE];
	char encoded_name[BUFFER_SIZE];
	char encoded_price[BUFFER_SIZE];

	url_encode(name, encoded_name, sizeof(encoded_name));
	url_encode(price, encoded_price, sizeof(encoded_price));

	snprintf(send_buffer, sizeof(send_buffer), "PUT /?newItemName=%s&newItemPrice=%s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n",
			 encoded_name, encoded_price, SERVER_IP, SERVER_PORT);

	send(sock, send_buffer, strlen(send_buffer), 0);

	printf("Response:\n");
	int bytes_received;
	while ((bytes_received = recv(sock, recv_buffer, sizeof(recv_buffer) - 1, 0)) > 0) {
		recv_buffer[bytes_received] = '\0';
		printf("%s", recv_buffer);
	}
	printf("\n");
}

void send_delete_request(SOCKET sock, int index) {
	char send_buffer[BUFFER_SIZE];
	char recv_buffer[BUFFER_SIZE];
	snprintf(send_buffer, sizeof(send_buffer), "DELETE /?index=%d HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n",
			 index, SERVER_IP, SERVER_PORT);

	send(sock, send_buffer, strlen(send_buffer), 0);

	printf("Response:\n");
	int bytes_received;
	while ((bytes_received = recv(sock, recv_buffer, sizeof(recv_buffer) - 1, 0)) > 0) {
		recv_buffer[bytes_received] = '\0';
		printf("%s", recv_buffer);
	}
	printf("\n");
}

int main() {
	WSADATA wsa;
	SOCKET sock;
	struct sockaddr_in server_addr;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
		return 1;
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

	if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
		printf("Connection failed. Error Code: %d\n", WSAGetLastError());
		closesocket(sock);
		WSACleanup();
		return 1;
	}

	int choice;
	while (1) {
		printf("Choose an option:\n1. GET items\n2. PUT new item\n3. DELETE item\n4. Exit\n");
		scanf("%d", &choice);

		getchar(); // Consume newline from the buffer

		if (choice == 1) {
			send_get_request(sock);
		} else if (choice == 2) {
			char name[BUFFER_SIZE], price[BUFFER_SIZE];
			printf("Enter item name: ");
			fgets(name, sizeof(name), stdin);
			name[strcspn(name, "\n")] = '\0'; // Remove newline character

			if (strlen(name) == 0) {
				printf("Item name cannot be empty.\n");
				continue;
			}

			printf("Enter item price: ");
			fgets(price, sizeof(price), stdin);
			price[strcspn(price, "\n")] = '\0'; // Remove newline character

			if (strlen(price) == 0) {
				printf("Item price cannot be empty.\n");
				continue;
			}

			send_put_request(sock, name, price);
		} else if (choice == 3) {
			int index;
			printf("Enter the index of the item to delete: ");
			scanf("%d", &index);

			send_delete_request(sock, index);
		} else if (choice == 4) {
			break;
		} else {
			printf("Invalid choice. Try again.\n");
		}

		// Reconnect for next operation (HTTP connections are stateless)
		closesocket(sock);
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == INVALID_SOCKET || connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
			printf("Reconnection failed. Exiting.\n");
			break;
		}
	}

	closesocket(sock);
	WSACleanup();
	return 0;
}
