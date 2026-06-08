#pragma once

#include <string>
#include <vector>
#include <cstring>
#include <stdexcept>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

class UdpListener {
public:
	struct ReceivedPacket {
		std::vector<uint8_t> data;
		sockaddr_in sender;
		std::string host;
		uint16_t port;
	};

	explicit UdpListener(uint16_t port = 5111) : port_(port), sock_(INVALID_SOCKET), blocking_(true) {
#ifdef _WIN32
		// Ініціалізація Winsock
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			throw std::runtime_error("WSAStartup failed");
		}
		wsaInitialized_ = true;
#endif

		sock_ = socket(AF_INET, SOCK_DGRAM, 0);
		if (sock_ == INVALID_SOCKET) {
			cleanup();
			throw std::runtime_error("Failed to create socket");
		}

		// Дозволяємо прийом broadcast пакетів
		int broadcast = 1;
		if (setsockopt(sock_, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast, sizeof(broadcast)) < 0) {
			cleanup();
			throw std::runtime_error("Failed to enable broadcast");
		}

		// Дозволяємо повторне використання адреси
		int reuse = 1;
		if (setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
			cleanup();
			throw std::runtime_error("Failed to set SO_REUSEADDR");
		}

		// Bind до порту
		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		addr.sin_port = htons(port_);

		if (bind(sock_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
			cleanup();
			throw std::runtime_error("Failed to bind to port " + std::to_string(port_));
		}
	}

	~UdpListener() {
		cleanup();
	}

	// Заборона копіювання
	UdpListener(const UdpListener&) = delete;
	UdpListener& operator=(const UdpListener&) = delete;

	// Перемикання режиму блокування
	void setBlocking(bool blocking) {
#ifdef _WIN32
		u_long mode = blocking ? 0 : 1;
		if (ioctlsocket(sock_, FIONBIO, &mode) != 0) {
			throw std::runtime_error("Failed to set socket blocking mode");
		}
#else
		int flags = fcntl(sock_, F_GETFL, 0);
		if (flags < 0) {
			throw std::runtime_error("Failed to get socket flags");
		}

		if (blocking) {
			flags &= ~O_NONBLOCK;
		}
		else {
			flags |= O_NONBLOCK;
		}

		if (fcntl(sock_, F_SETFL, flags) < 0) {
			throw std::runtime_error("Failed to set socket blocking mode");
		}
#endif
		blocking_ = blocking;
	}

	bool isBlocking() const {
		return blocking_;
	}

	// Прийом пакету (повертає false якщо немає даних в неблокуючому режимі)
	bool receive(ReceivedPacket& packet, size_t maxSize = 65536) {
		packet.data.resize(maxSize);

		sockaddr_in sender{};
		socklen_t senderLen = sizeof(sender);

		int received = recvfrom(sock_, (char*)packet.data.data(), (int)maxSize, 0,
			(struct sockaddr*)&sender, &senderLen);

		if (received < 0) {
#ifdef _WIN32
			int error = WSAGetLastError();
			if (error == WSAEWOULDBLOCK) {
				return false; // Немає даних в неблокуючому режимі
			}
			throw std::runtime_error("recvfrom failed: error " + std::to_string(error));
#else
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				return false; // Немає даних в неблокуючому режимі
			}
			throw std::runtime_error("recvfrom failed: " + std::string(strerror(errno)));
#endif
		}

		packet.data.resize(received);

		// Save raw sender address
		packet.sender = sender;

		// Convert sender address to string
		char hostBuf[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &sender.sin_addr, hostBuf, INET_ADDRSTRLEN);
		packet.host = hostBuf;
		packet.port = ntohs(sender.sin_port);

		return true;
	}

	// Send to specific host/port
	void sendTo(const std::string& host, uint16_t port, const void* data, size_t size) {
		sockaddr_in dest{};
		dest.sin_family = AF_INET;
		dest.sin_port = htons(port);

		if (inet_pton(AF_INET, host.c_str(), &dest.sin_addr) <= 0) {
			throw std::runtime_error("Invalid host address: " + host);
		}

		int sent = sendto(sock_, (const char*)data, (int)size, 0,
			(struct sockaddr*)&dest, sizeof(dest));

		if (sent < 0) {
#ifdef _WIN32
			throw std::runtime_error("sendto failed: error " + std::to_string(WSAGetLastError()));
#else
			throw std::runtime_error("sendto failed: " + std::string(strerror(errno)));
#endif
		}

		if (static_cast<size_t>(sent) != size) {
			throw std::runtime_error("Partial send: " + std::to_string(sent) + "/" + std::to_string(size));
		}
	}

	// Send directly to the sender from ReceivedPacket (using saved sockaddr_in)
	void sendTo(const ReceivedPacket& packet, const void* data, size_t size) {
		int sent = sendto(sock_, (const char*)data, (int)size, 0,
			(struct sockaddr*)&packet.sender, sizeof(packet.sender));

		if (sent < 0) {
#ifdef _WIN32
			throw std::runtime_error("sendto failed: error " + std::to_string(WSAGetLastError()));
#else
			throw std::runtime_error("sendto failed: " + std::string(strerror(errno)));
#endif
		}

		if (static_cast<size_t>(sent) != size) {
			throw std::runtime_error("Partial send: " + std::to_string(sent) + "/" + std::to_string(size));
		}
	}

	// Convenient overloads for std::vector
	void sendTo(const std::string& host, uint16_t port, const std::vector<uint8_t>& data) {
		sendTo(host, port, data.data(), data.size());
	}

	void sendTo(const ReceivedPacket& packet, const std::vector<uint8_t>& data) {
		sendTo(packet, data.data(), data.size());
	}

	// Convenient overloads for std::string
	void sendTo(const std::string& host, uint16_t port, const std::string& data) {
		sendTo(host, port, data.data(), data.size());
	}

	void sendTo(const ReceivedPacket& packet, const std::string& data) {
		sendTo(packet, data.data(), data.size());
	}

private:
	void cleanup() {
		if (sock_ != INVALID_SOCKET) {
			closesocket(sock_);
			sock_ = INVALID_SOCKET;
		}
#ifdef _WIN32
		if (wsaInitialized_) {
			WSACleanup();
			wsaInitialized_ = false;
		}
#endif
	}

	uint16_t port_;
	SOCKET sock_;
	bool blocking_;
#ifdef _WIN32
	bool wsaInitialized_ = false;
#endif
};
