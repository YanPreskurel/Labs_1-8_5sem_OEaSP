#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// Структура для взаимодейтсвия с клиентом
struct ClientInfo 
{
    std::string name;
    SOCKET socket;
};

std::vector<ClientInfo> clients;

// Обработка взаимодействий с клиентом
void HandleClient(ClientInfo client) 
{
    char buffer[1024]; // Буфер для хранения данных, прочитанных из сокета
    int bytesRead; // Отслеживание количества байтов, полученных из сокета

    while (true)
    {
        bytesRead = recv(client.socket, buffer, sizeof(buffer), 0); // считывание данных из сокета
        if (bytesRead <= 0)
        {
            // Входим в ветвление, если клиент отключился или произошла ошибка в вводе запроса клиентом
            for (auto it = clients.begin(); it != clients.end(); ++it)
            {
                if (it->socket == client.socket)
                {
                    std::cout << "Client " << it->name << " disconnected." << std::endl;
                    closesocket(client.socket);
                    clients.erase(it);

                    break;
                }
            }
            return;
        }

        // Определяем тип команды (UPLOAD или DOWNLOAD)
        std::string command(buffer, bytesRead);

        if (command == "UPLOAD") 
        {
            // Получаем имя файла
            bytesRead = recv(client.socket, buffer, sizeof(buffer), 0); // прием данных из сокета.
            if (bytesRead <= 0)
            {
                std::cerr << "Error receiving file name." << std::endl;
                return;
            }

            std::string fileName(buffer, bytesRead);

            // Получаем и сохраняем файл
            std::ofstream receivedFile(fileName, std::ios::out | std::ios::binary); // открываю в двоичном

            while ((bytesRead = recv(client.socket, buffer, sizeof(buffer), 0)) > 0) 
            {
                receivedFile.write(buffer, bytesRead); // записываю из сокета в буфер
            }
            receivedFile.close();
            std::cout << "Sent file to " << client.name << ": " << fileName << std::endl;


        }
        else if (command == "DOWNLOAD") 
        {
            // Получаем имя файла для отправки
            bytesRead = recv(client.socket, buffer, sizeof(buffer), 0);

            if (bytesRead <= 0) 
            {
                std::cerr << "Error receiving file name." << std::endl;
                return;
            }
            std::string fileName(buffer, bytesRead);

            // Отправить файл
            std::ifstream fileStream(fileName, std::ios::in | std::ios::binary); // для чтения 

            if (fileStream.is_open())
            {
                while (!fileStream.eof()) // проверка конца
                {
                    fileStream.read(buffer, sizeof(buffer)); 
                    bytesRead = static_cast<int>(fileStream.gcount()); // прочитанные байты 

                    if (bytesRead > 0) 
                    {
                        send(client.socket, buffer, bytesRead, 0); // отправка через файловый дескриптор сокета
                    }
                }
                fileStream.close();
                std::cout << "Sent file to " << client.name << ": " << fileName << std::endl;

            }
            else
            {
                std::cerr << "Error opening file for sending: " << fileName << std::endl;
            }
        }
    }
}

 // новый поток для обработки клиента
DWORD WINAPI HandleClientThread(LPVOID lpParam)
{
    ClientInfo* pClient = reinterpret_cast<ClientInfo*>(lpParam); // преобразую
    HandleClient(*pClient); // обработка клиента
    return 0;
}

int main()
{
    WSADATA wsaData; // Инициализация сокета
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) // макрос для версии 2.2 
    {
        std::cerr << "Error initializing Winsock." << std::endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0); // сокет использует TCP или TCP/IP
    if (serverSocket == INVALID_SOCKET) 
    {
        std::cerr << "Error creating a socket." << std::endl;
        WSACleanup(); // Завершение работы библиотеки
        return 1;
    }

    sockaddr_in serverAddr; // Адрес сервера для прослушивания входящих сооединений
    serverAddr.sin_family = AF_INET; // Семейство адресов для сервера
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Определение IP адреса
    serverAddr.sin_port = htons(12345); // привязка к порту 

    // Привязка к заданному локальному адресу и порту
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Error binding the socket to a port." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Настройка серверного сокета на прослушивание входящих соединений
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    { // макс. длина очереди ожидания соединения
        std::cerr << "Error listening to the socket." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "The server is running and listening on port 12345." << std::endl;

    while (true)
    {
        sockaddr_in clientAddr; // Адрес клиента для прослушивания входящих сообщений
        int clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        // заполнение структуры информацией о клиенте (IP адрес и порт)

        if (clientSocket != INVALID_SOCKET)
        {
            ClientInfo newClient;
            newClient.socket = clientSocket;

            char nameBuffer[1024];
            int nameBytes = recv(clientSocket, nameBuffer, sizeof(nameBuffer), 0);
            if (nameBytes > 0) 
            {
                newClient.name.assign(nameBuffer, nameBytes); // Присвоение имени клиента
            }

            std::cout << "Client " << newClient.name << " connected." << std::endl;
            clients.push_back(newClient);

            HANDLE hThread = CreateThread(NULL, 0, HandleClientThread, &newClient, 0, NULL);
            // уровень безопасности род проц, размер стека нового потока,.., флаги по умолчанию, идентификатор создания потока
            if (hThread == NULL) 
            {
                std::cerr << "Error creating thread." << std::endl;
                return 1;
            }
        }
    }
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}