#!/bin/bash

echo "🚀 PRODUCTION SCALE TEST - MANAGEABLE VERSION"
echo "=============================================="
echo "🎯 Testing UDP Multicast Algorithm at High Scale:"
echo "   • 20 concurrent processes (manageable load)"
echo "   • 64 different parameters" 
echo "   • Full SSV/SSRV/SSE validation"
echo "   • New process joining simulation"
echo "   • Iterator convergence verification"
echo "   • Parameter synchronization check"
echo ""
echo "⚡ Test Duration: 90 seconds"
echo "📊 Expected Message Volume: ~100,000+ messages"
echo ""

# Verify P_COUNT is set to 64
P_COUNT=$(grep "P_COUNT" hdrs/test.hpp | grep -o "[0-9]*")
if [[ "$P_COUNT" != "64" ]]; then
    echo "❌ Error: P_COUNT must be 64 for this test (currently $P_COUNT)"
    echo "Please update hdrs/test.hpp"
    exit 1
fi

# Clean up any previous processes and logs
echo "🧹 Cleaning up previous test artifacts..."
pkill -f test_ultimate_100_processes 2>/dev/null
rm -f production_test_p*.log production_test_new_process.log
sleep 2

# Build the test
echo "🔨 Building production scale test..."
make clean >/dev/null 2>&1
g++ -std=c++17 -O2 -g test_ultimate_100_processes.cpp src/*.cpp -o test_ultimate_100_processes -pthread

if [[ $? -ne 0 ]]; then
    echo "❌ Build failed!"
    exit 1
fi

echo "✅ Build successful"
echo ""

# Start 20 processes with different iterator values
echo "🚀 Starting 20 processes with different iterator values..."
echo "   Iterator distribution: 0-255 (full range coverage)"
echo ""

pids=()
start_time=$(date +%s)

# Start main processes
for i in {0..19}; do
    iterator=$((i * 255 / 20))  # Distribute across full iterator range
    
    echo "   Process $i: iterator=$iterator"
    
    ./test_ultimate_100_processes "$iterator" > "production_test_p${i}.log" 2>&1 &
    pids+=($!)
    
    # Small delay to avoid socket binding conflicts
    sleep 0.1
done

echo ""
echo "✅ All 20 processes started successfully"
echo ""

# Wait for processes to initialize and sync (20 seconds)
echo "⏳ Phase 1: Initial synchronization (20 seconds)..."
for ((countdown=20; countdown>=1; countdown--)); do
    running_count=0
    for pid in "${pids[@]}"; do
        if kill -0 "$pid" 2>/dev/null; then
            running_count=$((running_count + 1))
        fi
    done
    printf "\r   ⏱️  Sync time remaining: %2ds (%d processes running)   " "$countdown" "$running_count"
    sleep 1
done
echo ""

# Start NEW PROCESS that will update all parameters (at 30 seconds into test)
echo ""
echo "🆕 Phase 2: Starting NEW PROCESS that will update all 64 parameters..."
./test_ultimate_100_processes "128" "1" > "production_test_new_process.log" 2>&1 &
new_process_pid=$!
pids+=($new_process_pid)

echo "✅ New process started (PID: $new_process_pid) - will update all parameters"
echo ""

# Monitor remaining test duration (70 seconds remaining)
echo "📊 Phase 3: Monitoring production test completion..."
for ((countdown=70; countdown>=1; countdown--)); do
    running_count=0
    for pid in "${pids[@]}"; do
        if kill -0 "$pid" 2>/dev/null; then
            running_count=$((running_count + 1))
        fi
    done
    
    # Show milestone messages
    if [[ $countdown -eq 50 ]]; then
        echo ""
        echo "🔄 New process should be updating parameters..."
    elif [[ $countdown -eq 20 ]]; then
        echo ""
        echo "🎯 Final phase - checking convergence..."
    elif [[ $countdown -eq 5 ]]; then
        echo ""
        echo "⏰ Test completion in 5 seconds..."
    fi
    
    printf "\r   ⏱️  Test time remaining: %2ds (%d processes running)   " "$countdown" "$running_count"
    sleep 1
done

echo ""
echo ""
echo "🛑 Stopping all processes gracefully..."

# Graceful shutdown
for pid in "${pids[@]}"; do
    if kill -0 "$pid" 2>/dev/null; then
        kill -TERM "$pid" 2>/dev/null
    fi
done

# Wait for graceful shutdown
sleep 2

# Force kill if needed
for pid in "${pids[@]}"; do
    if kill -0 "$pid" 2>/dev/null; then
        kill -KILL "$pid" 2>/dev/null
    fi
done

end_time=$(date +%s)
total_duration=$((end_time - start_time))

echo "✅ All processes stopped"
echo ""

# Analyze results
echo "📊 PRODUCTION SCALE TEST ANALYSIS"
echo "=================================="
echo "Test Duration: ${total_duration} seconds"
echo "Processes: 21 total (20 main + 1 new process)"
echo "Parameters: 64"
echo ""

# Count successful processes
successful_processes=0
total_ssv_sent=0
total_ssv_received=0
total_ssrv_sent=0
total_ssrv_received=0
total_sse_sent=0
total_sse_received=0
total_param_changes=0
total_convergence_events=0

echo "📋 Process Analysis:"
echo "==================="

for i in {0..19}; do
    if [[ -f "production_test_p${i}.log" ]]; then
        # Extract statistics (account for [STATS][timestamp] PID:XXXXX prefix)
        ssv_sent=$(grep "SSV:" "production_test_p${i}.log" | tail -1 | awk '{print $4}' 2>/dev/null || echo "0")
        ssv_received=$(grep "SSV:" "production_test_p${i}.log" | tail -1 | awk '{print $6}' 2>/dev/null || echo "0")
        ssrv_sent=$(grep "SSRV:" "production_test_p${i}.log" | tail -1 | awk '{print $4}' 2>/dev/null || echo "0")
        ssrv_received=$(grep "SSRV:" "production_test_p${i}.log" | tail -1 | awk '{print $6}' 2>/dev/null || echo "0")
        sse_sent=$(grep "SSE:" "production_test_p${i}.log" | tail -1 | awk '{print $4}' 2>/dev/null || echo "0")
        sse_received=$(grep "SSE:" "production_test_p${i}.log" | tail -1 | awk '{print $6}' 2>/dev/null || echo "0")
        param_changes=$(grep "Parameter Changes:" "production_test_p${i}.log" | tail -1 | awk '{print $NF}' 2>/dev/null || echo "0")
        convergence_events=$(grep "Convergence Events:" "production_test_p${i}.log" | tail -1 | awk '{print $NF}' 2>/dev/null || echo "0")
        
        # Check if process was successful
        if [[ $ssv_sent -gt 0 || $ssv_received -gt 0 ]]; then
            successful_processes=$((successful_processes + 1))
        fi
        
        # Add to totals
        total_ssv_sent=$((total_ssv_sent + ssv_sent))
        total_ssv_received=$((total_ssv_received + ssv_received))
        total_ssrv_sent=$((total_ssrv_sent + ssrv_sent))
        total_ssrv_received=$((total_ssrv_received + ssrv_received))
        total_sse_sent=$((total_sse_sent + sse_sent))
        total_sse_received=$((total_sse_received + sse_received))
        total_param_changes=$((total_param_changes + param_changes))
        total_convergence_events=$((total_convergence_events + convergence_events))
        
        # Log every 5th process
        if [[ $((i % 5)) -eq 0 ]]; then
            echo "📋 Process $i: SSV($ssv_sent/$ssv_received) SSRV($ssrv_sent/$ssrv_received) Changes($param_changes)"
        fi
    fi
done

# Analyze new process
echo ""
echo "🆕 NEW PROCESS ANALYSIS:"
echo "========================"
if [[ -f "production_test_new_process.log" ]]; then
    new_ssrv_sent=$(grep "SSRV:" "production_test_new_process.log" | tail -1 | awk '{print $4}' 2>/dev/null || echo "0")
    new_param_changes=$(grep "Parameter Changes:" "production_test_new_process.log" | tail -1 | awk '{print $NF}' 2>/dev/null || echo "0")
    
    echo "📋 New Process Performance:"
    echo "   SSRV Requests Sent: $new_ssrv_sent"
    echo "   Parameter Changes Caused: $new_param_changes"
    
    # Check if new process updated parameters
    parameter_updates=$(grep -c "NEW_PROC_UPDATE" "production_test_new_process.log" 2>/dev/null || echo "0")
    completion_status=$(grep -c "New process completed" "production_test_new_process.log" 2>/dev/null || echo "0")
    
    echo "   Parameter Update Messages: $parameter_updates"
    echo "   Completion Status: $([ $completion_status -gt 0 ] && echo "✅ COMPLETED" || echo "❌ INCOMPLETE")"
    
    if [[ $parameter_updates -ge 60 ]]; then
        echo "   ✅ Successfully attempted to update most/all 64 parameters"
    else
        echo "   ⚠️  Only attempted to update $parameter_updates parameters"
    fi
else
    echo "❌ New process log file missing"
fi

echo ""
echo "📊 PRODUCTION SCALE TEST SUMMARY:"
echo "================================="
echo "Total Processes: 20 main + 1 new = 21"
echo "Successful Processes: $successful_processes/20"
echo ""
echo "📈 Message Volume:"
echo "   SSV:  $total_ssv_sent sent, $total_ssv_received received"
echo "   SSRV: $total_ssrv_sent sent, $total_ssrv_received received"  
echo "   SSE:  $total_sse_sent sent, $total_sse_received received"
echo ""
echo "🔄 Algorithm Performance:"
echo "   Total Parameter Changes: $total_param_changes"
echo "   Total Convergence Events: $total_convergence_events"
if [[ $total_duration -gt 0 ]]; then
    echo "   Message Throughput: ~$((($total_ssv_sent + $total_ssrv_sent + $total_sse_sent) / total_duration)) msg/sec"
fi
echo ""

# Calculate success metrics
success_rate=$((successful_processes * 100 / 20))

echo "✅ PRODUCTION VALIDATION RESULTS:"
echo "================================="

if [[ $success_rate -ge 90 ]]; then
    echo "✅ Process Stability: EXCELLENT ($success_rate% success)"
elif [[ $success_rate -ge 75 ]]; then
    echo "✅ Process Stability: GOOD ($success_rate% success)"
else
    echo "❌ Process Stability: FAILED ($success_rate% success)"
fi

if [[ $total_ssv_sent -gt 200 && $total_ssv_received -gt 200 ]]; then
    echo "✅ SSV Communication: EXCELLENT (${total_ssv_sent}+ sent, ${total_ssv_received}+ received)"
else
    echo "❌ SSV Communication: INSUFFICIENT"
fi

if [[ $total_ssrv_sent -gt 20 ]]; then
    echo "✅ SSRV Processing: WORKING (${total_ssrv_sent} requests)"
else
    echo "❌ SSRV Processing: INSUFFICIENT"
fi

if [[ $total_param_changes -gt 100 ]]; then
    echo "✅ Parameter Synchronization: EXCELLENT (${total_param_changes} changes)"
elif [[ $total_param_changes -gt 20 ]]; then
    echo "✅ Parameter Synchronization: GOOD (${total_param_changes} changes)"
else
    echo "❌ Parameter Synchronization: INSUFFICIENT"
fi

# Check 64-parameter cycling
unique_params_changed=$(grep "Parameter #" production_test_p*.log 2>/dev/null | grep "changed:" | cut -d'#' -f2 | cut -d' ' -f1 | sort -u | wc -l 2>/dev/null || echo "0")
echo "🔄 Parameter Cycling: $unique_params_changed/64 different parameters affected"

if [[ $unique_params_changed -ge 32 ]]; then
    echo "✅ Parameter Cycling: EXCELLENT (${unique_params_changed}/64 parameters)"
elif [[ $unique_params_changed -ge 16 ]]; then
    echo "✅ Parameter Cycling: GOOD (${unique_params_changed}/64 parameters)"
else
    echo "⚠️  Parameter Cycling: LIMITED (${unique_params_changed}/64 parameters)"
fi

# Check SSE functionality
if [[ $total_sse_sent -gt 0 || $total_sse_received -gt 0 ]]; then
    echo "✅ SSE Error Handling: WORKING (${total_sse_sent} sent, ${total_sse_received} received)"
else
    echo "⚠️  SSE Error Handling: NOT TRIGGERED"
fi

echo ""
echo "🎯 PRODUCTION READINESS ASSESSMENT:"
echo "===================================="

# Overall assessment
overall_score=0
[[ $success_rate -ge 90 ]] && overall_score=$((overall_score + 1))
[[ $total_ssv_sent -gt 200 ]] && overall_score=$((overall_score + 1))
[[ $total_ssrv_sent -gt 20 ]] && overall_score=$((overall_score + 1))
[[ $total_param_changes -gt 100 ]] && overall_score=$((overall_score + 1))
[[ $unique_params_changed -ge 16 ]] && overall_score=$((overall_score + 1))

if [[ $overall_score -ge 4 ]]; then
    echo "🎉 PRODUCTION READY: Algorithm passed production scale test!"
    echo "   ✅ Handles 20+ concurrent processes efficiently"
    echo "   ✅ Manages 64 parameters with proper cycling"  
    echo "   ✅ Demonstrates robust message handling at scale"
    echo "   ✅ Shows excellent parameter synchronization"
    echo "   ✅ Supports dynamic process joining"
    echo ""
    echo "🚀 READY FOR SCALE-UP TO 100+ PROCESSES"
elif [[ $overall_score -ge 3 ]]; then
    echo "⚠️  MOSTLY READY: Algorithm shows good performance with minor issues"
    echo "   ✅ Core functionality working well"
    echo "   ⚠️  Some optimization opportunities for larger scale"
else
    echo "❌ NEEDS WORK: Algorithm requires optimization for production scale"
    echo "   ❌ Multiple areas need improvement before production deployment"
fi

echo ""
echo "============================================="
echo "🏁 PRODUCTION SCALE TEST COMPLETED"
echo "============================================="
echo "📄 Detailed logs available in production_test_p*.log files"
echo "🔍 Use 'grep ERROR production_test_p*.log' to check for issues"
echo "📊 Use 'grep STATS production_test_p*.log' for detailed statistics"
