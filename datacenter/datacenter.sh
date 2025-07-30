#!/bin/bash
pkill -f ros_web_bridge.py 2>/dev/null
pkill -f "python3 -m http.server" 2>/dev/null
sleep 2
python3 ros_web_bridge.py >/dev/null 2>&1 &
python3 -m http.server 8000 >/dev/null 2>&1 &
sleep 2
echo "Dashboard: http://localhost:8000/dashboard.html"