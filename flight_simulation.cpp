#include <iostream>
#include <vector>
#include <fstream>
#include <queue>
#include <set>
#include <random>
#include <windows.h>

using namespace std;

// Struct for a reservation request
struct Request { 
    int clientId; // ID of client making request
    int seatNo;   // Seat to reserve
};

int clientSize;
ofstream outputTextFile;
set<int> availableSeats;        // Tracks free seats
vector<HANDLE> semClient;       // Each client signals server
HANDLE semServer;               // Server signals back to client
CRITICAL_SECTION reservationMutex, writeMutex, queueMutex;
queue<Request> requestQueue;    // Queue of requests from clients

struct Client { 
    int clientId; 
    int sleepTime;          // Wait before making request (ms)
    int reservedSeatNo;     // Assigned seat number
};

vector<Client> clients;

// Client thread function
DWORD WINAPI ClientFun(LPVOID data) {
    Client* client = (Client*)data;
    Sleep(client->sleepTime); // simulate client thinking

    int seatNo = -1;

    // Pick a seat safely
    EnterCriticalSection(&reservationMutex);
    if (!availableSeats.empty()) {
        auto it = availableSeats.begin();
        advance(it, rand() % availableSeats.size());
        seatNo = *it;
        availableSeats.erase(it);
    }
    LeaveCriticalSection(&reservationMutex);

    if (seatNo != -1) {
        // Push request to queue safely
        EnterCriticalSection(&queueMutex);
        requestQueue.push({client->clientId, seatNo});
        LeaveCriticalSection(&queueMutex);

        ReleaseSemaphore(semClient[client->clientId - 1], 1, NULL); // notify server
        WaitForSingleObject(semServer, INFINITE);                   // wait for server

        client->reservedSeatNo = seatNo; // store seat locally
    }

    return 0;
}

// Single server thread
DWORD WINAPI ServerFun(LPVOID data) {
    int processed = 0;

    while (processed < clientSize) {
        for (int i = 0; i < clientSize; i++) {
            WaitForSingleObject(semClient[i], 0); // check if client signaled

            EnterCriticalSection(&queueMutex);
            while (!requestQueue.empty()) {
                Request req = requestQueue.front();
                requestQueue.pop();

                EnterCriticalSection(&writeMutex);
                outputTextFile << "Client" << req.clientId
                               << " reserves Seat" << req.seatNo << endl;
                LeaveCriticalSection(&writeMutex);

                processed++;
                ReleaseSemaphore(semServer, 1, NULL); // notify client
            }
            LeaveCriticalSection(&queueMutex);
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) { 
        cout << "Usage: flight_simulation.exe [seat_number]\n"; 
        return 0; 
    }

    clientSize = atoi(argv[1]);

    // Initialize critical sections for thread safety
    InitializeCriticalSection(&reservationMutex);
    InitializeCriticalSection(&writeMutex);
    InitializeCriticalSection(&queueMutex);

    semServer = CreateSemaphore(NULL, 0, LONG_MAX, NULL);

    // Each client gets a semaphore to signal server
    for (int i = 0; i < clientSize; i++)
        semClient.push_back(CreateSemaphore(NULL, 0, LONG_MAX, NULL));

    // Open file to write reservation logs
    outputTextFile.open("output.txt");
    outputTextFile << "Number of total seats: " << clientSize << endl;

    // Initialize available seats
    for (int i = 1; i <= clientSize; i++)
        availableSeats.insert(i);

    srand((unsigned int)time(NULL)); // seed RNG

    // Setup clients
    clients.resize(clientSize);
    for (int i = 0; i < clientSize; i++) {
        clients[i].clientId = i + 1;
        clients[i].reservedSeatNo = -1;
        clients[i].sleepTime = rand() % 151 + 50; // 50-200 ms
    }

    // Start client threads
    vector<HANDLE> clientThreads(clientSize);
    for (int i = 0; i < clientSize; i++)
        clientThreads[i] = CreateThread(NULL, 0, ClientFun, &clients[i], 0, NULL);

    // Start single server thread
    HANDLE serverThread = CreateThread(NULL, 0, ServerFun, NULL, 0, NULL);

    // Wait for clients and server to finish
    WaitForMultipleObjects(clientSize, clientThreads.data(), TRUE, INFINITE);
    WaitForSingleObject(serverThread, INFINITE);

    outputTextFile << "All seats are reserved." << endl;
    outputTextFile.close();

    // Cleanup
    DeleteCriticalSection(&reservationMutex);
    DeleteCriticalSection(&writeMutex);
    DeleteCriticalSection(&queueMutex);
    CloseHandle(semServer);
    for (int i = 0; i < clientSize; i++) CloseHandle(semClient[i]);
    for (int i = 0; i < clientSize; i++) CloseHandle(clientThreads[i]);
    CloseHandle(serverThread);

    return 0;
}
