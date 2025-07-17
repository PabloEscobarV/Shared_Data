#!/usr/bin/env python3

import subprocess
import threading
import time
import json
import os
import signal
from collections import defaultdict
import queue
import re
import glob

class TestOrchestrator:
    def __init__(self):
        self.processes = {}
        self.process_data = defaultdict(dict)
        self.output_queues = {}
        self.running = True
        
    def compile_test(self):
        """Compile the simple test program"""
        print("Compiling simple test program...")
        
        # Find all cpp files in src directory
        src_files = glob.glob("src/*.cpp")
        if not src_files:
            print("Warning: No .cpp files found in src/ directory")
            src_files = []
        
        # Build the compilation command for simple_test
        # Add -I flag to include headers directory
        compile_cmd = ["g++", "-I.", "simple_test.cpp"] + src_files + ["-o", "simple_test", "-pthread"]
        
        print(f"Running: {' '.join(compile_cmd)}")
        
        result = subprocess.run(compile_cmd, capture_output=True, text=True)
        
        if result.returncode != 0:
            print(f"Compilation failed:")
            print(f"stdout: {result.stdout}")
            print(f"stderr: {result.stderr}")
            return False
            
        print("Compilation successful!")
        return True
    
    def launch_test_process(self, process_id, iterator_start, param_kef):
        """Launch a single test process with data collection"""
        try:
            # Launch the simple test process
            process = subprocess.Popen(
                ['./simple_test', str(iterator_start), str(param_kef)],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                preexec_fn=os.setsid  # Create new process group
            )
            
            self.processes[process_id] = {
                'process': process,
                'pid': process.pid,
                'params': {
                    'iterator_start': iterator_start,
                    'param_kef': param_kef
                }
            }
            
            # Start output monitoring thread
            self.output_queues[process_id] = queue.Queue()
            monitor_thread = threading.Thread(
                target=self.monitor_process_output,
                args=(process_id, process)
            )
            monitor_thread.daemon = True
            monitor_thread.start()
            
            print(f"Process {process_id} (PID: {process.pid}) launched with iterator_start={iterator_start}")
            return True
            
        except Exception as e:
            print(f"Failed to launch process {process_id}: {e}")
            return False
    
    def monitor_process_output(self, process_id, process):
        """Monitor output from a test process and extract data"""
        while self.running and process.poll() is None:
            try:
                line = process.stdout.readline()
                if line:
                    line = line.strip()
                    self.parse_output_line(process_id, line)
                    # Store in queue for real-time viewing
                    self.output_queues[process_id].put(line)
            except:
                break
    
    def parse_output_line(self, process_id, line):
        """Parse output lines and extract relevant data"""
        try:
            # Initialize process data structure if needed
            if process_id not in self.process_data:
                self.process_data[process_id] = {
                    'current_params': {},  # Current parameter values
                    'initial_params': {},  # Initial parameter values from startup
                    'param_changes': [],   # History of parameter changes
                    'raw_output': [],
                    'iterator_values': [],
                    'last_update': time.time()
                }
            
            # Parse initial parameter values at startup
            # Pattern: "Param X: Y" (initial values)
            initial_param_match = re.match(r'Param (\d+): (\d+)$', line)
            if initial_param_match:
                param_num = int(initial_param_match.group(1))
                param_value = int(initial_param_match.group(2))
                
                self.process_data[process_id]['initial_params'][param_num] = {
                    'value': param_value,
                    'timestamp': time.time()
                }
                # Also set as current value initially
                self.process_data[process_id]['current_params'][param_num] = {
                    'value': param_value,
                    'timestamp': time.time(),
                    'source': 'initial'
                }
            
            # Parse real-time parameter changes
            # Pattern: "Param X changed from Y to Z"
            change_match = re.match(r'Param (\d+) changed from (\d+) to (\d+)', line)
            if change_match:
                param_num = int(change_match.group(1))
                old_value = int(change_match.group(2))
                new_value = int(change_match.group(3))
                
                # Record the change
                change_record = {
                    'param_num': param_num,
                    'old_value': old_value,
                    'new_value': new_value,
                    'timestamp': time.time(),
                    'line': line
                }
                self.process_data[process_id]['param_changes'].append(change_record)
                
                # Update current parameter value
                self.process_data[process_id]['current_params'][param_num] = {
                    'value': new_value,
                    'timestamp': time.time(),
                    'source': 'changed',
                    'previous_value': old_value
                }
                
                # Keep only last 100 changes
                if len(self.process_data[process_id]['param_changes']) > 100:
                    self.process_data[process_id]['param_changes'] = \
                        self.process_data[process_id]['param_changes'][-100:]
            
            # Look for iterator values
            # Pattern: "ITERATOR: -32766"
            iterator_match = re.search(r'ITERATOR:\s*(-?\d+)', line)
            if iterator_match:
                iterator_val = int(iterator_match.group(1))
                self.process_data[process_id]['iterator_values'].append({
                    'value': iterator_val,
                    'timestamp': time.time()
                })
                # Keep only last 100 iterator values
                if len(self.process_data[process_id]['iterator_values']) > 100:
                    self.process_data[process_id]['iterator_values'] = \
                        self.process_data[process_id]['iterator_values'][-100:]
            
            # Store raw output for debugging (keep last 50 lines)
            self.process_data[process_id]['raw_output'].append({
                'line': line,
                'timestamp': time.time()
            })
            if len(self.process_data[process_id]['raw_output']) > 50:
                self.process_data[process_id]['raw_output'] = \
                    self.process_data[process_id]['raw_output'][-50:]
            
            self.process_data[process_id]['last_update'] = time.time()
            
        except Exception as e:
            pass  # Ignore parsing errors

    def compare_process_data(self):
        """Compare CURRENT parameter values between all processes"""
        if len(self.processes) < 2:
            print("Need at least 2 processes to compare")
            return
        
        print("\n" + "="*70)
        print("SHARED_DATA TEST COMPARISON")
        print("="*70)
        
        # Compare CURRENT parameter values
        print("\nCurrent Parameter Values:")
        print("-" * 50)
        all_param_nums = set()
        for proc_id in self.process_data:
            if 'current_params' in self.process_data[proc_id]:
                all_param_nums.update(self.process_data[proc_id]['current_params'].keys())
        
        for param_num in sorted(all_param_nums):
            print(f"\nParameter {param_num}:")
            current_values = {}
            timestamps = {}
            sources = {}
            
            for proc_id in sorted(self.processes.keys()):
                if ('current_params' in self.process_data[proc_id] and 
                    param_num in self.process_data[proc_id]['current_params']):
                    param_info = self.process_data[proc_id]['current_params'][param_num]
                    val = param_info['value']
                    ts = param_info['timestamp']
                    source = param_info.get('source', 'unknown')
                    
                    current_values[proc_id] = val
                    timestamps[proc_id] = ts
                    sources[proc_id] = source
                    
                    time_ago = time.time() - ts
                    prev_info = ""
                    if 'previous_value' in param_info:
                        prev_info = f" (was {param_info['previous_value']})"
                    
                    print(f"  Process {proc_id}: {val}{prev_info} [{source}, {time_ago:.1f}s ago]")
                else:
                    print(f"  Process {proc_id}: No data")
            
            # Check for differences in CURRENT values
            unique_values = set(current_values.values()) if current_values else set()
            if len(unique_values) > 1:
                print(f"  ⚠️  DIFFERENCE DETECTED! Current values: {unique_values}")
                # Show which processes have which values
                value_to_procs = {}
                for proc_id, val in current_values.items():
                    if val not in value_to_procs:
                        value_to_procs[val] = []
                    value_to_procs[val].append(proc_id)
                for val, procs in value_to_procs.items():
                    print(f"     Value {val}: Processes {procs}")
            elif len(unique_values) == 1:
                print(f"  ✅ All processes synchronized (current value: {list(unique_values)[0]})")
        
        # Compare latest iterator values
        print("\nLatest Iterator Values:")
        print("-" * 40)
        iterator_values = {}
        for proc_id in sorted(self.processes.keys()):
            if (proc_id in self.process_data and 
                'iterator_values' in self.process_data[proc_id] and
                self.process_data[proc_id]['iterator_values']):
                latest = self.process_data[proc_id]['iterator_values'][-1]
                iterator_values[proc_id] = latest['value']
                ts_ago = time.time() - latest['timestamp']
                print(f"  Process {proc_id}: {latest['value']} (updated {ts_ago:.1f}s ago)")
            else:
                print(f"  Process {proc_id}: No data")
        
        # Check iterator synchronization
        unique_iter_values = set(iterator_values.values()) if iterator_values else set()
        if len(unique_iter_values) > 1:
            print(f"  ⚠️  ITERATOR SYNC ISSUE! Values: {unique_iter_values}")
        elif len(unique_iter_values) == 1:
            print(f"  ✅ All iterators synchronized (value: {list(unique_iter_values)[0]})")

    def send_parameter_change(self, process_id, param_num, new_value):
        """Send a parameter change request via SSV message"""
        if process_id not in self.processes:
            print(f"Process {process_id} not found")
            return False
        
        try:
            process = self.processes[process_id]['process']
            # Send SSV command to the process via stdin
            command = f"SSV {param_num} {new_value}\n"
            process.stdin.write(command)
            process.stdin.flush()
            print(f"✅ Sent SSV message to process {process_id}: Change param {param_num} to {new_value}")
            return True
        except Exception as e:
            print(f"❌ Failed to send SSV message to process {process_id}: {e}")
            return False
    
    def send_ssv_interactive(self):
        """Interactive SSV message sending"""
        if not self.processes:
            print("No processes running")
            return
        
        print("\n" + "="*50)
        print("SEND SSV MESSAGE")
        print("="*50)
        
        # Show available processes
        print("Available processes:")
        for proc_id in sorted(self.processes.keys()):
            status = "Running" if self.processes[proc_id]['process'].poll() is None else "Stopped"
            print(f"  Process {proc_id}: PID {self.processes[proc_id]['pid']} - {status}")
        
        try:
            # Get process range
            min_proc = min(self.processes.keys())
            max_proc = max(self.processes.keys())
            
            print(f"\nProcess range: {min_proc} to {max_proc}")
            sender_proc = int(input(f"Enter sender process ID ({min_proc}-{max_proc}): "))
            
            if sender_proc not in self.processes:
                print(f"Process {sender_proc} not found")
                return
            
            if self.processes[sender_proc]['process'].poll() is not None:
                print(f"Process {sender_proc} is not running")
                return
            
            # Get parameter info
            param_num = int(input("Enter parameter number to change: "))
            new_value = int(input("Enter new parameter value: "))
            
            # Send the SSV message
            success = self.send_parameter_change(sender_proc, param_num, new_value)
            
            if success:
                print(f"\n✅ SSV message sent successfully!")
                print(f"   Sender: Process {sender_proc}")
                print(f"   Parameter: {param_num}")
                print(f"   New value: {new_value}")
                print(f"\nWatch for parameter changes in the comparison output...")
            
        except ValueError:
            print("Invalid input. Please enter valid numbers.")
        except Exception as e:
            print(f"Error: {e}")

    def interactive_menu(self, base_iterator, iterator_step, param_kef):
        """Interactive menu for controlling the test"""
        while self.running:
            print("\n" + "="*60)
            print("SHARED_DATA TEST ORCHESTRATOR")
            print("="*60)
            print(f"Base config: Iterator={base_iterator}, Step={iterator_step}, ParamKef={param_kef}")
            print("-" * 60)
            print("1. Launch new test process")
            print("2. List running processes")
            print("3. Compare process data")
            print("4. Show process output")
            print("5. Show parameter change history")
            print("6. Send SSV message")  # Changed from SSRV to SSV
            print("7. Stop specific process")
            print("8. Auto-launch multiple processes")
            print("9. Debug parameter detection")
            print("10. Stop all processes and exit")
            
            try:
                choice = input("\nEnter your choice (1-10): ").strip()
                
                if choice == '1':
                    proc_id = max(self.processes.keys()) + 1 if self.processes else 0
                    # Calculate iterator start for this process
                    iterator_start = base_iterator + (proc_id * iterator_step)
                    print(f"Launching process {proc_id} with iterator_start={iterator_start}")
                    self.launch_test_process(proc_id, iterator_start, param_kef)
                
                elif choice == '2':
                    self.list_processes()
                
                elif choice == '3':
                    self.compare_process_data()
                
                elif choice == '4':
                    self.list_processes()
                    if self.processes:
                        proc_id = int(input("Enter process ID: "))
                        lines = int(input("Number of lines to show (default 10): ") or "10")
                        self.show_process_output(proc_id, lines)
                
                elif choice == '5':
                    self.list_processes()
                    if self.processes:
                        proc_id = int(input("Enter process ID: "))
                        param_choice = input("Enter parameter number (or press Enter for all): "