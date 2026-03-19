# OPC UA TSN Configuration Framework

TPConf (TSN PubSub Configuration) is a tool-supported framework that maps traffic specifications to TSN and PubSub configurations that meet the specified timing constraints. TPConf also generates ready-to-compile publisher code for the open-source OPC UA PubSub implementation based on open62541.

---

## Components

### 1. PubSubGenerator
- Reads `input.ini`
- Determines:
  - Industrial Traffic Class and Specifications
  - TSN Class (ST / AVB / BE)
  - PubSub configuration
- Outputs:
  - `reco.ini` (recommended configuration)

### 2. CodeGenerator
- Reads `reco.ini`
- Generates:
  - `publisher_generated.c` (open62541 publisher)

---

## Build

```bash
gcc -std=c11 -Wall -Wextra -O2 PubSubGenerator.c -o PubSubGenerator
gcc -std=c11 -Wall -Wextra -O2 CodeGenerator.c -o CodeGenerator
```

## Run

```
./PubSubGenerator
./CodeGenerator reco.ini publisher_generated.c
```
