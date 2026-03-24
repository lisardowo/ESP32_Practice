[1.0.0] - 2026-03-24
## Added
- Promiscuous sniffer initialization flow (memory init, Wi-Fi init, callback registration).
- Channel hopping task for periodic channel scanning.
- Beacon-focused parsing path with management/type validation.
- Address resolution using DS direction bits.
- Network lifecycle management:
- create/find/update/delete operations
- timeout cleanup of stale entries
- Network visualization sorted by RSSI.
- Hash table + doubly linked list integration for efficient indexing and ordering.
## Changed
- RSSI handling updated to signed semantics (int8_t) across network update flow.
- SSID copy logic improved with bounded copy and explicit zeroing.
- Parsing flow now updates a concrete identified_network instance.
## Fixed
- Multiple-definition linker issues for global network state.
- Pointer type mismatches in parsing/update paths.
- Several signature inconsistencies between declarations and usage.
- Known Issues
- Some noisy debug/prototype logs still present.
- Potential missed detections for APs outside current channel configuration.
- Header/API cleanup still pending for a few legacy declarations.