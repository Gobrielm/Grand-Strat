extends Window
var location: Vector2i
var opened: bool = false
var depot_name: String
var current_trains: Array
var selected_index: int = -1
@onready var train_list: ItemList = $Train_Node/Train_List
const time_every_update: int = 1
var progress: float = 0.0

var map: TileMapLayer
# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	hide()
	map = get_parent()

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	progress += delta
	if progress > time_every_update:
		refresh_window()

func _on_close_requested() -> void:
	opened = false
	hide()

func open_window(new_location: Vector2i) -> void:
	location = new_location
	opened = true
	refresh_window()
	popup()

func refresh_window() -> void:
	progress = 0
	if opened:
		if train_list.get_selected_items().size() > 0:
			selected_index = train_list.get_selected_items()[0]
		else:
			selected_index = -1
		request_current_trains.rpc_id(1, location)
		request_current_name.rpc_id(1, location)
		depot_window()

@rpc("any_peer", "call_local", "unreliable")
func request_current_name(coords: Vector2i) -> void:
	var current_name: String = map_data.get_instance().get_depot_name(coords)
	update_current_name.rpc_id(multiplayer.get_remote_sender_id(), current_name)

@rpc("any_peer", "call_local", "unreliable")
func request_current_trains(coords: Vector2i) -> void:
	var trains: Array = map.get_trains_in_depot(coords)
	update_current_trains.rpc_id(multiplayer.get_remote_sender_id(), trains)

@rpc("authority", "call_local", "unreliable")
func update_current_trains(new_trains: Array) -> void:
	current_trains = new_trains

@rpc("authority", "call_local", "unreliable")
func update_current_name(new_name: String) -> void:
	depot_name = new_name

func depot_window() -> void:
	$Name.text = "[center][font_size=30]" + depot_name + "[/font_size][/center]"
	for i: int in train_list.item_count:
		train_list.remove_item(0)
	for train_obj: int in current_trains.size():
		var train_name: String = str(current_trains[train_obj])
		train_list.add_item(train_name)
	if train_list.item_count > selected_index and selected_index != -1:
		train_list.select(selected_index)

func _on_go_button_pressed() -> void:
	if selected_index != -1:
		depart_train.rpc_id(1, selected_index, location)
		train_list.remove_item(selected_index)

@rpc("any_peer", "call_local", "unreliable")
func depart_train(index: int, new_location: Vector2i) -> void:
	var depot: terminal = map.get_depot_or_terminal(new_location)
	depot.leave_depot(index)
