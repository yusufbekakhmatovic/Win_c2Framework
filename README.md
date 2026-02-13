<div align="center">

# ⚔️ VOID C2 — Advanced Command & Control Framework

<img src="https://img.shields.io/badge/Version-3.0-00ff9d?style=for-the-badge&logo=hackthebox&logoColor=white"/>
<img src="https://img.shields.io/badge/Platform-Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white"/>
<img src="https://img.shields.io/badge/Agent-C++-00599C?style=for-the-badge&logo=cplusplus&logoColor=white"/>
<img src="https://img.shields.io/badge/Server-Python-3776AB?style=for-the-badge&logo=python&logoColor=white"/>
<img src="https://img.shields.io/badge/Developer-Mr.Yusuf-ff00ff?style=for-the-badge&logo=github&logoColor=white"/>
<img src="https://img.shields.io/badge/Telegram-@Yusufbek_Akhmadovic-26A5E4?style=for-the-badge&logo=telegram&logoColor=white"/>

<br/>

```
██╗   ██╗ ██████╗ ██╗██████╗      ██████╗██████╗ 
██║   ██║██╔═══██╗██║██╔══██╗    ██╔════╝╚════██╗
██║   ██║██║   ██║██║██║  ██║    ██║      █████╔╝
╚██╗ ██╔╝██║   ██║██║██║  ██║    ██║     ██╔═══╝ 
 ╚████╔╝ ╚██████╔╝██║██████╔╝    ╚██████╗███████╗
  ╚═══╝   ╚═════╝ ╚═╝╚═════╝      ╚═════╝╚══════╝
```

**A cutting-edge Command & Control framework featuring military-grade evasion techniques, encrypted communications, and a sleek real-time web interface.**

---

[Features](#-core-capabilities) · [Architecture](#-system-architecture) · [Installation](#-installation-guide) · [Usage](#-operational-guide) · [Technical Insights](#-technical-deep-dive) · [Disclaimer](#%EF%B8%8F-legal-disclaimer)

</div>

---

## 🎯 Project Overview

**VOID C2** is an advanced offensive security framework engineered by **Mr.Yusuf** for professional red team operations, penetration testing, and security research. It combines a sophisticated C++ implant with a powerful Python command server, all managed through an intuitive web-based dashboard.

> ⚠️ **STRICTLY FOR AUTHORIZED TESTING & EDUCATION** — This framework is designed exclusively for controlled environments, security competitions, and legitimate penetration testing engagements.

---

## 🚀 Core Capabilities

### 🔹 Agent — Advanced C++ Implant
| Category | Feature | Technical Details |
|:---------|:--------|:-----------------|
| 🛡️ **Evasion** | AMSI Neutralization | Runtime patching of `AmsiScanBuffer` function |
| | ETW Suppression | Disables Windows Event Tracing telemetry |
| | Ntdll Unhooking | Restores hooked system libraries from clean disk copy |
| | Sandbox Detection | Multi-layer VM/sandbox environment detection |
| 💻 **Execution** | Interactive Shell | Persistent command shell with working directory tracking |
| | PowerShell Integration | Executes PS commands with `-ExecutionPolicy Bypass` |
| | Process Hollowing | Advanced process injection technique |
| 🔒 **Communication** | Dual Encryption | XOR cipher + Base64 encoding layer |
| | Protocol Framing | Newline-delimited message transmission over TCP |
| 🔄 **Persistence** | Registry Injection | Auto-start via Windows Registry Run key |
| | Task Scheduling | Scheduled task-based persistence option |
| 🥷 **Stealth** | Pure Win32 API | Zero managed code dependencies |
| | Hidden Execution | Invisible window creation and process spawning |
| | String Obfuscation | Runtime string deobfuscation |

### 🔹 Server — Python Control Center
| Feature | Description |
|:--------|:------------|
| 🌐 **Web Dashboard** | Modern real-time interface with Matrix aesthetics |
| 👥 **Multi-Session** | Concurrent management of unlimited agent connections |
| ⌨️ **Interactive Terminal** | Browser-based shell with command history navigation |
| 📡 **WebSocket Protocol** | Real-time bidirectional communication via Socket.IO |
| 🌏 **Encoding Support** | Automatic CP866/UTF-8 detection for localized output |
| 📂 **State Management** | Per-agent session tracking and directory synchronization |
| 🔐 **Secure Transport** | Encrypted command and control channel |

---

## 🏗️ System Architecture

```
┌──────────────────────────────────────────────────────────────────┐
│                       VOID C2 v3.0                               │
├──────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌──────────┐    XOR+B64/TCP    ┌──────────────┐   Socket.IO    │
│  │  AGENT   │ ◄═══════════════► │  C2 SERVER   │ ◄════════════► │  🖥️ WEB UI
│  │  (C++)   │    Port 2727      │  (Python)    │   Port 3434    │  (Browser)
│  └──────────┘                   └──────────────┘                │
│       │                              │                           │
│  ┌────┴────┐                    ┌────┴────┐                     │
│  │ AMSI    │                    │ Flask   │                     │
│  │ ETW     │                    │ SocketIO│                     │
│  │ Unhook  │                    │ Thread  │                     │
│  │ Sandbox │                    │ Manager │                     │
│  │ Hollow  │                    │ Crypto  │                     │
│  └─────────┘                    └─────────┘                     │
└──────────────────────────────────────────────────────────────────┘
```

---

## 📁 Repository Structure

```
Win_c2Framework/
├── 📄 c2_server.py              # Primary C2 Server (Flask + SocketIO)
├── 📄 quick_setup.sh            # Automated deployment script
├── 📄 build_agent.sh            # Cross-compilation build script
├── 📂 src/
│   ├── main.cpp                 # Agent entry point & core logic
│   ├── evasion/
│   │   ├── etw_bypass.cpp       # Event Tracing suppression
│   │   ├── sandbox_detect.cpp   # Environment fingerprinting
│   │   └── unhook.cpp           # DLL restoration routines
│   ├── execution/
│   │   ├── process_hollow.cpp   # Process injection engine
│   │   └── shell.cpp            # Command execution module
│   ├── network/
│   │   ├── connection.cpp       # C2 communication handler
│   │   └── encryption.cpp       # Cryptographic protocol
│   ├── persistence/
│   │   ├── registry.cpp         # Registry-based persistence
│   │   └── schtasks.cpp         # Scheduled task creation
│   └── utils/
│       ├── helpers.cpp          # Common utility functions
│       └── obfuscation.cpp      # String protection
├── 📂 include/
│   └── common.h                 # Global definitions & constants
├── 📂 templates/
│   └── index.html               # VOID C2 Web Interface
└── 📄 README.md                 # This file
```

---

## ⚡ Installation Guide

### System Requirements
- **Server**: Linux/macOS with Python 3.8+, MinGW-w64 cross-compiler
- **Target**: Windows 7/8/10/11 (x86/x64)

### One-Click Deployment
```bash
# Clone the repository
git clone https://github.com/yusufbekakhmatovic/Win_c2Framework.git
cd Win_c2Framework

# Execute automated setup
chmod +x quick_setup.sh
./quick_setup.sh
```

### Configuration Parameters
| Variable | Default | Purpose |
|:---------|:--------|:--------|
| `C2_IP` | `127.0.0.1` | C2 server listening address |
| `LPORT` | `2727` | Agent callback port |
| `LPORT_WEB` | `3434` | Web dashboard port |

### Manual Build (Optional)
```bash
# Compile agent manually
chmod +x build_agent.sh
./build_agent.sh

# Start C2 server
python3 c2_server.py
```

---

## 🎮 Operational Guide

### Step 1 — Initialize Server
```bash
python3 c2_server.py
```
Output:
```
[*] VOID C2 Server v3.0
[*] Listening on 0.0.0.0:2727
[*] Web interface: http://localhost:3434
```

### Step 2 — Deploy Agent
Transfer the compiled `.exe` to the target Windows system and execute. The agent will establish a covert connection to your C2 server.

### Step 3 — Command & Control Operations
Access the web dashboard and interact via the integrated terminal:

#### Available Commands
| Command | Function | Example |
|:--------|:---------|:--------|
| `sysinfo` | System reconnaissance | Hostname, OS, CPU, RAM, processes |
| `persist` | Enable persistence | Registry Run key installation |
| `ps <command>` | PowerShell execution | `ps Get-Process` |
| `cd <directory>` | Change directory | `cd C:\Windows\System32` |
| `dir` / `ls` | List directory | Current working directory contents |
| `download <file>` | File exfiltration | Transfer files from target |
| `upload <file>` | File deployment | Transfer files to target |
| `whoami` | User context | Current user privileges |
| `screenshot` | Screen capture | Capture desktop screenshot |
| `keylog start/stop` | Keylogger control | Credential harvesting |
| `help` | Command reference | Display all commands |
| `clear` | Terminal clear | Clear command output |
| `history` | Command history | View executed commands |
| `exit` | Terminate session | Close agent connection |

---

## 🔬 Technical Deep Dive

### 🔐 Encryption Protocol
```
Payload Processing Pipeline:
┌─────────────┐     ┌──────────┐     ┌──────────┐     ┌─────────┐
│  Plaintext  │ ──► │ XOR(0x42)│ ──► │ Base64   │ ──► │ TCP+'\n'│
└─────────────┘     └──────────┘     └──────────┘     └─────────┘
```

**Encryption Details:**
- XOR cipher with key `0x42` (customizable)
- Base64 encoding for transport safety
- Newline delimiter for message framing

### 🛡️ AMSI Bypass Implementation
Runtime memory patching technique:
```cpp
// Target: AmsiScanBuffer function
BYTE amsi_patch[] = { 
    0xB8, 0x57, 0x00, 0x07, 0x80,  // mov eax, 0x80070057
    0xC3                            // ret
};
// Result: Returns E_INVALIDARG, bypassing content inspection
```

### 🕵️ Sandbox Detection Logic
Multi-layer environment fingerprinting:

**Hardware Indicators:**
- CPU core count < 2
- Physical RAM < 4GB
- Disk storage < 60GB

**Behavioral Analysis:**
- Mouse movement patterns
- Execution timing analysis
- User interaction detection

**Process Signatures:**
```cpp
const char* vm_processes[] = {
    "vmtoolsd.exe", "VBoxService.exe", "vmsrvc.exe",
    "VGAuthService.exe", "vmware-tray.exe"
};
```

### 🔄 Persistence Implementation
```cpp
// Registry persistence mechanism
HKEY hKey;
RegOpenKeyExA(HKEY_CURRENT_USER, 
    "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
    0, KEY_SET_VALUE, &hKey);
    
RegSetValueExA(hKey, "WindowsSecurityUpdate", 0, REG_SZ,
    (BYTE*)executable_path, strlen(executable_path));
```

### 💉 Process Hollowing Technique
Advanced code injection workflow:
1. Create target process in suspended state
2. Unmap original executable from memory
3. Allocate new memory section
4. Write malicious payload
5. Update entry point and resume execution

---

## 🎓 Educational Applications

This framework demonstrates advanced concepts in:
- 🧠 **Windows Internals** — Low-level system programming and API manipulation
- 🔐 **Cryptography** — Custom protocol design and implementation
- 🛡️ **Defense Evasion** — Understanding modern security product bypass techniques
- 🌐 **Full-Stack Development** — Multi-language integration (C++ + Python + JavaScript)
- 🔬 **Memory Forensics** — Runtime code injection and process manipulation
- 🎯 **Offensive Security** — Red team tactics, techniques, and procedures (TTPs)

---

## 🛡️ Detection & Mitigation

### MITRE ATT&CK Mapping
| Technique | ID | Mitigation |
|:----------|:---|:-----------|
| AMSI Bypass | T1562.001 | Enable tamper protection, deploy EDR |
| ETW Evasion | T1562.001 | Kernel-level ETW consumers |
| Registry Persistence | T1547.001 | Monitor `Run` key modifications |
| Process Injection | T1055 | Memory integrity scanning |
| Command & Control | T1071.001 | Network segmentation, TLS inspection |

### Defense Recommendations
✅ **Network Level:**
- Deploy intrusion detection systems (IDS/IPS)
- Implement egress filtering
- Monitor for unusual outbound connections

✅ **Endpoint Level:**
- Enable Windows Defender Attack Surface Reduction
- Deploy Endpoint Detection & Response (EDR) solutions
- Regular integrity verification of system DLLs

✅ **Operational Level:**
- Security awareness training
- Principle of least privilege
- Regular security audits

---

## ⚠️ Legal Disclaimer

> **AUTHORIZED SECURITY TESTING & EDUCATION ONLY**

### ✅ Permitted Use Cases:
- Authorized penetration testing with written consent
- Personal laboratory environments and virtual machines
- Academic research and Capture The Flag (CTF) competitions
- Red team / Blue team training exercises
- Security product effectiveness testing

### ❌ Strictly Prohibited:
- Unauthorized access to any computer system
- Malicious activities or denial of service attacks
- Data theft, espionage, or surveillance
- Distribution of malware
- Any activity violating local, national, or international laws

**IMPORTANT:** The developer (**Mr.Yusuf**) assumes **NO LIABILITY** for misuse of this software. Users are **solely responsible** for ensuring compliance with all applicable laws and regulations. By using this framework, you agree to use it ethically and legally.

---

## 📚 References & Resources

- [MITRE ATT&CK Framework](https://attack.mitre.org/)
- [Windows API Documentation](https://docs.microsoft.com/en-us/windows/win32/api/)
- [OWASP Testing Guide](https://owasp.org/www-project-web-security-testing-guide/)
- [Red Team Field Manual](https://www.amazon.com/Rtfm-Red-Team-Field-Manual/dp/1494295504)

---

## 🤝 Contributing

Contributions are welcome! Please follow these guidelines:
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add AmazingFeature'`)
4. Push to branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

---

## 📊 Project Statistics

![GitHub stars](https://img.shields.io/github/stars/yusufbekakhmatovic/Win_c2Framework?style=social)
![GitHub forks](https://img.shields.io/github/forks/yusufbekakhmatovic/Win_c2Framework?style=social)
![GitHub watchers](https://img.shields.io/github/watchers/yusufbekakhmatovic/Win_c2Framework?style=social)

---

<div align="center">

### 👤 About the Developer

**Mr.Yusuf**  
*Offensive Security Researcher & Red Team Operator*

[![Telegram](https://img.shields.io/badge/Telegram-@Yusufbek_Akhmadovic-26A5E4?style=for-the-badge&logo=telegram)](https://t.me/Yusufbek_Akhmadovic)
[![GitHub](https://img.shields.io/badge/GitHub-yusufbekakhmatovic-181717?style=for-the-badge&logo=github)](https://github.com/yusufbekakhmatovic)

---

### 💖 Support the Project

If you find this project valuable, consider:
- ⭐ Starring the repository
- 🍴 Forking for your own experiments
- 📢 Sharing with the security community
- 💬 Providing feedback and suggestions

---

⚠️ *"With great power comes great responsibility. Use this knowledge to defend, not to attack."*

**© 2026 Mr.Yusuf — VOID C2 Framework Project**

*Licensed under MIT License — See LICENSE file for details*

</div>
