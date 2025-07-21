extends Node

# Autoload named Lobby

# These signals can be connected to by a UI lobby scene or the game scene.
signal player_connected(peer_id: int, player_info: Dictionary)
signal player_disconnected(peer_id: int)
signal server_disconnected

const PORT: int = 7000
const DEFAULT_SERVER_IP: String = "10.100.0.236" # IPv4 localhost
const MAX_CONNECTIONS: int = 20

# This will contain player info for every player,
# with the keys being each player's unique IDs.
var players: Dictionary = {}

# This is the local player info. This should be modified locally
# before the connection is made. It will be passed to every other peer.
# For example, the value of "name" can be set to something the player
# entered in a UI scene.
var player_info: Dictionary = {"name": "Name"}

var players_loaded: int = 0

var player_lobby: Node = null
var is_host: bool = false

func _ready() -> void:
	multiplayer.peer_connected.connect(_on_player_connected)
	multiplayer.peer_disconnected.connect(_on_player_disconnected)
	multiplayer.connected_to_server.connect(_on_connected_ok)
	multiplayer.connection_failed.connect(_on_connected_fail)
	multiplayer.server_disconnected.connect(_on_server_disconnected)


func join_game(address: String = "") -> Error:
	if address.is_empty():
		address = DEFAULT_SERVER_IP
	var peer: ENetMultiplayerPeer = ENetMultiplayerPeer.new()
	var error: Error = peer.create_client(address, PORT)
	if error:
		return error
	multiplayer.multiplayer_peer = peer
	return OK

func create_game() -> Error:
	var peer: ENetMultiplayerPeer = ENetMultiplayerPeer.new()
	var error: Error = peer.create_server(PORT, MAX_CONNECTIONS)
	if error:
		return error
	multiplayer.multiplayer_peer = peer

	players[1] = player_info
	player_connected.emit(1, player_info)
	switch_player_from_menu_to_lobby()
	register_player_in_lobby(1)
	return OK

@rpc("authority", "reliable")
func switch_player_from_menu_to_lobby() -> void:
	var main_menu: Node = get_node("Main_Menu")
	remove_child(main_menu)
	main_menu.queue_free()
	player_lobby = preload("res://Multiplayer/lobby_screen.tscn").instantiate()
	add_child(player_lobby)

func register_player_in_lobby(id: int) -> void:
	player_lobby.add_player(id)

func remove_multiplayer_peer() -> void:
	multiplayer.multiplayer_peer = null

# When the server decides to start the game from a UI scene,
# do Lobby.load_game.rpc(filepath)
@rpc("call_local", "reliable")
func start_game() -> void:
	remove_child(player_lobby)
	player_lobby.queue_free()
	var map_node: CanvasGroup = load("res://Game/map_node.tscn").instantiate()
	add_child(map_node)

# Every peer will call this when they have loaded the game scene.
@rpc("any_peer", "call_local", "reliable")
func player_loaded() -> void:
	if multiplayer.is_server():
		players_loaded += 1
		if players_loaded == players.size():
			#$/root/game.start_game()
			players_loaded = 0


# When a peer connects, send them my player info.
# This allows transfer of all desired data for each player, not only the unique ID.
func _on_player_connected(id: int) -> void:
	_register_player.rpc_id(id, player_info)
	if multiplayer.get_unique_id() == 1:
		if id != 1:
			switch_player_from_menu_to_lobby.rpc_id(id)
		register_player_in_lobby(id)

@rpc("any_peer", "reliable")
func _register_player(new_player_info: Dictionary) -> void:
	var new_player_id: int = multiplayer.get_remote_sender_id()
	players[new_player_id] = new_player_info
	player_connected.emit(new_player_id, new_player_info)


func _on_player_disconnected(id: int) -> void:
	print("Peer disconnected")
	players.erase(id)
	player_disconnected.emit(id)


func _on_connected_ok() -> void:
	var peer_id: int = multiplayer.get_unique_id()
	players[peer_id] = player_info
	player_connected.emit(peer_id, player_info)


func _on_connected_fail() -> void:
	print("Peer's connection failed")
	multiplayer.multiplayer_peer = null


func _on_server_disconnected() -> void:
	print("Server disconnected")
	multiplayer.multiplayer_peer = null
	players.clear()
	server_disconnected.emit()
