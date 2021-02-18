# Changelog
All notable changes to dual-Mag-Control will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added
- README.md
- LICENSE.md, MIT
- Option to use SBE39 CTD instead of RBR CTD data
- SDLogger class to support logging data to SD card if inserted

### Changed
- MIN_FLASH_DURATION changed to 1 (us)
- PlatformIO COM port changed to COM8
- Allow flash durations >= MIN_FLASH_DURATION
- Chnaged the Time Event end condition to fix extra 1 minute bug

## [1.0.0] - 2020-12-10
### Added
- First stable release of Dual-Mag-Control as deployed with Dual-Mag-01 Camera System

