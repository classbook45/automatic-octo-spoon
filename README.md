# PAY-LAUNCH

A controller-first PS4 homebrew dashboard built with the OpenOrbis toolchain.

## Features
- Dark XMB-inspired interface
- Overview, payload catalogue, console diagnostics, activity log, and settings
- D-pad / left-stick navigation, X to select, O to return
- GitHub Actions workflow builds a PS4 PKG and uploads it as an artifact

## Scope
This first version is a catalogue and dashboard. Entries are informational only: no exploit code or executable payloads are bundled, and the app does not run arbitrary payloads or modify firmware or console settings. Run / Install is intentionally unavailable in this build.

## Build
Push to main or use Actions -> Build PAY-LAUNCH PS4 PKG -> Run workflow. The workflow uses a pinned official OpenOrbis toolchain container image and uploads PAY-LAUNCH-PS4-PKG.

## Package identity
- Title: PAY-LAUNCH
- Title ID: PAYL00001
- Version: 01.00
