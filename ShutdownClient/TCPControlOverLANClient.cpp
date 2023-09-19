#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")

using namespace std;

unsigned get_hex_from_string(const std::string& s)
{
	unsigned hex{ 0 };

	for (size_t i = 0; i < s.length(); ++i) {
		hex <<= 4;
		if (isdigit(s[i])) {
			hex |= s[i] - '0';
		}
		else if (s[i] >= 'a' && s[i] <= 'f') {
			hex |= s[i] - 'a' + 10;
		}
		else if (s[i] >= 'A' && s[i] <= 'F') {
			hex |= s[i] - 'A' + 10;
		}
		else {
			throw std::runtime_error("Failed to parse hexadecimal " + s);
		}
	}
	return hex;
}

std::string get_ether(const std::string& hardware_addr)
{
	std::string ether_addr;

	for (size_t i = 0; i < hardware_addr.length();) {
		unsigned hex = get_hex_from_string(hardware_addr.substr(i, 2));
		i += 2;

		ether_addr += static_cast<char>(hex & 0xFF);

		if (hardware_addr[i] == ':')
			++i;
	}

	if (ether_addr.length() != 6)
		throw std::runtime_error(hardware_addr + " not a valid ether address");

	return ether_addr;
}



void wakeOnLan(std::string hardware_addr, unsigned long bcast, int port = 9) {

	WSADATA wsa_data;

	WSAStartup(MAKEWORD(2, 2), &wsa_data);


	const std::string ether_addr{ get_ether(hardware_addr) };

	int descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (descriptor < 0) {
		throw std::runtime_error("Unable to open socket");
	}
 
	std::string message(6, 0xFF);
	for (size_t i = 0; i < 16; ++i) {
		message += ether_addr;
	}

	const char optval{ 1 };
	if (setsockopt(descriptor, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) < 0) {
		throw std::runtime_error("Failed to set socket options");
	}

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = bcast;
	addr.sin_port = htons(port);

	if (sendto(descriptor, message.c_str(), message.length(), 0,
		reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
		cout << WSAGetLastError() << endl;
		throw std::runtime_error("Failed to send packet ");
	}
	cout << "WakeOnLAN Sent";
}

int sendShutdownPacket(std::string address, int port) {
	string buffer = "0";

	WSADATA wsa_data;
	SOCKADDR_IN addr;

	WSAStartup(MAKEWORD(2, 2), &wsa_data);
	const auto server = socket(AF_INET, SOCK_STREAM, 0);
	
	InetPton(AF_INET, std::wstring(address.begin(), address.end()).c_str(), &addr.sin_addr.s_addr);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	int resultCode = connect(server, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr));
	cout << "Connected to server! " << resultCode << endl;

	resultCode = send(server, buffer.c_str(), buffer.length(), 0);
	cout << "Message sent!" << resultCode << endl;

	closesocket(server);
	WSACleanup();
	cout << "Socket closed." << endl << endl;
	return 0;
}


int main(int argc, char* const argv[])
{
	if (argc < 4 || argc > 5){
		cout << "Invalid command prompt";
		return -1;
	}
	try {
		int32_t commandResult = 0;
		if (string(argv[1]) == "shutdown") {
			commandResult = sendShutdownPacket(argv[2], atoi(argv[3]));
			return 0;
		}
		else if (string(argv[1]) == "wakeonlan") {
			if (argc == 4) 
				wakeOnLan(argv[2], inet_addr(argv[3]));
			else if (argc == 5)
				wakeOnLan(argv[2], inet_addr(argv[3]), atoi(argv[4]));
		}
		else {
			cout << "Unknown command to send: " << argv[1];
			return -1;
		}

		return commandResult;
	}
	catch (exception e) {
		cout << e.what();
		return -1;
	}
}

