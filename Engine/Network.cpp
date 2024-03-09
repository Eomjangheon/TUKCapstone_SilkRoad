#include "Pch.h"

#include "Network.h"
#include "Input.h"
#include "Scene.h"
#include "SceneManager.h"
#include "GameObject.h"
#include "Transform.h"

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

	m_gameData[0].pos = Vec3{ 0, 0, 0 };
	m_gameData[0].id = 0;
	m_gameData[1].pos = Vec3{ 0, 0, 0 };
	m_gameData[1].id = 1;
	m_lastGameData[0].pos = Vec3{ 0, 0, 0 };
	m_lastGameData[0].id = 0;
	m_lastGameData[1].pos = Vec3{ 0, 0, 0 };
	m_lastGameData[1].id = 1;
}

void Host::MainLoop()
{
	while (GetState() != NETWORK_STATE::GUEST) {
		{
			Packet packet;
			if (GetState() == NETWORK_STATE::HOST) {
				// �Խ�Ʈ�� ���� ��Ŷ�� �޾� ����
				for (auto& guest : m_guestInfos) {
					while (guest.eventQue->toServer.TryPop(packet))
						m_gameData[guest.id] = packet;
				}
			}
			// ȣ��Ʈ Ŭ���̾�Ʈ�� Ǫ���� ��Ŷ�� �޾� ����
			string str = "MainLoop: Queue Size - " + to_string(m_eventQue.toServer.Size()) + "\n";
			while (m_eventQue.toServer.TryPop(packet)) {
				m_gameData[0] = packet;
			}
		}
		GameLoop();
		{
			//Packet packet;
			//if (GetState() == NETWORK_STATE::HOST) {
			//	// �Խ�Ʈ���� ���� ��Ŷ�� ť�� Ǫ��
			//	for (auto& guest : m_guestInfos)
			//		guest.eventQue.get()->toClient.Push(packet);
			//}
			//// ȣ��Ʈ Ŭ���̾�Ʈ���� ���� ��Ŷ�� ť�� Ǫ��	
			//m_eventQue.toClient.Push(packet);
		}
	}
	OutputDebugString(L"Host MainLoop End\n");
}

void Host::GameLoop()
{
	for (int i = 0; i < 2; i++) {
		if (m_gameData[i].pos != m_lastGameData[i].pos) {
			m_lastGameData[i] = m_gameData[i];

			for (auto& guest : m_guestInfos) {
				guest.eventQue->toClient.Push(m_gameData[i]);
			}
			m_eventQue.toClient.Push(m_gameData[i]);
		}
	}
}

void Host::Update()
{
	int count = 0;
	Packet packet;
	while (m_eventQue.toClient.TryPop(packet)) {
		// ���ӿ� ����
		if (packet.id == 0) {
			string str = "Update: Packet " + to_string(count++) + " - " + to_string(packet.pos.x) + ", " + to_string(packet.pos.y) + ", " + to_string(packet.pos.z) + "\n";
			OutputDebugStringA(str.c_str());
		}
		if (packet.id == 1) {
			string str2 = "Update: Packet2 " + to_string(count++) + " - " + to_string(packet.pos.x) + ", " + to_string(packet.pos.y) + ", " + to_string(packet.pos.z) + "\n";
			OutputDebugStringA(str2.c_str());
			break;
		}

		shared_ptr<GameObject> player = GET_SINGLE(SceneManager)->GetActiveScene()->GetPlayer();
		player->GetTransform()->SetLocalPosition(packet.pos);
	}
}

void Host::RunMulti()
{
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

void Host::Stop()
{
	//m_waitLoopThread.join();
	//m_mainLoopThread.join();
}

void Host::WaitLoop()
{
	struct sockaddr_in clientAddr;
	int addrLen;
	m_guestInfos.reserve(MAX_GUEST);
	int playerCount = 0;
	while (GetState() == NETWORK_STATE::HOST) {
		// WaitForClients
		addrLen = sizeof(clientAddr);
		SOCKET tempSocket = ::accept(m_listenSocket, (sockaddr*)&clientAddr, &addrLen);
		if (tempSocket == INVALID_SOCKET) {
			throw runtime_error("Fail accept client socket");
		}

		DWORD optval = 10;
		setsockopt(tempSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&optval, sizeof(optval));

		GuestInfo guest;
		guest.id = ++playerCount;
		guest.socket = move(tempSocket);
		m_guestInfos.emplace_back(guest);

		thread connectionThread = thread{ &Host::Connection, this, guest.id };
		connectionThread.detach();
	}
	closesocket(m_listenSocket);
	OutputDebugString(L"Host WaitLoop End\n");
}

void Host::Connection(ushort id)
{
	SOCKET socket;
	shared_ptr<PacketQueue> eventQue;
	for (auto& guest : m_guestInfos) {
		if (guest.id == id) {
			socket = guest.socket;
			eventQue = guest.eventQue;
			break;
		}
	}

	// �Խ�Ʈ�� ��Ŷ �ۼ���
	int retval;
	while (GetState() == NETWORK_STATE::HOST) {
		Packet packet;
		// �������� �Խ�Ʈ�� ������ ��Ŷ
		while (eventQue->toClient.TryPop(packet)) {
			retval = send(socket, (char*)&packet, sizeof(packet), 0);
			if (retval == SOCKET_ERROR) {

			}
		}
		// �Խ�Ʈ���� ������ ������ ��Ŷ
		while (true) {
			retval = recv(socket, (char*)&packet, sizeof(packet), 0);
			if (retval > 0) {
				eventQue->toServer.Push(packet);
			}
			if (retval < 0) {
				break;
			}
		}
	}
}
void Host::Send(Packet packet)
{
	m_eventQue.toServer.Push(packet);
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

	DWORD optval = 5;
	setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&optval, sizeof(optval));
}

void Guest::Update()
{
	// ��Ŷ�� �ް� ���ӿ� �����ϴ� �κ�
	int count = 0;
	int retval;
	shared_ptr<Packet> packet = make_shared<Packet>();
	while (Recv(packet)) {
		string str = "Update: Packet " + to_string(count++) + " - " + to_string(packet->pos.x) + ", " + to_string(packet->pos.y) + ", " + to_string(packet->pos.z) + "\n";
		OutputDebugStringA(str.c_str());

		if (packet->id == 0)
			break;

		shared_ptr<GameObject> player = GET_SINGLE(SceneManager)->GetActiveScene()->GetPlayer();
		player->GetTransform()->SetLocalPosition(packet->pos);
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
		SetNetworkState(NETWORK_STATE::HOST);

		dynamic_cast<Host*>(m_network.get())->RunMulti();
	}
}

void NetworkManager::ConnectAsGuest()
{
	if (GetNetworkState() != NETWORK_STATE::GUEST) {
		SetNetworkState(NETWORK_STATE::GUEST);

		dynamic_cast<Host*>(m_network.get())->Stop();

		m_network.release();
		m_network = make_unique<Guest>();

		dynamic_cast<Guest*>(m_network.get())->Connect();
	}
}

void NetworkScript::LateUpdate()
{
	if (INPUT->GetButtonDown(KEY_TYPE::KEY_1)) {
		GET_SINGLE(NetworkManager)->RunMulti();
		m_id = 0;
	}

	if (INPUT->GetButtonDown(KEY_TYPE::KEY_2)) {
		// ȣ��Ʈ���� �Խ�Ʈ�� ��ȯ
		GET_SINGLE(NetworkManager)->ConnectAsGuest();
		m_id = 1;
	}

	if (INPUT->GetButtonDown(KEY_TYPE::KEY_3)) {
		// �Խ�Ʈ���� ȣ��Ʈ�� ��ȯ
	}

	Packet packet;
	shared_ptr<GameObject> player = GET_SINGLE(SceneManager)->GetActiveScene()->GetPlayer();
	packet.pos = player->GetTransform()->GetLocalPosition();
	packet.id = m_id;
	//string str = "Send: " + to_string(packet.pos.x) + ", " + to_string(packet.pos.y) + ", " + to_string(packet.pos.z) + "\n";
	//OutputDebugStringA(str.c_str());
	GET_SINGLE(NetworkManager)->Send(packet);
}
#pragma endregion