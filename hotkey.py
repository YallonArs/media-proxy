import keyboard
import socket
import os

SOCKET_PATH = "./hotkey-socket.s"

if os.path.exists(SOCKET_PATH):
	os.remove(SOCKET_PATH)

def on_triggered(s: socket.socket):
	print("python: hotkey detected")
	s.send(b"1")

with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as server:
	server.bind(SOCKET_PATH)
	server.listen(1)
	print(f"Server listening on {SOCKET_PATH}...")
	
	conn, addr = server.accept()
	with conn:
		print("Client connected!")
		keyboard.add_hotkey('ctrl+shift+a', lambda: on_triggered(conn))
		keyboard.wait()
