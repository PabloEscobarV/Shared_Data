#!/bin/bash

echo "🚀 MULTI-PROCESS STRESS TEST (kef=1)"
echo "====================================="
echo "Testing with P_COUNT=64, all processes focusing on param #0"
echo ""

# Get number of processes from command line (default 10)
NUM_PROCESSES=${1:-10}

if [[ $NUM_PROCESSES -gt 50 ]]; then
    echo "⚠️  Limiting to 50 processes to avoid system overload"
    NUM_PROCESSES=50
fi

echo "🎯 Configuration:"
echo "   Processes: $NUM_PROCESSES"
echo "   Parameter focus: #0 (kef=1 for all)"
echo "   Test duration: 30 seconds"
echo "   P_COUNT: 64"
echo ""

# Clean up any existing processes and logs
pkill -f test_full_algorithm 2>/dev/null || true
rm -f stress_test_p*.log 2>/dev/null || true

echo "🚀 Starting $NUM_PROCESSES processes..."

# Start processes with varied iterator values
for ((i=0; i<NUM_PROCESSES; i++)); do
    # Create iterator values that will test edge cases
    if [[ $i -eq 0 ]]; then
        iter=10      # Low iterator
    elif [[ $i -eq 1 ]]; then
        iter=250     # High iterator (wraparound test)
    elif [[ $i -eq 2 ]]; then
        iter=100     # Medium iterator
    elif [[ $i -eq 3 ]]; then
        iter=100     # Same as process 2 (process ID priority test)
    elif [[ $i -eq 4 ]]; then
        iter=200     # Another high iterator
    elif [[ $i -eq 5 ]]; then
        iter=50      # Another low iterator
    else
        # Random iterator for remaining processes
        iter=$((RANDOM % 256))
    fi
    
    echo "   Process $i: iterator=$iter, kef=1"
    timeout 35s ./test_full_algorithm $iter 1 30000 > "stress_test_p${i}.log" 2>&1 &
    
    # Small delay between process starts
    sleep 0.1
done

echo ""
echo "📡 Running stress test for 32 seconds..."

# Progress indicator
for ((t=32; t>0; t--)); do
    running=$(ps aux | grep -c '[t]est_full_algorithm' || echo "0")
    printf "\r⏱️  Time remaining: %2ds (%d processes running)   " $t $running
    sleep 1
done

echo ""
echo ""
echo "🛑 Stopping any remaining processes..."
pkill -f test_full_algorithm 2>/dev/null || true
wait 2>/dev/null || true

echo ""
echo "📊 STRESS TEST ANALYSIS:"
echo "========================"

# Collect statistics
total_processes=0
successful_processes=0
total_ssv_sent=0
total_ssv_received=0
total_ssrv_sent=0
total_ssrv_received=0
total_param_changes=0
unique_final_values=()

echo ""
echo "📋 Individual Process Results:"
echo "------------------------------"

for ((i=0; i<NUM_PROCESSES; i++)); do
    if [[ -f "stress_test_p${i}.log" ]]; then
        total_processes=$((total_processes + 1))
        
        # Extract statistics
        ssv_sent=$(grep "SSV:" "stress_test_p${i}.log" | grep "sent" | tail -1 | awk '{print $(NF-1)}' 2>/dev/null || echo "0")
        ssv_received=$(grep "SSV:" "stress_test_p${i}.log" | grep "received" | tail -1 | awk '{print $(NF-1)}' 2>/dev/null || echo "0")
        ssrv_sent=$(grep "SSRV:" "stress_test_p${i}.log" | grep "sent" | tail -1 | awk '{print $(NF-1)}' 2>/dev/null || echo "0")
        ssrv_received=$(grep "SSRV:" "stress_test_p${i}.log" | grep "received" | tail -1 | awk '{print $(NF-1)}' 2>/dev/null || echo "0")
        param_changes=$(grep "Parameter Changes:" "stress_test_p${i}.log" | tail -1 | awk '{print $NF}' 2>/dev/null || echo "0")
        final_param=$(grep "Param #0:" "stress_test_p${i}.log" | tail -1 | awk '{print $NF}' 2>/dev/null || echo "unknown")
        
        # Check if process was successful
        if [[ $ssv_sent -gt 0 || $ssv_received -gt 0 ]]; then
            successful_processes=$((successful_processes + 1))
        fi
        
        # Accumulate totals
        total_ssv_sent=$((total_ssv_sent + ssv_sent))
        total_ssv_received=$((total_ssv_received + ssv_received))
        total_ssrv_sent=$((total_ssrv_sent + ssrv_sent))
        total_ssrv_received=$((total_ssrv_received + ssrv_received))
        total_param_changes=$((total_param_changes + param_changes))
        
        # Track unique final values
        if [[ "$final_param" != "unknown" ]]; then
            unique_final_values+=("$final_param")
        fi
        
        echo "P$i: SSV($ssv_sent/$ssv_received) SSRV($ssrv_sent/$ssrv_received) Changes:$param_changes Final:$final_param"
    else
        echo "P$i: ❌ Log file missing"
    fi
done

echo ""
echo "📊 AGGREGATE STATISTICS:"
echo "========================"
echo "Processes completed: $total_processes/$NUM_PROCESSES"
echo "Successful processes: $successful_processes/$total_processes"
echo "Total SSV messages: $total_ssv_sent sent, $total_ssv_received received"
echo "Total SSRV messages: $total_ssrv_sent sent, $total_ssrv_received received"
echo "Total parameter changes: $total_param_changes"

# Analyze convergence
unique_values=($(printf '%s\n' "${unique_final_values[@]}" | sort -u))
echo "Unique final parameter values: ${#unique_values[@]}"
echo "Final values: ${unique_values[*]}"

echo ""
echo "✅ STRESS TEST VALIDATION:"
echo "=========================="

# Success criteria
success_rate=$((successful_processes * 100 / total_processes))
avg_ssv_per_process=$((total_ssv_sent / successful_processes))
avg_param_changes=$((total_param_changes / successful_processes))

echo "Success rate: $success_rate%"
echo "Avg SSV per process: $avg_ssv_per_process"
echo "Avg param changes: $avg_param_changes"

if [[ $success_rate -ge 80 ]]; then
    echo "✅ Process stability: PASSED ($success_rate% success)"
else
    echo "❌ Process stability: FAILED ($success_rate% success)"
fi

if [[ $total_ssv_sent -gt 0 && $total_ssv_received -gt 0 ]]; then
    echo "✅ SSV Communication: WORKING"
else
    echo "❌ SSV Communication: FAILED"
fi

if [[ $total_ssrv_sent -gt 0 && $total_ssrv_received -gt 0 ]]; then
    echo "✅ SSRV Communication: WORKING"
else
    echo "❌ SSRV Communication: FAILED"
fi

if [[ ${#unique_values[@]} -le 3 ]]; then
    echo "✅ Parameter convergence: GOOD (${#unique_values[@]} unique values)"
else
    echo "⚠️  Parameter convergence: NEEDS_IMPROVEMENT (${#unique_values[@]} unique values)"
fi

echo ""
echo "🔍 DETAILED EDGE CASE ANALYSIS:"
echo "==============================="

# Check specific edge cases
if [[ -f "stress_test_p0.log" && -f "stress_test_p1.log" ]]; then
    echo "📊 Iterator wraparound test (P0:iter=10 vs P1:iter=250):"
    p0_final=$(grep "Param #0:" "stress_test_p0.log" | tail -1 | awk '{print $NF}' 2>/dev/null || echo "unknown")
    p1_final=$(grep "Param #0:" "stress_test_p1.log" | tail -1 | awk '{print $NF}' 2>/dev/null || echo "unknown")
    echo "   P0 final: $p0_final, P1 final: $p1_final"
    
    if [[ "$p0_final" == "$p1_final" && "$p0_final" != "unknown" ]]; then
        echo "   ✅ Wraparound handling: WORKING"
    else
        echo "   ⚠️  Wraparound handling: NEEDS_CHECK"
    fi
fi

if [[ -f "stress_test_p2.log" && -f "stress_test_p3.log" ]]; then
    echo ""
    echo "📊 Process ID priority test (P2&P3 both iter=100):"
    p2_final=$(grep "Param #0:" "stress_test_p2.log" | tail -1 | awk '{print $NF}' 2>/dev/null || echo "unknown")
    p3_final=$(grep "Param #0:" "stress_test_p3.log" | tail -1 | awk '{print $NF}' 2>/dev/null || echo "unknown")
    echo "   P2 final: $p2_final, P3 final: $p3_final"
    
    if [[ "$p2_final" == "$p3_final" && "$p2_final" != "unknown" ]]; then
        echo "   ✅ Process ID priority: WORKING"
    else
        echo "   ⚠️  Process ID priority: NEEDS_CHECK"
    fi
fi

echo ""
echo "============================================="
if [[ $success_rate -ge 80 && $total_ssv_sent -gt 0 && $total_param_changes -gt 0 ]]; then
    echo "🎉 STRESS TEST: PASSED"
    echo "✅ Algorithm scales well with multiple processes!"
else
    echo "❌ STRESS TEST: FAILED"
    echo "⚠️  Algorithm needs optimization for multi-process scenarios"
fi
echo "============================================="
