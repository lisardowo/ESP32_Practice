# Artemisa Library/API Documentation

Artemisa is a Wi-Fi promiscuous sniffer and 802.11 frame parser intended for ESP32 (ESP-IDF via PlatformIO). The final objective of the device is to scan and display relevant security information of nearby networks for auditing; paired with their big brother(Hermes-Module HUB(currently under development T.T)) the device can display a small map of the networks it scans and throw sum small, basic attacks to prove founded vulnerabilityes. It is organized as small C modules that can be reused as a library and extended by final users.

## Index

1. Purpose and Architecture
2. Dependencies
3. Data Model
4. Public API (headers)
5. Implemented Functions
6. Functional Behavior Summary
7. Using Artemisa as a Library/API
8. Optimization -> FLAVORTOWN OPTIMIZATION QUEST :3
9. Current Known Gaps for API Consumers
10. Minimal Example

## Project Origin

Artemisa's library is customly developed from scratch for this project and adapted over time according to project needs.

For optimization and maintainability purposes, artemisa's library has been stripped down to the very minimum but a full version of an sniffer interpeter will continue in development at:

- https://github.com/lisardowo/EIII-808.11-Sniffer-interpreter

## 1. Purpose and Architecture

Main pipeline:

1. Initialize memory and Wi-Fi stack.
2. Enable promiscuous mode.
3. Receive raw 802.11 frames in callback.
4. Validate payload length.
5. Extract frame header fields and management tags.
6. Store selected network data in `identified_network`.

Modules:

- `setupSniffer.*`: Basic Setup stuff (runtime initialization, channel hopping, packet callback).
- `validate.*`: Utility functions to validate founded signals (payload and subtype validation).
- `extract.*`: logic for extracting relevant information (frame control and tag extraction).
- `addressing.*`: logic for addressing interpretation of IEE requests (interpretation based on ToDS/FromDS bits).
- `networkStruct.*`: logic of the structs that store network information (data model (`identified_network`) and field fill helpers).

## 2. Dependencies

### 2.1 External dependencies (ESP32 / ESP-IDF)

Used by `setupSniffer.*`:

- FreeRTOS: `freertos/FreeRTOS.h`, `freertos/task.h`, `freertos/queue.h`
- Wi-Fi and system: `esp_wifi.h`, `esp_event.h`, `esp_netif.h`, `esp_system.h`
- NVS: `nvs_flash.h`

These come from ESP-IDF and are available through PlatformIO when `framework = espidf`.

### 2.2 C standard library dependencies

- `stdio.h`
- `string.h`
- `stdint.h`
- `inttypes.h`
- `stdbool.h`
- `stdlib.h`

### 2.3 Internal module dependencies

- `setupSniffer.c` depends on: `validate.h`, `extract.h`
- `extract.c` depends on: `networkStruct.h`, `validate.h`, `addressing.h`
- `addressing.c` depends on: `extract.h`
- `networkStruct.c` depends on: `networkStruct.h`
- `validate.c` depends on: `validate.h`

## 3. Data Model

## `identified_network` (in `networkStruct.h`)

Fields:

- `mac[6]`
- `ssid[24]`
- `rssi`
- `channel`
- `lastSeen`
- `packetCount`
- `securityFlags` bitfield:
  - `wpsActive:1`
  - `authMode:3`
  - `pmfRequired:1`
  - `isRogue:1`
  - `reserved:2`

Notes:

- `ssidMaxSizeStruct` is currently 24 bytes (not full 32-byte SSID max of 802.11) to save up some memory.

## 4. Public API (headers)

This section lists all functions declared in headers (expected API surface).

### 4.1 `setupSniffer.h`

- `void payload_interpreter(unsigned char *payload, uint16_t payloadSize);`
  - Passes frame to extraction functions.
- `void memory_initializer();`
  - Initializes NVS and handles common NVS recovery path.
- `void set_promiscuous();`
  - Initializes Wi-Fi and enables promiscuous mode.
- `void channel_swapping();`
  - Declared with no args, but implementation uses task signature with `void *` parameter.
- `void sniffed_packets_handler();`
  - Declared with no args, but implementation uses ESP-IDF callback signature.
- `void sniffer_init();`
  - High-level startup function. Initializes stack and registers callback.

### 4.2 `validate.h`

- `bool validate_management_frame(int type);`
  - Returns `true` for management frames (`type == 0`).
- `bool validate_beacon_subtype(int subtype);`
  - Returns `true` when subtype is beacon (`8`).
- `bool is_valid_payload(int size);`
  - Returns `true` if payload length >= 24-byte (802.11 header baseline).

### 4.3 `extract.h`

- `void payload_header_extractor(unsigned char *payload, uint16_t payloadSize);`
  - Main frame header parsing entry point.
- `void extract_protocol(unsigned char *payload, uint_least8_t *flagsBoolean);`
  - Extracts protocol bits from Frame Control.
- `int extract_type(unsigned char *payload);`
  - Extracts frame type (management/control/data).
- `int extract_subtype(unsigned char *payload);`
  - Extracts frame subtype from Frame Control.
- `void extract_toDs(unsigned char *payload, uint_least8_t *flagsBoolean);`
- `void extract_fromDs(unsigned char *payload, uint_least8_t *flagsBoolean);`
- `void extract_retry(unsigned char *payload, uint_least8_t *flagsBoolean);`
- `void extract_powerManagement(unsigned char *payload, uint_least8_t *flagsBoolean);`
- `void extract_wep(unsigned char *payload, uint_least8_t *flagsBoolean);`
- `void extract_order(unsigned char *payload, uint_least8_t *flagsBoolean);`
  - These parse individual Frame Control flags and set bits in `flagsBoolean`.
- `void extract_addrs1(unsigned char *payload, const char *type);`
- `void extract_addrs2(unsigned char *payload, const char *type);`
- `void extract_addrs3(unsigned char *payload, const char *type);`
- `void extract_addrs4(unsigned char *payload, const char *type);`
  - Extract address fields from fixed header offsets.
- `void payload_data_walker(unsigned char *payload, uint16_t totalLenght, identified_network* network);`
  - Iterates management tags (TLV style) and updates a specific network instance.

### 4.4 `addressing.h`

- `void frame_type_interpreter(uint_least8_t *frameType, unsigned char *payload, uint16_t payloadSize);`
  - Dispatches behavior by management/control/data frame type.
- `void type_of_addressing(uint_least8_t booleanFlags, unsigned char *payload);`
  - Determines address interpretation from ToDS/FromDS bit combination.

### 4.5 `networkStruct.h`

- `void fill_mac(identified_network *identified_network, unsigned char *mac);`
- `void fill_ssid(identified_network *identified_network, unsigned char *networkName);`
- `void fill_rssi(identified_network *identified_network, int8_t *rssi);`
- `void fill_channel(identified_network *identified_network, uint8_t *channel);`
- `void fill_lastSeen(identified_network *identified_network, uint32_t *lastSeen);`
- `void fill_packetCount(identified_network *identified_network, uint16_t *packetCount);`
- `void fill_wpsActive(identified_network *identified_network, unsigned char *wpsActive);`
- `void fill_authMode(identified_network *identified_network, unsigned char *authMode);`
- `void fill_pmfRequired(identified_network *identified_network, unsigned char *pmfRequired);`
- `void fill_isRogue(identified_network *identified_network, unsigned char *isRogue);`
- `void fill_reserved(identified_network *identified_network, unsigned char *reserved);`

## 5. All Implemented Functions (including non-header/internal)

The following functions exist in `.c` files and are relevant for maintenance and extension.

### `setupSniffer.c`

- `void wifi_stack_init()`
  - Internal init guard (`static bool init`) and esp-netif setup.
- `void channel_swapping(void *parametersTopass)`
  - Infinite task loop hopping channels 1..11 every 3 seconds.
- `void memory_initializer()`
- `void set_promiscuous()`
- `void sniffed_packets_handler(void* buf, wifi_promiscuous_pkt_type_t type)`
  - Packet callback with filtering by management frame + beacon subtype.
  - Resolves source MAC, finds/creates network, updates RSSI/timestamp, and parses tags.
- `void payload_interpreter(unsigned char *payload, uint16_t payloadSize)`
- `void sniffer_init()`

### `extract.c`

- `void extract_protocol(...)`
- `int extract_type(...)`
- `int extract_subtype(...)`
- `void extract_toDs(...)`
- `void extract_fromDs(...)`
- `void extract_retry(...)`
- `void extract_powerManagement(...)`
- `void extract_wep(...)`
- `void extract_order(...)`
- `void extract_more_frag(...)` (implemented, not declared in `extract.h`)
- `void extract_more_data(...)` (implemented, not declared in `extract.h`)
- `void extract_addrs1(...)`
- `void extract_addrs2(...)`
- `void extract_addrs3(...)`
- `void extract_addrs4(...)`
- `void payload_header_extractor(...)`
- `void payload_data_walker(...)`

### `addressing.c`

- `void type_of_addressing(...)`
- `void frame_type_interpreter(...)`

### `validate.c`

- `bool is_valid_payload(int size)`
- `bool validate_management_frame(int type)`
- `bool validate_beacon_subtype(int subtype)`

### `networkStruct.c`

- `void fill_mac(...)`
- `void fill_ssid(...)`
- `void fill_rssi(...)`
- `void fill_channel(...)`
- `void fill_lastSeen(...)`
- `void fill_packetCount(...)`
- `void fill_wpsActive(...)`
- `void fill_authMode(...)`
- `void fill_pmfRequired(...)`
- `void fill_isRogue(...)`
- `void fill_reserved(...)`
- `void create_new_network(...)`
- `identified_network* find_network(...)`
- `void delete_old_networks()`
- `void delete_network(...)`
- `void update_network(...)`
- `void display_networks()`
- `int compare_network_rssi(...)`

## 6. Functional Behavior Summary

- Frame Control parsing is done byte-wise with masks and shifts.
- Address extraction uses fixed offsets from 802.11 header layout.
- Management tags are parsed using a TLV walker:
  - `tag_id = payload[position]`
  - `tag_length = payload[position + 1]`
  - `value = payload[position + 2 ... + tag_length]`
- The parser currently emphasizes beacon management frames.

## 7. Using Artemisa as a Library/API

### 7.1 Recommended integration entry point

Use `sniffer_init()` from your application startup.

Expected sequence inside:

1. NVS init (`memory_initializer`).
2. netif init (`wifi_stack_init`).
3. Promiscuous enable (`set_promiscuous`).
4. Channel hop task creation (`channel_swapping`).
5. Packet callback registration (`sniffed_packets_handler`).

### 7.2 Extending behavior

Common extension points:

- In `frame_type_interpreter`: add control/data frame handling.
- In `payload_data_walker`: decode more 802.11 tags.
- In `networkStruct.c`: implement currently stubbed `fill_*` functions.
- Replace `printf` debug flow with user callbacks (library-friendly output).

## 8. Optimization

Current prototype implements several optimization strategies:

1. Efficient algorithms
- Hash table + doubly linked list were implemented for network indexing and lifecycle handling.
- `find_network()` is hash-based for fast lookup by BSSID (O(1)).
- The doubly linked list (`head`/`tail` + `next`/`previous`) supports efficient recency updates and timeout-based deletion.
- `display_networks()` uses a temporary pointer buffer plus `qsort` instead of reordering linked list nodes directly.

2. Reduce memory usage
- `identified_network` keeps compact field sizes and bitfields in `securityFlags`.
- Parsing uses a single-pass TLV walk (`payload_data_walker`) with bounds checks, avoiding unnecessary extra buffers.

3. Minimize dependencies
- Core parser/data modules rely on C stdlib + ESP-IDF only.
- No external third-party parser framework is required.
- Library was manually developed from scratch to ensure maximizing the optimization

4. Lazy loading (applied as processing-on-demand)
- Management tag parsing is only triggered after frame-type/subtype validation in the sniff callback.
- New network nodes are created only when a BSSID is not already indexed.

5. Caching strategies
- At application level, the in-memory hash table acts as a short-lived network state cache.

Potential next optimization steps:

- Fix duplicate `esp_netif_init()` call in `wifi_stack_init()` and ensure event loop initialization flow is complete.
- Gate logs behind compile-time macros to reduce callback path overhead.
- Use tag length (`tagLenght`) as bounded copy length for SSID writes to prevent noisy strings and unnecessary copy work.
- Add a lightweight queue between promiscuous callback and heavy parsing if packet rate increases.

## 9. Current Known Gaps for API Consumers

- Some function prototypes in headers do not match implementation signatures (`channel_swapping`, `sniffed_packets_handler`).
- `extract_more_frag` and `extract_more_data` are implemented but not declared in `extract.h`.
- `ssidMaxSizeStruct` is currently 24 (still below full IEEE SSID max of 32 chars + null terminator).


## 10. Minimal Example

```c

#include "setupSniffer.h"

void app_main(void)
{
    sniffer_init();
}

```

This starts promiscuous sniffing and parsing with current defaults.
