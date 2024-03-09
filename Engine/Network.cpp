#include "Pch.h"

#include "Network.h"
#include "Input.h"

#pragma region Legacy
/*
void Network::Run()
{
	thread serverThread{ &Network::MainLoop, this };
	m_serverThread = move(serverThread);
	m_isRunning = true;
}

void Network::Stop()
{
	// Host���� Guest �÷��̷� ��ȯ�� �� ȣ��
	if (m_isRunning) {
		m_isRunning = false;
		m_serverThread.join();

		SetState(NETWORK_STATE::GUEST_DISCONNECTED);
	}
}

void Network::Initialize()
{
	if (WSAStartup(MAKEWORD(2, 2), &m_wsaData) != 0) {
		throw runtime_error("Fail initialize WSADATA");
	}
	Run();
}

void Network::Host_RunMulti()
{
	SetState(NETWORK_STATE::HOST_MULTI);

	//if (WSAStartup(MAKEWORD(2, 2), &m_wsaData) != 0) {
	//	throw runtime_error("Fail initialize WSADATA");
	//}

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET) {
		throw runtime_error("Fail initialize listen socket");
	}

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(9000);
	if (::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		throw runtime_error("Fail bind listen socket");
	};

	if (listen(listenSocket, MAX_GUEST) == SOCKET_ERROR) {
		throw runtime_error("Fail listen listen socket");
	}

	m_listenSocket = move(listenSocket);

	thread waitThread{ &Network::Host_WaitLoop, this };
	m_waitThread = move(waitThread);
}

void Network::MainLoop()
{
	Packet inPacket, outPacket;
	while (m_isRunning) {
		// Ŭ�� �����忡�� ��Ŷ�� �޾� ����
		while (m_packetQueue.in.TryPop(inPacket)) {
			// ��Ŷ ó��
		}

		// �Խ�Ʈ�� ���� ��Ŷ�� �޾� ����
		for (auto& guest : m_guestInfo) {
			while (guest.packetQueue.in.TryPop(inPacket)) {
				// ��Ŷ ó��
			}
		}

		// outPacket = GameLoop();

		// Ŭ�� ������� ��Ŷ�� ����
		m_packetQueue.out.Push(outPacket);

		// �Խ�Ʈ���� ��Ŷ�� ����
		for (auto& guest : m_guestInfo) {
			guest.packetQueue.out.Push(outPacket);
		}

		// Test
		// ���� ���� ���� �ۼ� �� �ӽ÷� ���
		// ��� �÷��̾��� ��ġ�� 1/60�ʸ��� ����
		// ��� �÷��̾ ���� �÷��̾��� ��ġ�� �޾Ƽ� �״�� �׷���
	}
}

void Network::Host_WaitLoop()
{
	struct sockaddr_in clientAddr;
	int addrLen;
	m_guestInfo.reserve(MAX_GUEST);
	int playerCount = 0;
	while (m_isRunning && m_networkState == NETWORK_STATE::HOST_MULTI) {
		// WaitForClients
		SOCKET tempSocket = ::accept(m_listenSocket, reinterpret_cast<struct sockaddr*>(&clientAddr), &addrLen);
		if (tempSocket == INVALID_SOCKET) {
			throw runtime_error("Fail accept client socket");
		}
		ClientInfo clientInfo{ playerCount++, tempSocket };
		m_guestInfo.emplace_back(clientInfo);

		// ȣ��Ʈ����-�Խ�Ʈ ��� ����
		thread guestConnectionThread;
	}
}

void Network::Guest_Connect()
{
	// ���� ���� ������ ����
	Stop();

	Guest tempGuest;

	// ���õ� ���� �ʱ�ȭ
	tempGuest.socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tempGuest.socket == INVALID_SOCKET) {
		throw runtime_error("Fail initialize socket");
	}

	// ȣ��Ʈ�� IP�ּҸ� ���� ȣ��Ʈ ������ ����
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(9000);
	if (connect(tempGuest.socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		throw runtime_error("Fail connect server socket");
	}

	// ȣ��Ʈ�� �����͸� �ۼ����� �� �ִ� ������ ����
	thread guestConnectionThread{ &Network::Guest_Connection, this };
	tempGuest.connectionThread = move(guestConnectionThread);

	m_guestInfo.emplace_back(tempGuest);
}

// ������ �Խ�Ʈ���� �����带 ����
// �׷��� ������ �ӿ� ������ ������ ��� �ϳ�?...
// Network Ŭ���� ���ο� Guest, Host(Server) Ŭ������ ����
// ���/���� �ؾ� �� �� ����...
void Network::Guest_Connection()
{
	// ushort id = id; ???

	// ������ �Խ�Ʈ ��Ŷ �ۼ���
	// ȣ��Ʈ�� ������ ���ӵǴ� ����
	while (true) {

	}
}

void Network::Send(Packet packet)
{
	if (m_networkState == NETWORK_STATE::HOST_MULTI) {
		// ȣ��Ʈ �������� �Խ�Ʈ�� ��Ŷ ����
		PushPacket(packet);
	}
	else if (m_networkState == NETWORK_STATE::GUEST_CONNECTED) {
		// �Խ�Ʈ���� ȣ��Ʈ ������ ��Ŷ ����
		SendPacket(packet);
	}
}

Packet Network::Recv()
{
	if (m_networkState == NETWORK_STATE::HOST_MULTI) {
		// ȣ��Ʈ �������� �Խ�Ʈ�κ��� ��Ŷ ����
		return PopPacket();
	}
	else if (m_networkState == NETWORK_STATE::GUEST_CONNECTED) {
		// �Խ�Ʈ���� ȣ��Ʈ �����κ��� ��Ŷ ����
		return RecvPacket();
	}

	return Packet();
}

void Network::SendPacket(Packet packet)
{
	// �Խ�Ʈ���� ȣ��Ʈ ������ ��Ŷ ����
	Packet tempPacket;
	for (auto& guest : m_guestInfo) {
		while (guest.toHostEvent.TryPop()) {
			if (send(guest.socket, reinterpret_cast<char*>(&tempPacket), sizeof(Packet), 0) == SOCKET_ERROR) {
				throw runtime_error("Fail send packet");
			}
		}
	}
}

Packet Network::RecvPacket()
{
	// �Խ�Ʈ���� ȣ��Ʈ �����κ��� ��Ŷ ����
	Packet tempPacket;
	for (auto& guest : m_guestInfo) {
		while (true) {
			int returnValue = recv(guest.socket, reinterpret_cast<char*>(&tempPacket), sizeof(Packet), 0);
			if (returnValue == SOCKET_ERROR) {
				throw runtime_error("Fail recv packet");
			}
			else {
				return tempPacket;
			}
		}
	}
	return Packet();
}

void Network::PushPacket(Packet packet)
{
	m_packetQueue.Push(packet);
}

Packet Network::PopPacket()
{
	Packet packet;
	m_packetQueue.WaitPop(packet);
	return packet;
}

*/
#pragma endregion

Network::Network()
{
	if (WSAStartup(MAKEWORD(2, 2), &m_wsaData) != 0) {
		throw runtime_error("Fail initialize WSADATA");
	}
}

#pragma region Host
Host::Host() : Network()
{
	m_mainLoopThread = thread{ &Host::MainLoop, this };
}

void Host::MainLoop()
{
	// ���⼭ ���� ���� ó��
	//Packet packet;
	//while (m_packetQueue.TryPop(packet)) {
	//	// ���ӿ� ����
	//}
	//for (auto& guest : m_guestInfos) {
	//	while (guest.packetQueue.in.TryPop(packet)) {
	//		// ���ӿ� ����
	//	}
	//}

	//GameLoop();

	//for (auto& guest : m_guestInfos) {
	//	guest.packetQueue.out.Push(packet);
	//}
	//m_packetQueue.Push(packet);
}

void Host::GameLoop()
{

}

void Host::Update()
{
	Packet packet;
	while (m_packetQueue.TryPop(packet)) {
		// ���ӿ� ����
	}
}

void Host::RunMulti()
{
	m_isMultiRunning = true;

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET) {
		throw runtime_error("Fail initialize listen socket");
	}

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(9000);
	if (::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		throw runtime_error("Fail bind listen socket");
	};

	if (listen(listenSocket, MAX_GUEST) == SOCKET_ERROR) {
		throw runtime_error("Fail listen listen socket");
	}

	m_listenSocket = move(listenSocket);

	m_waitLoopThread = thread{ &Host::WaitLoop, this };
}

void Host::WaitLoop()
{
	struct sockaddr_in clientAddr;
	int addrLen;
	m_guestInfos.reserve(MAX_GUEST);
	int playerCount = 0;
	while (m_isMultiRunning) {
		// WaitForClients
		addrLen = sizeof(clientAddr);
		SOCKET tempSocket = ::accept(m_listenSocket, (sockaddr*)&clientAddr, &addrLen);
		if (tempSocket == INVALID_SOCKET) {
			throw runtime_error("Fail accept client socket");
		}

		DWORD optval = 10;
		setsockopt(tempSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&optval, sizeof(optval));

		GuestInfo guest;
		guest.id = playerCount++;
		guest.socket = move(tempSocket);
		m_guestInfos.emplace_back(guest);

		thread connectionThread = thread{ &Host::Connection, this, guest.id };
		connectionThread.detach();
	}
}

//thread_local shared_ptr<PacketQueue> tl_packetQueue = make_shared<PacketQueue>();

void Host::Connection(ushort id)
{
	// �Խ�Ʈ�� ��Ŷ �ۼ���
	int retval;
	for (auto& guest : m_guestInfos) {
		if (guest.id == id) {
			//while (true) {
			//	Packet packet;
			//	// �������� �Խ�Ʈ�� ������ ��Ŷ
			//	while (tl_packetQueue.get()->out.TryPop(packet)) {
			//		retval = send(guest.socket, (char*)&packet, sizeof(packet), 0);
			//		if (retval == SOCKET_ERROR) {

			//		}
			//	}
			//	// �Խ�Ʈ���� ������ ������ ��Ŷ
			//	while (true) {
			//		retval = recv(guest.socket, (char*)&packet, sizeof(packet), 0);
			//		if (retval > 0) {
			//			tl_packetQueue.get()->in.Push(packet);
			//		}
			//		if (retval < 0) {
			//			break;
			//		}
			//	}
			//}
		}
	}
}
#pragma endregion

#pragma region Guest
Guest::Guest() : Network()
{

}

void Guest::Connect()
{
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == INVALID_SOCKET) {
		throw runtime_error("Fail initialize socket");
	}

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, m_serverIP, &serverAddr.sin_addr);
	serverAddr.sin_port = htons(SERVER_PORT);
	int retval = connect(m_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (retval == SOCKET_ERROR) {
		//err_display(retval);
		throw runtime_error("Fail connect server");
	}

	DWORD optval = 10;
	setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&optval, sizeof(optval));
}

void Guest::Update()
{
	// ��Ŷ�� �ް� ���ӿ� �����ϴ� �κ�
	int retval;
	shared_ptr<Packet> packet = make_shared<Packet>();
	while (Recv(packet)) {
		// ���ӿ� ����
	}
}

void Guest::Send(Packet packet)
{
	int retval = send(m_socket, (char*)&packet, sizeof(Packet), 0);
	if (retval < 0) {
		//err_display(retval);
	}
}

bool Guest::Recv(shared_ptr<Packet> packet)
{
	int retval = recv(m_socket, (char*)packet.get(), sizeof(Packet), 0);
	if (retval < 0) {
		return false;
	}
	return true;
}
#pragma endregion

#pragma region Network
void NetworkManager::Initialize()
{
	m_network = make_unique<Host>();
}

void NetworkManager::Update()
{
	m_network.get()->Update();
}

void NetworkManager::RunMulti()
{
	if (GetNetworkState() == NETWORK_STATE::SINGLE) {
		dynamic_cast<Host*>(m_network.get())->RunMulti();

		SetNetworkState(NETWORK_STATE::HOST);
	}
}

void NetworkManager::ConnectAsGuest()
{
	if (GetNetworkState() == NETWORK_STATE::SINGLE) {
		m_network.reset();
		m_network = make_unique<Guest>();

		dynamic_cast<Guest*>(m_network.get())->Connect();

		SetNetworkState(NETWORK_STATE::GUEST);
	}
}

void NetworkScript::LateUpdate()
{
	if (INPUT->GetButtonDown(KEY_TYPE::KEY_1)) {
		GET_SINGLE(NetworkManager)->RunMulti();
	}

	if (INPUT->GetButtonDown(KEY_TYPE::KEY_2)) {
		// ȣ��Ʈ���� �Խ�Ʈ�� ��ȯ
		GET_SINGLE(NetworkManager)->ConnectAsGuest();
	}

	if (INPUT->GetButtonDown(KEY_TYPE::KEY_3)) {
		// �Խ�Ʈ���� ȣ��Ʈ�� ��ȯ
	}
}
#pragma endregion