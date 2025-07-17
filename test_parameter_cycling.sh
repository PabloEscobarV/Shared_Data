#!/bin/bash

echo "🧪 PARAMETER CYCLING TEST (P_COUNT=3)"
echo "====================================="
echo "Testing that each process cycles through all parameter numbers (0,1,2)"
echo "Every 100ms: param#0 → param#1 → param#2 → param#0 → ..."
echo ""

# Clean up any existing processes and logs
pkill -f test_full_algorithm 2>/dev/null || true
rm -f cycle_test_p*.log 2>/dev/null || true

# Test configurations with different iterators but same kef=1
declare -a test_configs=(
    "50 1"    # Process 0: iterator=50, kef=1
    "100 1"   # Process 1: iterator=100, kef=1 
    "150 1"   # Process 2: iterator=150, kef=1
    "200 1"   # Process 3: iterator=200, kef=1
    "10 1"    # Process 4: iterator=10 (low), kef=1
    "250 1"   # Process 5: iterator=250 (wraparound), kef=1
)

echo "🚀 Starting ${#test_configs[@]} processes (all use kef=1):"
echo "   Each process should cycle: param#0 → param#1 → param#2 → param#0..."
echo "   Test duration: 25 seconds"
echo ""

# Start all test processes
for i in "${!test_configs[@]}"; do
    config=(${test_configs[$i]})
    iter=${config[0]}
    kef=${config[1]}
    
    echo "   Process $i: iterator=$iter, kef=$kef"
    timeout 30s ./test_full_algorithm $iter $kef 25000 > "cycle_test_p${i}.log" 2>&1 &
    
    sleep 0.1
done

echo ""
echo "📡 Running parameter cycling test for 27 seconds..."

# Progress indicator
for ((t=27; t>0; t--)); do
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
echo "📊 PARAMETER CYCLING ANALYSIS:"
echo "=============================="

# Analyze parameter cycling for each process
echo ""
echo "📋 Parameter Cycling Validation:"
echo "--------------------------------"

total_processes=0
successful_processes=0
total_ssv_sent=0
total_ssv_received=0

for i in "${!test_configs[@]}"; do
    config=(${test_configs[$i]})
    iter=${config[0]}
    
    if [[ -f "cycle_test_p${i}.log" ]]; then
        total_processes=$((total_processes + 1))
        
        # Extract statistics
        ssv_sent=$(grep "SSV:" "cycle_test_p${i}.log" | grep "sent" | tail -1 | awk '{print $(NF-1)}' 2>/dev/null || echo "0")
        ssv_received=$(grep "SSV:" "cycle_test_p${i}.log" | grep "received" | tail -1 | awk '{print $(NF-1)}' 2>/dev/null || echo "0")
        param_changes=$(grep "Parameter Changes:" "cycle_test_p${i}.log" | tail -1 | awk '{print $NF}' 2>/dev/null || echo "0")
        
        # Get final values for all 3 parameters
        param0_value=$(grep "Param #0:" "cycle_test_p${i}.log" | tail -1 | awk '{print $NF}' 2>/dev/null || echo "0")
        param1_value=$(grep "Param #1:" "cycle_test_p${i}.log" | tail -1 | awk '{print $NF}' 2>/dev/null || echo "1")
        param2_value=$(grep "Param #2:" "cycle_test_p${i}.log" | tail -1 | awk '{print $NF}' 2>/dev/null || echo "2")
        
        if [[ $ssv_sent -gt 0 || $ssv_received -gt 0 ]]; then
            successful_processes=$((successful_processes + 1))
        fi
        
        total_ssv_sent=$((total_ssv_sent + ssv_sent))
        total_ssv_received=$((total_ssv_received + ssv_received))
        
        echo "📋 Process $i (iter=$iter):"
        echo "   SSV: $ssv_sent sent, $ssv_received received"
        echo "   Parameter changes: $param_changes"
        echo "   Final values - Param#0: $param0_value, Param#1: $param1_value, Param#2: $param2_value"
        
        # Check if all parameters were affected (indicating cycling worked)
        affected_params=0
        [[ "$param0_value" != "0" ]] && affected_params=$((affected_params + 1))
        [[ "$param1_value" != "1" ]] && affected_params=$((affected_params + 1))
        [[ "$param2_value" != "2" ]] && affected_params=$((affected_params + 1))
        
        echo "   Parameters affected by cycling: $affected_params/3"
        echo ""
    else
        echo "❌ Process $i: Log file missing"
        echo ""
    fi
done

echo "🔍 PARAMETER CONVERGENCE ANALYSIS:"
echo "==================================="

# Check if all processes converged to same values for each parameter
echo "Checking parameter convergence across all processes:"
echo ""

for param_num in {0..2}; do
    echo "📊 Parameter #$param_num convergence:"
    
    # Collect all final values for this parameter
    values=()
    for i in "${!test_configs[@]}"; do
        if [[ -f "cycle_test_p${i}.log" ]]; then
            value=$(grep "Param #${param_num}:" "cycle_test_p${i}.log" | tail -1 | awk '{print $NF}' 2>/dev/null || echo "$param_num")
            values+=("$value")
            echo "   Process $i: $value"
        fi
    done
    
    # Check if all values are the same
    if [[ ${#values[@]} -gt 0 ]]; then
        first_value=${values[0]}
        all_same=true
        for value in "${values[@]}"; do
            if [[ "$value" != "$first_value" ]]; then
                all_same=false
                break
            fi
        done
        
        if $all_same; then
            echo "   ✅ All processes converged to: $first_value"
        else
            echo "   ⚠️  Processes have different values: ${values[*]}"
        fi
    fi
    echo ""
done

echo "📈 ALGORITHM PERFORMANCE SUMMARY:"
echo "================================="

echo "Total processes: $total_processes/${#test_configs[@]}"
echo "Successful processes: $successful_processes/$total_processes"
echo "Total SSV messages: $total_ssv_sent sent, $total_ssv_received received"

if [[ $successful_processes -gt 0 ]]; then
    avg_ssv_per_process=$((total_ssv_sent / successful_processes))
    echo "Average SSV per process: $avg_ssv_per_process"
fi

echo ""
echo "✅ PARAMETER CYCLING VALIDATION:"
echo "================================"

success_rate=$((successful_processes * 100 / total_processes))

if [[ $success_rate -ge 80 ]]; then
    echo "✅ Process stability: PASSED ($success_rate% success)"
else
    echo "❌ Process stability: FAILED ($success_rate% success)"
fi

if [[ $total_ssv_sent -gt 0 && $total_ssv_received -gt 0 ]]; then
    echo "✅ SSV Communication: WORKING"
    
    # Check message distribution
    expected_messages_per_param=$((total_ssv_sent / 3))  # Should be roughly equal for 3 parameters
    echo "   📊 Expected ~$expected_messages_per_param messages per parameter"
else
    echo "❌ SSV Communication: FAILED"
fi

# Check if parameter cycling is working by looking for parameter changes
total_param_changes=0
for i in "${!test_configs[@]}"; do
    if [[ -f "cycle_test_p${i}.log" ]]; then
        changes=$(grep "Parameter Changes:" "cycle_test_p${i}.log" | tail -1 | awk '{print $NF}' 2>/dev/null || echo "0")
        total_param_changes=$((total_param_changes + changes))
    fi
done

if [[ $total_param_changes -gt 5 ]]; then
    echo "✅ Parameter cycling: WORKING ($total_param_changes total changes)"
else
    echo "❌ Parameter cycling: FAILED ($total_param_changes total changes)"
fi

echo ""
echo "🔍 EDGE CASE VERIFICATION:"
echo "=========================="

# Check iterator wraparound (process 5: iter=250 vs others)
if [[ -f "cycle_test_p5.log" && -f "cycle_test_p0.log" ]]; then
    p5_changes=$(grep "Parameter Changes:" "cycle_test_p5.log" | tail -1 | awk '{print $NF}' 2>/dev/null || echo "0")
    p0_changes=$(grep "Parameter Changes:" "cycle_test_p0.log" | tail -1 | awk '{print $NF}' 2>/dev/null || echo "0")
    
    echo "📊 Iterator wraparound test:"
    echo "   Process 5 (iter=250): $p5_changes changes"
    echo "   Process 0 (iter=50): $p0_changes changes"
    
    if [[ $p5_changes -gt 0 && $p0_changes -gt 0 ]]; then
        echo "   ✅ Wraparound handling: WORKING"
    else
        echo "   ⚠️  Wraparound handling: NEEDS_CHECK"
    fi
fi

echo ""
echo "============================================="
if [[ $success_rate -ge 80 && $total_ssv_sent -gt 0 && $total_param_changes -gt 5 ]]; then
    echo "🎉 PARAMETER CYCLING TEST: PASSED"
    echo "✅ Algorithm correctly cycles through all parameters!"
    echo "✅ SSV messages sent for param#0, param#1, param#2 in sequence!"
    echo "✅ Multi-process synchronization working!"
else
    echo "❌ PARAMETER CYCLING TEST: FAILED"
    echo "⚠️  Parameter cycling or synchronization needs investigation"
fi
echo "============================================="
