# Copilot Instructions for guitarSynth.lv2

## Project Overview
- **guitarSynth.lv2** is an LV2 audio plugin that converts an input signal (e.g., guitar) into a synthesized sawtooth or square wave output.
- The main implementation is in `guitarSynth.c`, following the LV2 plugin API.
- Plugin metadata and port definitions are in `guitarSynth.ttl` and `manifest.ttl.in`.
- The build system uses Waf (`wscript`), with build artifacts and bundles placed in `build/lv2/guitarSynth.lv2/`.

## Key Files & Structure
- `guitarSynth.c`: Core DSP and LV2 plugin logic. Ports are defined by the `PortIndex` enum and mapped in `connect_port`.
- `guitarSynth.ttl`, `manifest.ttl.in`: LV2 RDF metadata, port configuration, and plugin registration.
- `wscript`: Waf build script. Handles configuration, build, and installation. Uses `autowaf` and the `lv2` Waf tool.
- `build/`: Output directory for build artifacts.

## Build & Install Workflow
- Configure: `./waf configure`
- Build: `./waf build`
- Install: `sudo ./waf install`
- If needed, manually move the bundle to your LV2 directory (e.g., `~/.lv2/`, `/usr/lib/lv2/`).
- Restart or refresh your LV2 host to load the plugin.

## Project Conventions
- All plugin code is in a single C file (`guitarSynth.c`).
- LV2 port indices and symbols must match between C and `.ttl` files.
- Use the provided Waf build system; do not use `make` or CMake.
- Metadata files are generated/copied into the build bundle by the Waf script.
- External dependency: LV2 SDK and math library (`libm`).

## Development Tips
- To add new controls or ports, update both `guitarSynth.c` (enum, connect_port) and `guitarSynth.ttl`.
- For debugging, add print statements in C and rebuild. LV2 hosts may buffer output; check logs after host restart.
- Use the `waflib/extras/lv2.py` and `autowaf.py` helpers for LV2-specific build logic.
- Example LV2 host directories: `~/.lv2/`, `/usr/lib/lv2/`, `/usr/local/lib/lv2/`.

## References
- See `README.md` for user-facing install/build instructions.
- See `wscript` for build logic and customizations.
- See `guitarSynth.ttl` and `manifest.ttl.in` for LV2 metadata structure.

---
If you add new features or ports, always update both the C and TTL files to keep them in sync. For questions about LV2 or Waf, refer to the official documentation or the `waflib/extras` scripts included in this repo.
