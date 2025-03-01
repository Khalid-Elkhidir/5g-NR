# 5G NR Layer 2 Protocol Implementation

## Overview
This project implements a simulation of the 5G NR Layer 2 protocol stack. It integrates several sublayers including HARQ, MAC, RLC, and PDCP. The system simulates the complete transmission chain from the network layer (using a dummy IP packet generator) through PDCP, RLC, and MAC, and finally employs a loopback mechanism to simulate the PHY layer.

### Key Features
- Complete Layer 2 protocol stack simulation
- PDCP sublayer with header compression and ciphering
- RLC sublayer with segmentation/reassembly in multiple modes (TM/UM)
- MAC sublayer with logical channel mapping and scheduling
- HARQ process management for reliable transmission
- PHY layer simulation through loopback mechanism
- Detailed logging at each processing stage

## Project Structure
```
.
├── harq/               # HARQ (Hybrid ARQ) implementation
│   ├── harq.c         # HARQ process management
│   └── harq.h         # HARQ interfaces and structures
├── ipgen/             # IP packet generation
│   ├── ipgen.c        # Dummy packet generator implementation
│   └── ipgen.h        # IPv4 header structures and interfaces
├── mac/               # MAC sublayer implementation
│   ├── mac.c          # MAC procedures and channel management
│   └── mac.h          # MAC interfaces and channel structures
├── pdcp/              # PDCP sublayer implementation
│   ├── pdcp.c         # PDCP entity and data handling
│   └── pdcp.h         # PDCP interfaces and security features
├── rlc/               # RLC sublayer implementation
│   ├── rlc.c          # RLC modes and data handling
│   └── rlc.h          # RLC interfaces and structures
├── loopback/          # PHY layer simulation
│   ├── loopback.c     # Loopback mechanism implementation
│   └── loopback.h     # Loopback interfaces
├── main.c             # Main simulation driver
├── Makefile           # Build configuration
└── README.md          # Project documentation
```

## Implementation Details

### PDCP Sublayer
- Header compression for efficient radio transmission
- Security through ciphering/deciphering operations
- Sequence number management
- SDU delivery to upper layers

### RLC Sublayer
- Multiple operation modes:
  - Transparent Mode (TM): Direct forwarding
  - Unacknowledged Mode (UM): Segmentation without retransmission
- Segment size configuration
- Reassembly of segmented PDUs
- In-sequence delivery

### MAC Sublayer
- Logical channel management
- Multiplexing/demultiplexing of data flows
- Buffer status reporting
- Scheduling request handling
- HARQ process management

### HARQ Implementation
- Support for multiple HARQ processes
- Retransmission handling
- Soft combining of received data
- ACK/NACK processing

### IP Packet Generation
- IPv4 packet creation with valid headers
- Checksum calculation
- Configurable source and destination addresses
- Test payload generation

## Building and Running

### Prerequisites
- Linux operating system (Ubuntu 20.04 or similar)
- GCC compiler (version 9 or later recommended)
- Make utility

### Build Instructions
```bash
# Clone the repository
git clone [repository-url]
cd 5g-nr

# Build the project
make

# Run the simulation
./5g
```

### Runtime Behavior
The simulation will:
1. Generate dummy IP packets
2. Process them through the PDCP layer
3. Handle segmentation in RLC
4. Process through MAC layer
5. Simulate transmission via loopback
6. Process received data back up through the stack

To stop the simulation, press `Ctrl+C`.

## Output and Logging
The simulation provides detailed logging at each stage:
- Packet generation details
- PDCP processing information
- RLC segmentation/reassembly status
- MAC layer operations
- HARQ process status
- Transmission confirmations

## Future Improvements
- Add support for RLC Acknowledged Mode (AM)
- Implement more sophisticated scheduling algorithms
- Add QoS support in MAC layer
- Enhance security features in PDCP
- Add configuration file support
- Implement more realistic PHY layer simulation

## Authors
- Mustafa Kamil Elsheikh Ibrahim - 174098
- Ahmed Osman Ahmed Ibrahim - 174013
- Khalid Fadhl Elmargi Bakhit Elkhidir - 174030
- Omer Abdelaziz Ibrahim Mohammed - 174058

## License
- MIT License
- Copyright (c) 2024    

## Acknowledgments
- 3GPP specifications and standards v2
- Open source community
