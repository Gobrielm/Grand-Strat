extends Window
var location = null
var depot_name: String
var current_trains: Array
var selected_index: int = -1
@onready var train_list: ItemList = $Train_Node/Train_List
const time_every_update = 1
var progress: float = 0.0

var map: TileMapLayer
# Called when the node enters the scene tree for the first time.
func _ready():
	hide()
	map = get_parent()

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	progress += delta
	if progress > time_every_update:
		refresh_window()

func _on_close_requested():
	hide()

func open_window(new_location: Vector2i):
	location = new_location
	refresh_window()
	popup()

func refresh_window():
	progress = 0
	if location != null:
		if train_list.get_selected_items().size() > 0:
			selected_index = train_list.get_selected_items()[0]
		else:
			selected_index = -1
		request_current_trains.rpc_id(1, location)
		request_current_name.rpc_id(1, location)
		depot_window()

@rpc("any_peer", "call_local", "unreliable")
func request_current_name(coords: Vector2i):
	var current_name = map_data.get_instance().get_depot_name(coords)
	update_current_name.rpc_id(multiplayer.get_remote_sender_id(), current_name)

@rpc("any_peer", "call_local", "unreliable")
func request_current_trains(coords: Vector2i):
	var trains = map.get_trains_in_depot(coords)
	update_current_trains.rpc_id(multiplayer.get_remote_sender_id(), trains)

@rpc("authority", "call_local", "unreliable")
func update_current_trains(new_trains: Array):
	current_trains = new_trains

@rpc("authority", "call_local", "unreliable")
func update_current_name(new_name: String):
	depot_name = new_name

func depot_window():
	$Name.text = "[center][font_size=30]" + depot_name + "[/font_size][/center]"
	for i in train_list.item_count:
		train_list.remove_item(0)
	for train_obj in current_trains.size():
		var train_name: String = str(current_trains[train_obj])
		train_list.add_item(train_name)
	if train_list.item_count > selected_index and selected_index != -1:
		train_list.select(selected_index)

func _on_go_button_pressed():
	if selected_index != -1:
		depart_train.rpc_id(1, selected_index, location)
		train_list.remove_item(selected_index)

@rpc("any_peer", "call_local", "unreliable")
func depart_train(index: int, new_location: Vector2i):
	var depot = map.get_depot_or_terminal(new_location)
	depot.leave_depot(index)
