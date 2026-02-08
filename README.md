# SysPulse

System resource monitor using Qt

<div align="center">

![SysPulse Logo](https://img.shields.io/badge/SysPulse-v2.0-blue.svg)
![Qt](https://img.shields.io/badge/Qt-5.15-green.svg)
![Platform](https://img.shields.io/badge/platform-Linux-orange.svg)
![License](https://img.shields.io/badge/license-MIT-blue.svg)

**A powerful, real-time system monitoring application for Linux with GUI and Terminal interfaces**

[Features](#features) • [Installation](#installation) • [Usage](#usage) • [Terminal Mode](#terminal-mode)

</div>

---

## Features

### Dual Interface Modes

- **GUI Mode** - Beautiful dark-themed Qt5 interface
- **Terminal Mode** - Colorful command-line interface for SSH/headless systems

### Overview Dashboard (GUI)

- **Real-time CPU Monitoring** - Track overall CPU usage with detailed core information
- **Memory Usage Tracking** - Monitor RAM consumption with detailed MB statistics
- **Disk Space Monitoring** - Keep track of filesystem usage with GB details
- **System Uptime** - Display system uptime in days, hours, and minutes
- **Process Counter** - Total running processes count
- **CPU Temperature** - Real-time temperature monitoring (if available)

### Process Manager (GUI)

- **Process List** - View all running processes with detailed information
- **Sortable Columns** - Sort by PID, name, CPU%, memory%, or user
- **Kill Process** - Terminate processes directly from the interface
- **Auto-Refresh** - Optional automatic process list updates
- **User Information** - See which user owns each process

### Network Monitor

- **Download/Upload Speed** - Real-time network throughput
- **Total Traffic** - Cumulative network statistics
- **Visual Activity Bar** - Network activity indicator

### Advanced Charts (GUI)

- **CPU History Chart** - 60-second rolling CPU usage graph
- **Memory History Chart** - 60-second rolling memory usage graph
- **Network Traffic Chart** - Dual-line chart for upload/download
- **Dark Theme** - Beautiful dark mode for all charts

### Terminal Mode Features

- **Colorful Interface** - ANSI color-coded statistics
- **ASCII Progress Bars** - Visual representation in terminal
- **Live Updates** - Auto-refresh every 2 seconds
- **Network Stats** - Real-time speed and totals
- **SSH-Friendly** - Perfect for remote monitoring

### Additional Features

- **Data Export** - Export monitoring data to CSV format
- **Tab-based Interface** - Organized, clean UI with multiple tabs
- **Configurable Updates** - Adjustable refresh intervals
- **Dark Mode** - Professional dark theme throughout
- **Dual Modes** - GUI or Terminal based on preference

## Requirements

### System Requirements

- **OS**: Ubuntu 20.04+ (or any Linux with `/proc` filesystem)
- **Qt**: Qt5.15 or later (for GUI mode only)
- **Compiler**: GCC 7+ or Clang 5+ (C++17 support)
- **CMake**: 3.16 or later

### Dependencies

Install all required packages:

```bash
sudo apt update
sudo apt install qtbase5-dev libqt5charts5-dev build-essential cmake
```

## Installation

### Build from Source

1. **Clone the repository:**

```bash
git clone https://github.com/mosioc/sys-pulse
cd SysPulse
```

1. **Create build directory:**

```bash
mkdir build
cd build
```

1. **Configure with CMake:**

```bash
cmake ..
```

1. **Compile:**

```bash
make -j$(nproc)
```

1. **Run:**

```bash
# GUI mode (default)
./bin/SysPulse

# Terminal mode
./bin/SysPulse --terminal
```

## Usage

### GUI Mode (Default)

Run the graphical interface:

```bash
./bin/SysPulse
```

**If you encounter library conflicts:**

```bash
LD_PRELOAD=/lib/x86_64-linux-gnu/libpthread.so.0 ./bin/SysPulse
```

### Terminal Mode

Perfect for SSH sessions, headless servers, or if you prefer the command line:

```bash
./bin/SysPulse --terminal
# or
./bin/SysPulse -t

# or, in case of problem:
LD_PRELOAD=/lib/x86_64-linux-gnu/libpthread.so.0 ./bin/SysPulse --terminal

LD_PRELOAD=/lib/x86_64-linux-gnu/libpthread.so.0 ./bin/SysPulse -t
```

**Terminal mode features:**

- Colorful ASCII interface with progress bars
- Auto-refreshes every 2 seconds
- Shows CPU, Memory, Disk, Network stats
- System uptime and process count
- CPU temperature (if available)
- Works over SSH
- No GUI dependencies needed when running in terminal mode

**Terminal Mode Screenshot:**

```
╔════════════════════════════════════════════════════════════════════╗
║                    SYSPULSE SYSTEM MONITOR                         ║
╚════════════════════════════════════════════════════════════════════╝

┌─ System Information ────────────────────────────────────────────────
│ Uptime: 5d 12h 34m  │  Processes: 342  │  CPU Temp: 45.2°C
└─────────────────────────────────────────────────────────────────────

┌─ CPU Usage ─────────────────────────────────────────────────────────
│ [████████████████████░░░░░░░░░░░░░░░░░░░░░░] 42%
└─────────────────────────────────────────────────────────────────────

┌─ Memory Usage ──────────────────────────────────────────────────────
│ [██████████████████████████░░░░░░░░░░░░░░░░] 65%
└─────────────────────────────────────────────────────────────────────

┌─ Network Statistics ────────────────────────────────────────────────
│ Download Speed: 1.24 MB/s  │  Upload Speed: 256.43 KB/s
│ Total Downloaded: 15.67 GB  │  Total Uploaded: 3.21 GB
└─────────────────────────────────────────────────────────────────────

Press Ctrl+C to exit | Refreshing every 2 seconds...
```

### Creating Aliases

Add to your `~/.bashrc` or `~/.zshrc`:

```bash
# GUI mode
alias syspulse='/path/to/SysPulse/build/bin/SysPulse'

# Terminal mode
alias sysmon='/path/to/SysPulse/build/bin/SysPulse --terminal'
alias htop-alt='syspulse --terminal'
```

Then use:

```bash
syspulse      # Launch GUI
sysmon        # Launch terminal mode
```

## Technical Details

### System Information Sources

| Metric | Source | Method |
|--------|--------|--------|
| CPU Usage | `/proc/stat` | Calculate delta between idle and total time |
| Memory | `/proc/meminfo` | Read MemTotal and MemAvailable |
| Disk | `statvfs()` syscall | Query filesystem statistics |
| Processes | `/proc/loadavg` | Read total process count |
| Network | `/proc/net/dev` | Parse interface statistics |
| Temperature | `/sys/class/thermal/` | Read thermal zone data |
| Uptime | `/proc/uptime` | Parse system uptime |

### Command Line Options

```bash
SysPulse [options]

Options:
  -h, --help         Display help information
  -v, --version      Display version information
  -t, --terminal     Run in terminal mode (text-based interface)
```

## Dark Mode

SysPulse features a beautiful dark theme by default:

- Dark gray backgrounds (RGB: 53, 53, 53)
- Color-coded charts (Blue CPU, Green Memory, Red/Green Network)
- Styled progress bars
- Professional appearance
- Easy on the eyes for long monitoring sessions

## Troubleshooting

### Terminal Mode Issues

**Colors not showing:**

```bash
# Make sure your terminal supports ANSI colors
echo $TERM  # Should show something like xterm-256color
```

**Terminal mode not working:**

```bash
# Terminal mode doesn't require Qt, only C++ standard library
# If it fails, check your compiler supports C++17
g++ --version  # Should be 7.0 or higher
```

### GUI Mode Issues

**Build Fails: Qt5Charts not found:**

```bash
sudo apt install libqt5charts5-dev
rm -rf build/*
cmake .. && make
```

**Snap library conflicts:**

```bash
LD_PRELOAD=/lib/x86_64-linux-gnu/libpthread.so.0 ./bin/SysPulse
```
