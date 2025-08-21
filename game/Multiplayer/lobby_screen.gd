extends Control
@onready var lobby: Node = get_parent()
var started: bool = false
var target_progress: int = 0
var incrementage: float = 0

func _process(delta: float) -> void:
	if ($ColorRect/ProgressBar.value < target_progress):
		$ColorRect/ProgressBar.value += incrementage

func _on_button_pressed() -> void:
	if (!started):
		lobby.rpc("start_game")
	started = true

func delete_children() -> void:
	for child: Node in get_children(true):
		for child2: Node in get_children(true):
			child2.queue_free()
		child.queue_free()

func update_progress_bar(amount: float) -> void:
	target_progress = amount
	incrementage = (target_progress - $ColorRect/ProgressBar.value) / 100.0

func add_player(id: int) -> void:
	var player_list: ItemList = $ColorRect/PlayerList
	player_list.add_item(str(id), null, false)
	if id == 1:
		player_list.move_item(player_list.item_count - 1, 0)
		player_list.set_item_text(0, player_list.get_item_text(0) + "(host)")
	if multiplayer.get_unique_id() == 1:
		refresh_player_list.rpc(get_player_array())

func get_player_array() -> Array[int]:
	var player_list: ItemList = $ColorRect/PlayerList
	var toReturn: Array[int] = []
	for i: int in player_list.item_count:
		toReturn.push_back(int(player_list.get_item_text(i)))
	
	return toReturn

@rpc("authority", "call_remote", "reliable")
func refresh_player_list(players: Array[int]) -> void:
	var player_list: ItemList = $ColorRect/PlayerList
	for i: int in players.size():
		var player: int = players[i]
		if i < player_list.item_count and player_list.get_item_text(i) != str(player):
			add_player(player)
		elif i >= player_list.item_count:
			add_player(player)
