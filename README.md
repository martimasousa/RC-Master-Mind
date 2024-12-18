
---

# RC-Master-Mind
---

## Overview  
This project implements a client-server game application, where players interact with a server to play a game through network communication. The project is designed using modular code architecture, ensuring maintainability and separation of concerns between different functionalities.

### Features:
- A **server** to manage the game logic and communication.
- A **client** that allows players to interact with the game.
- Pre-existing directories for **game and score management**.
- Modularized directories for better organization and extensibility.
- Shared utilities and constants to simplify integration between client and server components.

---

## Project Structure

The project consists of the following main directories:  

### **`src/`**  
This folder contains the **main application files**:
- `server`: The server application responsible for handling game sessions, player communication, and enforcing game rules.
- `player`: The client application that allows a player to connect to the server and interact with the game.

### **`server_core/`**  
This folder includes all **server-related logic** and the core mechanics of the game, such as:  
- Game state management.
- Trial validation.
- Endgame logic.

### **`client_core/`**  
This folder contains all the **player-side logic**, including:  
- Input processing.
- Request handling.
- Response interpretation.

### **`include/`**  
This folder holds **shared constants and utility functions** that are used by both the client and server components, ensuring code reusability and consistency. Examples include:
- Common constants for configuration.
- Helper functions for string manipulation, file handling, or communication protocols.

### **`GAMES/`**  
This directory is pre-created and used to store **game-related data**, such as ongoing and completed game logs.

### **`SCORES/`**  
This directory stores the **scoreboard data**, maintaining records of players' game statistics.

### **`reports/`**  
This folder contains the **results of the scripts provided by the course**, which are used to analyze and evaluate the program's execution.

### Dynamic Folder: **`CLIENT_CACHE/`**  
During program execution, a folder called `CLIENT_CACHE` will be **dynamically created** to store the files transferred from the server to the client.  

---

## How to Run  
1. Compile the project using the provided `Makefile` (or follow manual build instructions if necessary).  
2. Run the **server**:
   ```bash
   ./server.o
   ```
3. Run the **client** (player):
   ```bash
   ./player.o
   ```

Ensure that both the client and server are running in the appropriate network environment to enable communication.  

---