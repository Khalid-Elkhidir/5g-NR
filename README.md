# 5G NR Layer 2 Protocol Implementation

## Overview
This project implements a simulation of the 5G NR Layer 2 protocol stack. It integrates several sublayers including HARQ, MAC, RLC, and PDCP. The system simulates the complete transmission chain from the network layer (using a dummy IP packet generator) through PDCP, RLC, and MAC, and finally employs a loopback mechanism to simulate the PHY layer. In this implementation, PDCP provides header compression and ciphering, RLC handles segmentation/reassembly, and MAC manages logical channel mapping and scheduling. The system continuously generates dummy IP packets and processes them through the entire chain until interrupted.

## Project Structure
├── 5g ├── harq │   ├── harq.c │   └── harq.h ├── ipgen │   ├── ipgen.c │   └── ipgen.h ├── mac │   ├── mac.c │   ├── mac.h │   ├── loopback.c │   └── loopback.h ├── main.c ├── Makefile ├── pdcp │   ├── pdcp.c │   └── pdcp.h └── rlc ├── rlc.c └── rlc.h
**Explanation:**
- **harq/**: Contains the HARQ sublayer implementation.
- **ipgen/**: Contains the dummy IP packet generator that creates valid IPv4 packets with static source and destination IPs.
- **mac/**: Contains the MAC sublayer implementation, including logical channel mapping, multiplexing, and a loopback mechanism that simulates the PHY layer.
- **pdcp/**: Contains the PDCP sublayer implementation with header compression, ciphering/deciphering, and interfacing with the RLC sublayer.
- **rlc/**: Contains the RLC sublayer implementation with segmentation, reassembly, and interfacing with PDCP and MAC.
- **main.c**: The main driver that orchestrates the simulation and continuously processes packets through the stack.
- **Makefile**: Build configuration file for compiling the project.

## How to Run the Project
1. **Prerequisites:**
   - Linux operating system (e.g., Ubuntu 20.04 or similar)
   - GCC compiler (version 9 or later recommended)
   - Make utility

2. **Build the Project:**
   - Open a terminal in the project root directory.
   - Run the command:
     ```
     make
     ```
   - This will compile all source files and generate the executable (e.g., `5g_nr_sim`).

3. **Run the Project:**
   - In the terminal, execute:
     ```
     ./5g_nr_sim
     ```
   - The simulation will continuously generate dummy IP packets, process them through PDCP, RLC, and MAC sublayers, loop them back, and deliver them upward. Logs will be printed at each step to provide detailed information on packet sizes and processing stages.
   - To stop the simulation, press `Ctrl+C`.

## Environment
- **Operating System:** Linux (e.g., Ubuntu 20.04)
- **Compiler:** GCC (version 9+ recommended)
- **Dependencies:** Standard C libraries (stdlib, stdio, string, unistd, arpa/inet)

## Team Members
- **Name:** [Your Name Here]
  **Email:** [your.email@example.com]

- **Name:** [Team Member Name]
  **Email:** [team.member@example.com]

*(Add additional team members as needed.)*

## License
*(Optional: Add your project's license information here.)*

## Acknowledgements
*(Optional: Add acknowledgements here.)*
