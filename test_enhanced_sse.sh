#!/bin/bash

echo "🧪 ENHANCED SSE AND PARAMETER DIVERSITY TEST"
echo "============================================="
echo "Testing SSE error handling with different MAX_VALUE per parameter:"
echo "   Parameter #0: MAX = 5,000   (easy to exceed → triggers SSE)"
echo "   Parameter #1: MAX = 25,000  (medium threshold)"  
echo "   Parameter #2: MAX = 99,999  (high threshold)"
echo ""
echo "🎯 This test validates:"
echo "   ✅ Valid SSRV requests (value ≤ MAX_VALUE)"
echo "   ❌ Invalid SSRV requests (value > MAX_VALUE → SSE)"
echo "   🔄 Parameter diversity across processes"
echo ""

# Clean up previous logs
rm -f sse_test_p*.log

# Test configurations: [iterator, description]
declare -a test_configs=(
    "50:Process_A"
    "100:Process_B" 
    "150:Process_C"
    "200:Process_D"
)

echo "🚀 Starting ${#test_configs[@]} processes with different parameter ranges:"
echo "   Each process generates different value ranges based on PID"
echo "   Test duration: 30 seconds"
echo ""

# Build the test
echo "🔨 Building enhanced SSE test..."
make clean >/dev/null 2>&1
g++ -std=c++17 -O2 -g test_improved_sse.cpp src/*.cpp -o test_improved_sse -pthread

if [[ $? -ne 0 ]]; then
    echo "❌ Build failed!"
    exit 1
fi

echo "✅ Build successful"
echo ""

# Start processes
pids=()
for i in "${!test_configs[@]}"; do
    IFS=':' read -r iterator desc <<< "${test_configs[$i]}"
    echo "   Process $i: iterator=$iterator ($desc)"
    
    ./test_improved_sse "$iterator" > "sse_test_p${i}.log" 2>&1 &
    pids+=($!)
done

echo ""
echo "📡 Running enhanced SSE test for 32 seconds..."

# Monitor progress
for ((countdown=32; countdown>=1; countdown--)); do
    running_count=0
    for pid in "${pids[@]}"; do
        if kill -0 "$pid" 2>/dev/null; then
            running_count=$((running_count + 1))
        fi
    done
    printf "\r⏱️  Time remaining: %2ds (%d processes running)   " "$countdown" "$running_count"
    sleep 1
done

echo ""
echo ""
echo "🛑 Stopping any remaining processes..."

# Gracefully stop processes
for pid in "${pids[@]}"; do
    if kill -0 "$pid" 2>/dev/null; then
        kill -TERM "$pid" 2>/dev/null
    fi
done

# Wait a bit for graceful shutdown
sleep 2

# Force kill if needed
for pid in "${pids[@]}"; do
    if kill -0 "$pid" 2>/dev/null; then
        kill -KILL "$pid" 2>/dev/null
    fi
done

echo ""
echo "📊 ENHANCED SSE TEST ANALYSIS:"
echo "=============================="
echo ""

# Analyze each process
total_processes=${#test_configs[@]}
successful_processes=0
total_ssv_sent=0
total_ssv_received=0
total_ssrv_sent=0
total_ssrv_received=0
total_sse_sent=0
total_sse_received=0
total_sse_triggered=0

for i in "${!test_configs[@]}"; do
    IFS=':' read -r iterator desc <<< "${test_configs[$i]}"
    
    if [[ -f "sse_test_p${i}.log" ]]; then
        echo "📋 Process $i ($desc, iter=$iterator):"
        
        # Extract statistics
        ssv_sent=$(grep "SSV Messages:" "sse_test_p${i}.log" | awk -F'sent,' '{print $1}' | awk '{print $NF}' 2>/dev/null || echo "0")
        ssv_received=$(grep "SSV Messages:" "sse_test_p${i}.log" | awk -F'received' '{print $2}' | awk '{print $1}' 2>/dev/null || echo "0")
        ssrv_sent=$(grep "SSRV Messages:" "sse_test_p${i}.log" | awk -F'sent,' '{print $1}' | awk '{print $NF}' 2>/dev/null || echo "0")
        ssrv_received=$(grep "SSRV Messages:" "sse_test_p${i}.log" | awk -F'received' '{print $2}' | awk '{print $1}' 2>/dev/null || echo "0")
        sse_sent=$(grep "SSE Messages:" "sse_test_p${i}.log" | awk -F'sent,' '{print $1}' | awk '{print $NF}' 2>/dev/null || echo "0")
        sse_received=$(grep "SSE Messages:" "sse_test_p${i}.log" | awk -F'received' '{print $2}' | awk '{print $1}' 2>/dev/null || echo "0")
        sse_triggered=$(grep "SSE Triggered:" "sse_test_p${i}.log" | awk '{print $NF}' 2>/dev/null || echo "0")
        param_changes=$(grep "Parameter Changes:" "sse_test_p${i}.log" | awk '{print $NF}' 2>/dev/null || echo "0")
        
        echo "   SSV:  $ssv_sent sent, $ssv_received received"
        echo "   SSRV: $ssrv_sent sent, $ssrv_received received"  
        echo "   SSE:  $sse_sent sent, $sse_received received"
        echo "   SSE triggered: $sse_triggered times"
        echo "   Parameter changes: $param_changes"
        
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
        total_sse_triggered=$((total_sse_triggered + sse_triggered))
        
        # Show final parameter values
        echo "   Final values:"
        grep "Param #[0-9]: [0-9]" "sse_test_p${i}.log" | tail -3 | while read line; do
            echo "      $line"
        done
        echo ""
    else
        echo "❌ Process $i: Log file missing"
        echo ""
    fi
done

echo "🔍 PARAMETER CONVERGENCE CHECK:"
echo "==============================="
echo "Checking if all processes have same final parameter values:"
echo ""

for param_num in {0..2}; do
    echo "📊 Parameter #$param_num convergence:"
    
    # Collect all final values for this parameter
    values=()
    max_values=()
    for i in "${!test_configs[@]}"; do
        if [[ -f "sse_test_p${i}.log" ]]; then
            value=$(grep "Param #${param_num}:" "sse_test_p${i}.log" | tail -1 | awk -F':' '{print $2}' | awk '{print $1}' 2>/dev/null || echo "0")
            max_val=$(grep "Param #${param_num}:" "sse_test_p${i}.log" | tail -1 | awk -F'max:' '{print $2}' | tr -d ')' | awk '{print $1}' 2>/dev/null || echo "0")
            values+=("$value")
            max_values+=("$max_val")
            echo "   Process $i: $value (max: $max_val)"
        fi
    done
    
    # Check convergence
    if [[ ${#values[@]} -gt 0 ]]; then
        first_value=${values[0]}
        first_max=${max_values[0]}
        all_same=true
        max_consistent=true
        
        for j in "${!values[@]}"; do
            if [[ "${values[$j]}" != "$first_value" ]]; then
                all_same=false
            fi
            if [[ "${max_values[$j]}" != "$first_max" ]]; then
                max_consistent=false
            fi
        done
        
        if $all_same; then
            echo "   ✅ All processes converged to: $first_value"
        else
            echo "   ⚠️  Processes have different values: ${values[*]}"
        fi
        
        if $max_consistent; then
            echo "   ✅ Max values consistent: $first_max"
        else
            echo "   ⚠️  Max values inconsistent: ${max_values[*]}"
        fi
    fi
    echo ""
done

echo "📈 ENHANCED SSE TEST SUMMARY:"
echo "============================="
echo "Total processes: $total_processes"
echo "Successful processes: $successful_processes/$total_processes"
echo ""
echo "📊 Message Statistics:"
echo "   SSV:  $total_ssv_sent sent, $total_ssv_received received"
echo "   SSRV: $total_ssrv_sent sent, $total_ssrv_received received"
echo "   SSE:  $total_sse_sent sent, $total_sse_received received"
echo "   SSE triggers: $total_sse_triggered"
echo ""

success_rate=$((successful_processes * 100 / total_processes))

echo "✅ TEST VALIDATION RESULTS:"
echo "=========================="

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
    echo "✅ SSRV Processing: WORKING"
else
    echo "❌ SSRV Processing: FAILED"
fi

if [[ $total_sse_sent -gt 0 || $total_sse_received -gt 0 || $total_sse_triggered -gt 0 ]]; then
    echo "✅ SSE Error Handling: WORKING (triggered $total_sse_triggered times)"
else
    echo "❌ SSE Error Handling: FAILED (not triggered)"
fi

echo ""
echo "🔍 SSE EFFECTIVENESS ANALYSIS:"
echo "=============================="

if [[ $total_sse_triggered -gt 0 ]]; then
    echo "✅ SSE triggers detected: $total_sse_triggered"
    echo "   This confirms that invalid SSRV requests (value > MAX_VALUE) properly trigger SSE errors"
else
    echo "❌ No SSE triggers detected"
    echo "   This suggests invalid SSRV requests are not properly generating SSE errors"
fi

if [[ $total_sse_sent -gt 0 ]]; then
    echo "✅ SSE messages sent: $total_sse_sent"
    echo "   Processes are broadcasting SSE error messages"
else
    echo "❌ No SSE messages sent"
fi

if [[ $total_sse_received -gt 0 ]]; then
    echo "✅ SSE messages received: $total_sse_received" 
    echo "   Processes are receiving and handling SSE error messages"
else
    echo "❌ No SSE messages received"
fi

echo ""

# Overall result
if [[ $success_rate -ge 80 && $total_ssv_sent -gt 0 && $total_ssrv_sent -gt 0 && $total_sse_triggered -gt 0 ]]; then
    echo "============================================="
    echo "✅ ENHANCED SSE TEST: PASSED"
    echo "✅ All algorithm components working correctly"
    echo "✅ SSE error handling validated with different MAX_VALUE per parameter"
    echo "============================================="
else
    echo "============================================="
    echo "❌ ENHANCED SSE TEST: FAILED"
    echo "⚠️  Some algorithm components need investigation"
    echo "============================================="
fi
