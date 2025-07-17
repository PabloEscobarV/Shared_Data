#!/bin/bash

echo "🧪 Quick Multi-Process Algorithm Test"
echo "======================================="

# Clean up any existing processes
pkill -f test_full_algorithm 2>/dev/null || true

echo "🚀 Starting 2 test processes..."

# Start first process in background
./test_full_algorithm 50 1 10000 > process1.log 2>&1 &
PID1=$!
echo "   Process 1 started (PID: $PID1)"

# Small delay
sleep 1

# Start second process in background  
./test_full_algorithm 100 2 10000 > process2.log 2>&1 &
PID2=$!
echo "   Process 2 started (PID: $PID2)"

echo ""
echo "📡 Running test for 12 seconds..."

# Wait for completion or timeout
sleep 12

echo ""
echo "🛑 Stopping processes..."
kill $PID1 $PID2 2>/dev/null || true
wait 2>/dev/null || true

echo ""
echo "📊 Results Summary:"
echo "=================="

echo ""
echo "📝 Process 1 Output:"
echo "-------------------"
tail -20 process1.log 2>/dev/null || echo "No output from process 1"

echo ""
echo "📝 Process 2 Output:"
echo "-------------------"
tail -20 process2.log 2>/dev/null || echo "No output from process 2"

echo ""
echo "🔍 Message Exchange Analysis:"
echo "=============================="

# Count messages
SSV_COUNT=$(grep -c "SSV" process*.log 2>/dev/null || echo "0")
SSRV_COUNT=$(grep -c "SSRV" process*.log 2>/dev/null || echo "0")
SSE_COUNT=$(grep -c "SSE" process*.log 2>/dev/null || echo "0")
PARAM_COUNT=$(grep -c "PARAM_CHANGE" process*.log 2>/dev/null || echo "0")

echo "   SSV messages: $SSV_COUNT"
echo "   SSRV messages: $SSRV_COUNT"
echo "   SSE messages: $SSE_COUNT"
echo "   Parameter changes: $PARAM_COUNT"

# Cleanup
rm -f process1.log process2.log

if [ $PARAM_COUNT -gt 0 ]; then
    echo ""
    echo "✅ SUCCESS: Algorithm working - parameter changes detected!"
else
    echo ""
    echo "⚠️  Limited success: Processes running but minimal interaction"
fi
