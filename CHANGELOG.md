# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [unreleased]

### Added
- Added bulk data collection example
- Added nrf52832 Fota bootloader build
- Added flash write example
- Added changelog file
- Added broadcast example (before changelog)
- Added the format-configuration repo from GitHub for clang-format config

### Changed
- Use new nrfutil version
- Update fota_sender_with_driver example to use new mira_fota_set_driver API
- Update FOTA examples to use Mira CRC API
- Update fota_receiver_with_bootloader to use new mira_fota_header_t struct
