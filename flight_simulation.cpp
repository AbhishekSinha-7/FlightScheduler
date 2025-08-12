#include <iostream>
#include <vector>
#include <fstream>
#include <time.h>
#include <queue>
#include <set>
#include <random>
#include <windows.h>

using namespace std;

// This struct provides information sharing between client and server thread
struct Pipe {
    queue<int> clientToServerMessages;
    queue<bool> serverToClientMessages;
};

// This struct is for storing information of each Client
struct Client {
    int sleepTime;
    int clientId;
    int reservedSeatNo;
    int pipeId;
};

int clientSize;
ofstream outputTextFile;
set<int> availableSeats;
vector<Pipe> pipes;
vector<Client> clients;
vector<HANDLE> semClient;  // Semaphores for client signaling
vector<HANDLE> semServer;  // Semaphores for server signaling
CRITICAL_SECTION reservationMutex;
CRITICAL_SECTION writeMutex;

DWORD WINAPI ClientFun(LPVOID data) {
    Client* client = (Client*)data;
    Sleep(client->sleepTime / 1000);  // Convert microseconds to milliseconds
    Pipe* p = &pipes[client->pipeId];
    srand((unsigned int)time(NULL));
    
    EnterCriticalSection(&reservationMutex);
    int size = availableSeats.size();
    set<int>::iterator it = availableSeats.begin();
    advance(it, rand() % size);
    int seatNo = *it;
    availableSeats.erase(it);
    LeaveCriticalSection(&reservationMutex);
    
    p->clientToServerMessages.push(seatNo);
    ReleaseSemaphore(semClient[client->clientId - 1], 1, NULL);
    WaitForSingleObject(semServer[client->clientId - 1], INFINITE);
    
    if (p->serverToClientMessages.front()) {
        p->serverToClientMessages.pop();
    }
    return 0;
}

DWORD WINAPI ServerFun(LPVOID data) {
    Client* client = (Client*)data;
    Pipe* p = &pipes[client->pipeId];
    
    WaitForSingleObject(semClient[client->clientId - 1], INFINITE);
    int seatNo = p->clientToServerMessages.front();
    p->clientToServerMessages.pop();
    client->reservedSeatNo = seatNo;
    
    EnterCriticalSection(&writeMutex);
    outputTextFile << "Client" << client->clientId << " reserves Seat" << client->reservedSeatNo << endl;
    LeaveCriticalSection(&writeMutex);
    
    p->serverToClientMessages.push(true);
    ReleaseSemaphore(semServer[client->clientId - 1], 1, NULL);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "You need to give an argument" << endl;
        cout << "e.g. flight_simulation.exe [seat_number]" << endl;
        return 0;
    }
    
    clientSize = atoi(argv[1]);
    InitializeCriticalSection(&reservationMutex);
    InitializeCriticalSection(&writeMutex);

    // Initialize semaphores
    for (int i = 0; i < clientSize; i++) {
        semClient.push_back(CreateSemaphore(NULL, 0, 1, NULL));
        semServer.push_back(CreateSemaphore(NULL, 0, 1, NULL));
    }
    
    outputTextFile.open("output.txt");
    outputTextFile << "Number of total seats: " << clientSize << endl;
    
    for (int i = 0; i < clientSize; i++) {
        availableSeats.insert(i + 1);
    }
    
    vector<HANDLE> clientThreads(clientSize);
    vector<HANDLE> serverThreads(clientSize);
    srand((unsigned int)time(NULL));
    
    for (int i = 0; i < clientSize; i++) {
        Client data;
        data.clientId = i + 1;
        data.sleepTime = (rand() % 151 + 50) * 1000;
        data.reservedSeatNo = -1;
        data.pipeId = i;
        clients.push_back(data);
        Pipe pipe;
        pipes.push_back(pipe);
    }
    
    for (int i = 0; i < clientSize; i++) {
        clientThreads[i] = CreateThread(NULL, 0, ClientFun, &clients[i], 0, NULL);
        serverThreads[i] = CreateThread(NULL, 0, ServerFun, &clients[i], 0, NULL);
    }
    
    WaitForMultipleObjects(clientSize, serverThreads.data(), TRUE, INFINITE);
    
    outputTextFile << "All seats are reserved." << endl;
    outputTextFile.close();
    
    // Cleanup
    DeleteCriticalSection(&reservationMutex);
    DeleteCriticalSection(&writeMutex);
    for (int i = 0; i < clientSize; i++) {
        CloseHandle(clientThreads[i]);
        CloseHandle(serverThreads[i]);
        CloseHandle(semClient[i]);
        CloseHandle(semServer[i]);
    }
    
    return 0;
}