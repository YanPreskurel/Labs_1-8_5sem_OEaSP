#include <winsock2.h>
#include <Ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>

// Отправка файлов
void SendFile(SOCKET clientSocket, const std::string& filePath)
{
    char buffer[1024];
    int bytesRead; // Байты, считанные из файла

    std::string fileName = filePath.substr(filePath.find_last_of("\\/") + 1);

    // Отправляем команду "UPLOAD" серверу
    std::string uploadCommand = "UPLOAD";

    // Команда возвращает -1 в случае ошибки
    if (send(clientSocket, uploadCommand.c_str(), uploadCommand.size(), 0) < 0)
    {
        std::cerr << "Error sending the upload command." << std::endl;
        return;
    }

    // Отправляем имя файла серверу
    if (send(clientSocket, fileName.c_str(), fileName.size(), 0) < 0)
    {
        std::cerr << "Error sending the file name." << std::endl;
        return;
    }

    std::ifstream fileStream(filePath, std::ios::in | std::ios::binary);

    if (!fileStream.is_open())
    {
        std::cerr << "Error opening file for reading: " << filePath << std::endl;
        return;
    }

    while (!fileStream.eof())
    {
        fileStream.read(buffer, sizeof(buffer));
        bytesRead = static_cast<int>(fileStream.gcount());

        if (bytesRead > 0)
        {
            send(clientSocket, buffer, bytesRead, 0);
        }
    }

    fileStream.close();
    std::cout << "Sent file to the server: " << filePath << std::endl;
}

void ReceiveFile(SOCKET clientSocket)
{
    char buffer[1024];
    int bytesRead;

    // Отправляем команду "DOWNLOAD" серверу
    std::string downloadCommand = "DOWNLOAD";
    if (send(clientSocket, downloadCommand.c_str(), downloadCommand.size(), 0) < 0)
    {
        std::cerr << "Error sending the download command." << std::endl;
        return;
    }

    std::cout << "Enter the file name you want to download: ";
    std::string fileNameToDownload;
    std::cin >> fileNameToDownload;

    // Отправляем имя файла, который хотим скачать
    if (send(clientSocket, fileNameToDownload.c_str(), fileNameToDownload.size(), 0) < 0)
    {
        std::cerr << "Error sending the file name to download." << std::endl;
        return;
    }

    std::string savePath = fileNameToDownload;

    std::ofstream receivedFile(savePath, std::ios::out | std::ios::binary);
    if (!receivedFile.is_open())
    {
        std::cerr << "Error opening file for writing: " << savePath << std::endl;
        return;
    }
    else
    {
        while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
        {
            receivedFile.write(buffer, bytesRead);
        }
        receivedFile.close();
        std::cout << "Received and saved file: " << fileNameToDownload << std::endl;
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "Error initializing Winsock." << std::endl;
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    // Протокол TCP, конкретный протокол не указан, что будет использован tcp
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Error creating a socket." << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; // Использование айпиви4
    inet_pton(AF_INET, "127.0.0.1", &(serverAddr.sin_addr));
    serverAddr.sin_port = htons(12345);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Error connecting to the server." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::string name = "Client1"; // Уникальное имя клиента
    send(clientSocket, name.c_str(), name.size(), 0);

    std::cout << "Choose an option:\n1. Upload a file to the server\n2. Download a file from the server\n";
    int option;
    std::cin >> option;

    if (option == 1)
    {
        std::cout << "Enter the full path to the file you want to send: ";
        std::string filePath;
        std::cin >> filePath;
        SendFile(clientSocket, filePath);
    }
    else if (option == 2)
    {
        ReceiveFile(clientSocket);
    }
    else
    {
        std::cerr << "Invalid option." << std::endl;
    }
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
