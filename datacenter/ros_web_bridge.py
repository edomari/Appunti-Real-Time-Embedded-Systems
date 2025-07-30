import rclpy
from rclpy.node import Node
from std_msgs.msg import String
import asyncio
import websockets
import json
import threading
import subprocess
from datetime import datetime
import time

class DataCenterMonitorNode(Node):
    def __init__(self):
        super().__init__('ros_web_bridge')
        
        self.datacenter_sub = self.create_subscription(String, '/datacenter_status', self.datacenter_callback, 10)
        self.shared_state_sub = self.create_subscription(String, '/datacenter_technitian', self.shared_state_callback, 10)
        
        self.data = {
            'temp': None,
            'hum': None, 
            'motion': False, 
            'motion_attributed_to_tech': False,
            'technician_inside': False, 
            'pir_enabled': True, 
            'topics': {},
            'technician_entry_counter': 0,
        }
        
        self.websocket_clients = set()
        
        self.update_topics()
        self.create_timer(30.0, self.update_topics)
    
    def get_topics(self):
        try:
            result = subprocess.run(['ros2', 'topic', 'list'], capture_output=True, text=True, timeout=10)
            if result.returncode != 0: return {}
            
            topics = {}
            for topic in result.stdout.strip().split('\n'):
                if topic.strip():
                    topics[topic.strip()] = {
                        'name': topic.strip()
                    }
            
            return topics
        except Exception:
            return {}
    
    def update_topics(self):
        self.data['topics'] = self.get_topics()
        asyncio.run(self.broadcast())
    
    def shared_state_callback(self, msg):
        try:
            data = json.loads(msg.data)
            
            if 'entry_counter' in data:
                self.data['technician_entry_counter'] = data['entry_counter']
            
            asyncio.run(self.broadcast())
            
        except Exception as e:
            self.get_logger().error(f'Errore nel parsing shared_state: {e}')
    
    def datacenter_callback(self, msg):
        try:
            data = json.loads(msg.data)
                        
            if 'temp' in data and data['temp'] is not None:
                self.data['temp'] = float(data['temp'])
            
            if 'hum' in data and data['hum'] is not None:
                self.data['hum'] = float(data['hum'])
            
            if 'tech_inside' in data:
                self.data['technician_inside'] = data['tech_inside']
            
            if 'pir_enabled' in data:
                self.data['pir_enabled'] = data['pir_enabled']
            
            if 'motion' in data and data['motion'] is not None:
                self.data['motion'] = bool(data['motion'])
                
            if 'motion_attributed_to_tech' in data:
                self.data['motion_attributed_to_tech'] = bool(data['motion_attributed_to_tech'])
    
            asyncio.run(self.broadcast())
            
        except Exception as e:
            self.get_logger().error(f'Errore nel parsing datacenter: {e}')
      
    async def broadcast(self):
        if self.websocket_clients:
            message = json.dumps(self.data, default=str)
            disconnected = set()
            
            for client in self.websocket_clients:
                try:
                    await client.send(message)
                except:
                    disconnected.add(client)
            
            self.websocket_clients -= disconnected

class WebSocketServer:
    def __init__(self, ros_node):
        self.ros_node = ros_node
    
    async def handle_client(self, websocket):
        self.ros_node.websocket_clients.add(websocket)
        
        try:
            await websocket.send(json.dumps(self.ros_node.data, default=str))
            async for message in websocket:
                try:
                    command = json.loads(message)
                    if command.get('action') == 'refresh_topics':
                        self.ros_node.update_topics()
                except:
                    pass
        except:
            pass
        finally:
            self.ros_node.websocket_clients.discard(websocket)
    
    async def start_server(self):
        await websockets.serve(self.handle_client, 'localhost', 8765)

def run_websocket_server(ros_node):
    server = WebSocketServer(ros_node)
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    
    try:
        loop.run_until_complete(server.start_server())
        loop.run_forever()
    except KeyboardInterrupt:
        pass

def main():
    rclpy.init()
    monitor_node = DataCenterMonitorNode()
    
    websocket_thread = threading.Thread(target=run_websocket_server, args=(monitor_node,))
    websocket_thread.daemon = True
    websocket_thread.start()
    
    try:
        rclpy.spin(monitor_node)
    except KeyboardInterrupt:
        pass
    finally:
        monitor_node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()