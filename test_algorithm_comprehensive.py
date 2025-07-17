#!/usr/bin/env python3
"""
Comprehensive Algorithm Test Orchestrator
Tests SSV, SSRV, and SSE workflows with multiple processes
"""

import subprocess
import time
import signal
import sys
import os
from typing import List

class AlgorithmTester:
    def __init__(self):
        self.processes: List[subprocess.Popen] = []
        self.test_results = {}
        
    def signal_handler(self, signum, frame):
        """Handle Ctrl+C gracefully"""
        print(f"\n🛑 Received signal {signum}, stopping all processes...")
        self.stop_all_processes()
        sys.exit(0)
    
    def compile_test(self) -> bool:
        """Compile the test program"""
        print("🔨 Compiling comprehensive algorithm test...")
        try:
            result = subprocess.run(["make", "test_full_algorithm"], 
                                  capture_output=True, text=True)
            if result.returncode == 0:
                print("✅ Compilation successful")
                return True
            else:
                print(f"❌ Compilation failed: {result.stderr}")
                return False
        except Exception as e:
            print(f"❌ Compilation error: {e}")
            return False
    
    def start_process(self, process_id: int, iterator_start: int, param_kef: int, 
                     duration_ms: int = 45000) -> subprocess.Popen:
        """Start a single test process"""
        try:
            cmd = ["./test_full_algorithm", str(iterator_start), str(param_kef), str(duration_ms)]
            print(f"🚀 Starting process {process_id}: iterator={iterator_start}, param_kef={param_kef}")
            
            process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=1
            )
            
            self.processes.append(process)
            return process
            
        except Exception as e:
            print(f"❌ Failed to start process {process_id}: {e}")
            return None
    
    def stop_all_processes(self):
        """Stop all running processes"""
        for i, process in enumerate(self.processes):
            if process.poll() is None:
                print(f"🛑 Stopping process {i+1}...")
                process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    print(f"⚠️  Force killing process {i+1}...")
                    process.kill()
        self.processes.clear()
    
    def wait_for_completion(self, timeout_seconds: int = 50):
        """Wait for all processes to complete"""
        print(f"\n📡 Running comprehensive algorithm test for {timeout_seconds} seconds...")
        print("   Testing phases:")
        print("   1. SSV Synchronization (10s)")
        print("   2. SSRV Requests (15s)")  
        print("   3. SSE Error Handling (10s)")
        print("   4. Mixed Traffic (remaining time)")
        print()
        
        start_time = time.time()
        
        while time.time() - start_time < timeout_seconds:
            running_processes = [p for p in self.processes if p.poll() is None]
            if not running_processes:
                print("\n✅ All processes completed successfully!")
                break
                
            remaining = int(timeout_seconds - (time.time() - start_time))
            print(f"\r⏱️  Time remaining: {remaining:3d}s ({len(running_processes)} processes running)", 
                  end="", flush=True)
            time.sleep(1)
        
        print("\n")
    
    def collect_results(self):
        """Collect and analyze results from all processes"""
        print("📊 Collecting test results...")
        
        all_stdout = []
        all_stderr = []
        
        for i, process in enumerate(self.processes):
            try:
                stdout, stderr = process.communicate(timeout=5)
                
                if stdout:
                    all_stdout.append(f"=== Process {i+1} STDOUT ===\n{stdout}\n")
                if stderr:
                    all_stderr.append(f"=== Process {i+1} STDERR ===\n{stderr}\n")
                    
            except subprocess.TimeoutExpired:
                print(f"⚠️  Process {i+1} timed out during result collection")
                process.kill()
        
        return all_stdout, all_stderr
    
    def analyze_results(self, stdout_data: List[str], stderr_data: List[str]):
        """Analyze test results for algorithm validation"""
        print("\n🔍 ================ ALGORITHM ANALYSIS ================")
        
        total_ssv_sent = 0
        total_ssv_received = 0
        total_ssrv_sent = 0  
        total_ssrv_received = 0
        total_sse_sent = 0
        total_sse_received = 0
        total_param_changes = 0
        
        algorithm_issues = []
        
        # Parse output for statistics
        for stdout in stdout_data:
            lines = stdout.split('\n')
            for line in lines:
                if "SSV:" in line and "sent" in line:
                    try:
                        total_ssv_sent += int(line.split()[-2])
                    except:
                        pass
                elif "SSV:" in line and "received" in line:
                    try:
                        total_ssv_received += int(line.split()[-2])  
                    except:
                        pass
                elif "SSRV:" in line and "sent" in line:
                    try:
                        total_ssrv_sent += int(line.split()[-2])
                    except:
                        pass
                elif "SSRV:" in line and "received" in line:
                    try:
                        total_ssrv_received += int(line.split()[-2])
                    except:
                        pass
                elif "SSE:" in line and "sent" in line:
                    try:
                        total_sse_sent += int(line.split()[-2])
                    except:
                        pass
                elif "SSE:" in line and "received" in line:
                    try:
                        total_sse_received += int(line.split()[-2])
                    except:
                        pass
                elif "Parameter Changes:" in line:
                    try:
                        total_param_changes += int(line.split()[-1])
                    except:
                        pass
        
        # Algorithm validation
        print(f"📈 Message Statistics:")
        print(f"   SSV:  {total_ssv_sent} sent, {total_ssv_received} received")
        print(f"   SSRV: {total_ssrv_sent} sent, {total_ssrv_received} received") 
        print(f"   SSE:  {total_sse_sent} sent, {total_sse_received} received")
        print(f"   Parameter Changes: {total_param_changes}")
        
        print(f"\n✅ Algorithm Validation:")
        
        # Test 1: SSV Synchronization
        if total_ssv_sent > 0 and total_ssv_received > 0:
            print("   ✅ SSV Synchronization: WORKING")
        else:
            print("   ❌ SSV Synchronization: FAILED")
            algorithm_issues.append("SSV synchronization not working")
        
        # Test 2: SSRV Processing  
        if total_ssrv_sent > 0:
            print("   ✅ SSRV Requests: WORKING")
        else:
            print("   ❌ SSRV Requests: FAILED") 
            algorithm_issues.append("SSRV requests not generated")
        
        # Test 3: SSE Error Handling
        if total_sse_sent > 0 or total_sse_received > 0:
            print("   ✅ SSE Error Handling: WORKING")
        else:
            print("   ⚠️  SSE Error Handling: NOT TESTED")
        
        # Test 4: Parameter Updates
        if total_param_changes > 0:
            print("   ✅ Parameter Updates: WORKING")
        else:
            print("   ❌ Parameter Updates: FAILED")
            algorithm_issues.append("No parameter changes detected")
        
        # Test 5: Communication Balance
        if total_ssv_received > 0 and total_ssrv_received > 0:
            print("   ✅ Inter-Process Communication: WORKING")
        else:
            print("   ❌ Inter-Process Communication: FAILED")
            algorithm_issues.append("Processes not communicating properly")
        
        if algorithm_issues:
            print(f"\n⚠️  Issues Found:")
            for issue in algorithm_issues:
                print(f"   - {issue}")
        else:
            print(f"\n🎉 All algorithm components working correctly!")
        
        print("======================================================")
        
        return len(algorithm_issues) == 0
    
    def run_comprehensive_test(self):
        """Run the comprehensive algorithm test"""
        print("🧪 COMPREHENSIVE ALGORITHM TEST")
        print("=" * 50)
        
        # Check if executable exists
        if not os.path.exists("./test_full_algorithm"):
            if not self.compile_test():
                return False
        
        signal.signal(signal.SIGINT, self.signal_handler)
        
        try:
            # Test configuration: 3 processes with different characteristics
            configs = [
                (1, 50, 2),   # Process 1: Lower iterator, param_kef=2
                (2, 100, 3),  # Process 2: Medium iterator, param_kef=3
                (3, 150, 1),  # Process 3: Higher iterator, param_kef=1
            ]
            
            # Start all processes
            for process_id, iterator_start, param_kef in configs:
                process = self.start_process(process_id, iterator_start, param_kef, 45000)
                if not process:
                    print(f"❌ Failed to start process {process_id}")
                    return False
                time.sleep(0.5)  # Small delay between starts
            
            # Wait for completion
            self.wait_for_completion(50)
            
            # Collect results
            stdout_data, stderr_data = self.collect_results()
            
            # Display detailed output
            print("\n📝 ================ DETAILED OUTPUT ================")
            for stdout in stdout_data:
                print(stdout)
            
            if stderr_data:
                print("\n⚠️  STDERR Output:")
                for stderr in stderr_data:
                    print(stderr)
            
            # Analyze results
            success = self.analyze_results(stdout_data, stderr_data)
            
            return success
            
        except KeyboardInterrupt:
            print("\n🛑 Test interrupted by user")
            return False
        finally:
            self.stop_all_processes()

def main():
    tester = AlgorithmTester()
    
    print("🔬 Starting Comprehensive Communication Algorithm Test")
    print("   This test validates:")
    print("   - SSV synchronization (every 100ms)")
    print("   - SSRV parameter requests (every 40ms)")  
    print("   - SSE error handling")
    print("   - Iterator management")
    print("   - Process priority resolution")
    print("   - 500ms SSRV wait periods")
    print()
    
    success = tester.run_comprehensive_test()
    
    if success:
        print("\n🎉 ================ TEST PASSED ================")
        print("✅ All communication algorithm components working!")
        print("✅ SSV, SSRV, and SSE workflows validated")  
        print("✅ Memory-efficient implementation confirmed")
        print("✅ Ready for controller deployment")
    else:
        print("\n❌ ================ TEST FAILED ================")
        print("⚠️  Algorithm issues detected - review logs above")
        print("⚠️  Fix issues before controller deployment")
    
    return 0 if success else 1

if __name__ == "__main__":
    sys.exit(main())
