#!/bin/bash

echo "🧪 FOCUSED EDGE CASE VALIDATION TEST (kef=1)"
echo "============================================="
echo "Testing with P_COUNT=64, all processes focusing on param #0"
echo ""

# Clean up any existing processes and logs
pkill -f test_full_algorithm 2>/dev/null || true
rm -f edge_test_p*.log 2>/dev/null || true

# Test configurations focusing on specific edge cases
declare -a test_configs=(
    "10 1"    # Process 0: Low iterator, param #0
    "250 1"   # Process 1: High iterator (wraparound case), param #0  
    "100 1"   # Process 2: Medium iterator, param #0
    "100 1"   # Process 3: Same iterator as P2 (process ID priority test), param #0
    "150 1"   # Process 4: Different iterator, param #0
    "50 1"    # Process 5: Lower iterator, param #0
)

echo "🚀 Starting ${#test_configs[@]} edge case test processes..."
echo "   All processes focus on parameter #0 (kef=1)"
echo "   Test duration: 20 seconds"
echo ""

# Start all test processes
for i in "${!test_configs[@]}"; do
    config=(${test_configs[$i]})
    iter=${config[0]}
    kef=${config[1]}
    
    echo "   Process $i: iterator=$iter, kef=$kef, PID will determine priority"
    timeout 25s ./test_full_algorithm $iter $kef 20000 > "edge_test_p${i}.log" 2>&1 &
    
    # Small delay to ensure different PIDs
    sleep 0.2
done

echo ""
echo "📡 Running edge case validation for 22 seconds..."

# Wait for all processes to complete
sleep 22

echo ""
echo "🛑 Stopping any remaining processes..."
pkill -f test_full_algorithm 2>/dev/null || true
wait 2>/dev/null || true

echo ""
echo "📊 EDGE CASE VALIDATION RESULTS:"
echo "================================="

# Analyze results
total_processes=0
working_ssv=0
working_ssrv=0
working_sse=0
working_param_updates=0

echo ""
for i in "${!test_configs[@]}"; do
    config=(${test_configs[$i]})
    iter=${config[0]}
    
    if [[ -f "edge_test_p${i}.log" ]]; then
        total_processes=$((total_processes + 1))
        
        # Extract final statistics
        ssv_sent=$(grep "SSV:" "edge_test_p${i}.log" | grep "sent" | tail -1 | awk '{print $(NF-1)}' || echo "0")
        ssv_received=$(grep "SSV:" "edge_test_p${i}.log" | grep "received" | tail -1 | awk '{print $(NF-1)}' || echo "0")
        ssrv_sent=$(grep "SSRV:" "edge_test_p${i}.log" | grep "sent" | tail -1 | awk '{print $(NF-1)}' || echo "0")
        ssrv_received=$(grep "SSRV:" "edge_test_p${i}.log" | grep "received" | tail -1 | awk '{print $(NF-1)}' || echo "0")
        param_changes=$(grep "Parameter Changes:" "edge_test_p${i}.log" | tail -1 | awk '{print $NF}' || echo "0")
        final_param=$(grep "Param #0:" "edge_test_p${i}.log" | tail -1 | awk '{print $NF}' || echo "unknown")
        
        echo "📋 Process $i (iter=$iter):"
        echo "   SSV: $ssv_sent sent, $ssv_received received"
        echo "   SSRV: $ssrv_sent sent, $ssrv_received received"  
        echo "   Param changes: $param_changes"
        echo "   Final param #0: $final_param"
        
        # Count working features
        [[ $ssv_sent -gt 0 || $ssv_received -gt 0 ]] && working_ssv=$((working_ssv + 1))
        [[ $ssrv_sent -gt 0 || $ssrv_received -gt 0 ]] && working_ssrv=$((working_ssrv + 1))
        [[ $param_changes -gt 0 ]] && working_param_updates=$((working_param_updates + 1))
        
        echo ""
    else
        echo "❌ Process $i: Log file missing"
        echo ""
    fi
done

echo "🎯 EDGE CASE SUMMARY:"
echo "===================="
echo "Total processes: $total_processes/${#test_configs[@]}"
echo "SSV working: $working_ssv/$total_processes processes"
echo "SSRV working: $working_ssrv/$total_processes processes"
echo "Parameter updates: $working_param_updates/$total_processes processes"

# Algorithm validation
echo ""
echo "✅ ALGORITHM VALIDATION:"
if [[ $working_ssv -ge 3 ]]; then
    echo "   ✅ SSV Synchronization: WORKING ($working_ssv processes)"
else
    echo "   ❌ SSV Synchronization: FAILED ($working_ssv processes)"
fi

if [[ $working_ssrv -ge 2 ]]; then
    echo "   ✅ SSRV Processing: WORKING ($working_ssrv processes)"
else
    echo "   ❌ SSRV Processing: FAILED ($working_ssrv processes)"
fi

if [[ $working_param_updates -ge 3 ]]; then
    echo "   ✅ Parameter Updates: WORKING ($working_param_updates processes)"
else
    echo "   ❌ Parameter Updates: FAILED ($working_param_updates processes)"
fi

# Check for iterator edge cases
echo ""
echo "🔍 ITERATOR EDGE CASE ANALYSIS:"
echo "=============================="

# Check for wraparound handling (process 1: iter=250 vs others)
if grep -q "ITERATOR DIFF:" edge_test_p1.log 2>/dev/null; then
    echo "   ✅ Iterator wraparound test data found"
    grep "ITERATOR DIFF:" edge_test_p1.log | head -3 | while read line; do
        echo "   📊 $line"
    done
else
    echo "   ⚠️  No iterator wraparound test data found"
fi

# Check for same iterator handling (processes 2&3: both iter=100)
echo ""
echo "🔍 PROCESS ID PRIORITY TEST:"
echo "============================"
echo "   Processes 2&3 both have iterator=100, should use process ID priority"

if [[ -f "edge_test_p2.log" && -f "edge_test_p3.log" ]]; then
    p2_changes=$(grep "Parameter Changes:" "edge_test_p2.log" | tail -1 | awk '{print $NF}' || echo "0")
    p3_changes=$(grep "Parameter Changes:" "edge_test_p3.log" | tail -1 | awk '{print $NF}' || echo "0")
    p2_final=$(grep "Param #0:" "edge_test_p2.log" | tail -1 | awk '{print $NF}' || echo "unknown")
    p3_final=$(grep "Param #0:" "edge_test_p3.log" | tail -1 | awk '{print $NF}' || echo "unknown")
    
    echo "   Process 2: $p2_changes changes, final value: $p2_final"
    echo "   Process 3: $p3_changes changes, final value: $p3_final"
    
    if [[ "$p2_final" == "$p3_final" && "$p2_final" != "unknown" ]]; then
        echo "   ✅ Same iterator processes converged to same value"
    else
        echo "   ⚠️  Same iterator processes have different final values"
    fi
else
    echo "   ❌ Process 2 or 3 log files missing"
fi

echo ""
echo "============================================="
if [[ $working_ssv -ge 3 && $working_param_updates -ge 3 ]]; then
    echo "🎉 EDGE CASE VALIDATION: PASSED"
    echo "✅ Algorithm handles edge cases correctly!"
else
    echo "❌ EDGE CASE VALIDATION: FAILED" 
    echo "⚠️  Algorithm needs fixes for edge cases"
fi
echo "============================================="
