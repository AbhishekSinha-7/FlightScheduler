# FlightScheduler ✈️

A **multithreaded flight seat reservation simulation system** written in C++.  
This project simulates multiple clients reserving seats on a flight, with a server handling reservation requests.  
It demonstrates **concurrency control**, **synchronization using semaphores**, and **mutual exclusion using critical sections**.

---

## 📌 Features
- Each client thread requests a random seat after a short randomized delay.
- Server threads process client requests and confirm reservations.
- Synchronization between client ↔ server using semaphores.
- Ensures no two clients reserve the same seat (critical section protected).
- Logs all reservations to an output file (`output.txt`).

---

## 📂 Project Structure
```abhisheksinha-7-flightscheduler/
├── README.md # Project documentation
├── flight_simulation.cpp # Main C++ simulation code
└── Makefile # Build instructions for MinGW (Windows)
```

---

## ⚙️ Requirements
- **OS**: Windows  
- **Compiler**: MinGW (g++)  
- **Libraries**: 
  - `windows.h` (Win32 API for threads & semaphores)
  - Standard C++ libraries (`<iostream>`, `<queue>`, `<set>`, etc.)

---

## 🛠️ Build Instructions

Open **Command Prompt** or **PowerShell**, navigate to the project directory, and run:

```
make
```
This will compile the code and generate:

```
flight_simulation.exe
```

To clean build files:

```
make clean
```


▶️ Run the Program
```
./flight_simulation.exe [seat_count]
```

Example:

```
./flight_simulation.exe 5
```
This simulates 5 clients trying to reserve seats.


📑 Output:\
The program generates an output.txt file containing the seat reservation log.

Example:\
```
Number of total seats: 5
Client1 reserves Seat 3
Client2 reserves Seat 1
Client3 reserves Seat 5
Client4 reserves Seat 2
Client5 reserves Seat 4
All seats are reserved.
```
## 🚀 Concepts Demonstrated
- Multithreading using Win32 threads (CreateThread, WaitForMultipleObjects).

- Inter-thread communication using queues (Pipe struct).

- Synchronization with semaphores (CreateSemaphore, ReleaseSemaphore, WaitForSingleObject).

- Critical sections to prevent race conditions:

# 📌 Future Improvements
- Port the project to POSIX threads (pthreads) for Linux compatibility.

- Add support for multiple flight instances.

- Implement cancellation & rebooking system.

## 👨‍💻 Author
**Abhishek Kumar Sinha**\
FlightScheduler – Operating Systems & Multithreading Simulation Project