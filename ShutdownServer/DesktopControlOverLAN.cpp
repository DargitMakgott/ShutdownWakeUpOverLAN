#include <future>
#include <iostream>
#include <thread>
#include <chrono>

#include "DesktopControlOverLAN.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;


int main(int argc, char* const argv[])
{
	int port = 5555;

	if (argc == 2)	port = atoi(argv[1]);
	

	WSADATA wsa_data;
	SOCKADDR_IN server_addr, client_addr;

	WSAStartup(MAKEWORD(2, 2), &wsa_data);
	const auto server = socket(AF_INET, SOCK_STREAM, 0);

	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	::bind(server, reinterpret_cast<SOCKADDR*>(&server_addr), sizeof(server_addr));
	listen(server, 0);

	cout << "Listening for incoming connections..." << endl;

	int client_addr_size = sizeof(client_addr);

	for (;;)
	{
		SOCKET client;

		if ((client = accept(server, reinterpret_cast<SOCKADDR*>(&client_addr), &client_addr_size)) != INVALID_SOCKET)
		{
			auto fut = async(launch::async, on_client_connect, client);
		}

		const auto last_error = WSAGetLastError();

		if (last_error > 0)
		{
			cout << "Error: " << last_error << endl;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void on_client_connect(SOCKET client)
{
	char buffer[1];

	cout << "Client connected!" << endl;
	recv(client, buffer, sizeof(buffer), 0);

	if (buffer[0] == '0') 
		system("shutdown -h");
	

	memset(buffer, 0, sizeof(buffer));

	closesocket(client);
	cout << "Client disconnected." << endl;
}