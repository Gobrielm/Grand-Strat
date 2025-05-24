extends TileMapLayer

var unit_creator: Node
var selected_army: army
var map: TileMapLayer

var army_locations: Dictionary[Vector2i, Array] = {} #Array[army]
var attacking_army_locations: Dictionary[Vector2i, Array] = {} #Array[army]
var army_data: Dictionary[int, client_army] #Army id -> Army

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	unit_creator = preload("res://Units/unit_managers/army_creator.gd").new()
	map = get_parent() as TileMapLayer

func _input(event: InputEvent) -> void:
	if event.is_action_pressed("deselect"):
		if state_machine.is_selecting_unit():
			request_set_army_route.rpc_id(1, selected_army.get_army_id(), map.get_cell_position())
			map.update_info_window(selected_army.get_army_client_array())

@rpc("any_peer", "call_remote", "unreliable")
func request_refresh_map() -> void:
	pass

#TODO: Write full resync function
@rpc("authority", "call_remote", "reliable")
func refresh_map(_visible_tiles: Array, _unit_atlas: Dictionary) -> void:
	pass

@rpc("any_peer", "call_remote", "unreliable")
func request_refresh(_tile: Vector2i) -> void:
	pass

@rpc("authority", "call_local", "unreliable")
func refresh_army(army_id: int, info_array: Array, units_array: Array) -> void:
	get_army(army_id).update_stats(info_array, units_array)
	var node: Node = get_control_node(army_id)
	var morale_bar: ProgressBar = node.get_node("MoraleBar")
	morale_bar.value = info_array[1]

	var manpower_label: RichTextLabel = node.get_node("Manpower_Label")
	manpower_label.text = "[center]" + str(info_array[0]) + "[/center]"

# === Unit Checks ===
func get_army(army_id: int) -> client_army:
	if !army_data.has(army_id):
		assert(false, "Desync")
	return army_data[army_id]

func get_top_army(tile: Vector2i) -> client_army:
	return get_army(army_locations[tile][0])

#TODO: Change in general to also allow neutral armies
func tile_has_enemy_army(tile_to_check: Vector2i, player_id: int) -> bool:
	return !tile_has_no_army(tile_to_check) and get_top_army(tile_to_check).get_player_id() != player_id

func tile_has_no_army(tile_to_check: Vector2i) -> bool:
	return !army_locations.has(tile_to_check) or army_locations[tile_to_check].is_empty()

func tile_has_friendly_army(coords: Vector2i, player_id: int) -> bool:
	#TODO: Change to allow alliances
	return !tile_has_no_army(coords) and get_top_army(coords).get_player_id() == player_id

func tile_has_attacking_army(coords: Vector2i) -> bool:
	return attacking_army_locations.has(coords)

func get_control_node(army_id: int) -> Control:
	return get_node("Control_" + str(army_id))

@rpc("any_peer", "call_local", "unreliable")
func check_before_create(_coords: Vector2i, _type: int, _player_id: int) -> void:
	pass

@rpc("authority", "call_remote", "unreliable")
func create_army(coords: Vector2i, type: int, player_id: int, army_id: int) -> void:
	create_army_locally(coords, type, player_id, army_id)

func create_army_locally(coords: Vector2i, type: int, player_id: int, army_id: int) -> void:
	if !army_locations.has(coords):
		army_locations[coords] = []
		set_cell(coords, 0, Vector2i(0, 0))

	
	var new_army: client_army = client_army.new(player_id, coords, army_id)
	army_locations[coords].append(army_id)
	army_data[army_id] = new_army
	if type != -1:
		var unit_class: GDScript = get_unit_class(type)
		new_army.add_unit(unit_class.new())

	create_label(new_army.get_army_id(), coords, str(new_army))
	request_refresh.rpc_id(1, coords)

func get_unit_class(type: int) -> GDScript:
	return unit_creator.get_unit_class(type)

# === Label Creation ===
func create_label(army_id: int, coords: Vector2i, text: String) -> void:
	var label: Label = Label.new()
	label.name = "Label"

	var node: Control = Control.new()
	add_child(node)
	node.add_child(label)
	node.name = "Control_" + str(army_id)

	label.text = text
	move_control(army_id, coords)
	label.position = Vector2(-label.size.x / 2, label.size.y)

	var morale_bar: ProgressBar = create_morale_bar(label.size)
	node.add_child(morale_bar)

	var manpower_label: RichTextLabel = create_manpower_label(label.size)
	node.add_child(manpower_label)

func move_control(army_id: int, move_to: Vector2i) -> void:
	var node: Control = get_control_node(army_id)
	node.position = map_to_local(move_to)

func create_morale_bar(size: Vector2) -> ProgressBar:
	var morale_bar: ProgressBar = ProgressBar.new()
	morale_bar.name = "MoraleBar"
	morale_bar.show_percentage = false
	morale_bar.value = 100
	morale_bar.size = size
	morale_bar.position = Vector2(-size.x / 2, size.y * 2)

	var background_color: StyleBoxFlat = StyleBoxFlat.new()
	background_color.bg_color = Color(1, 1, 1, 0)

	var fill_color: StyleBoxFlat = StyleBoxFlat.new()
	fill_color.bg_color = Color(0.5, 1, 0.5, 1)

	morale_bar.add_theme_stylebox_override("background", background_color)
	morale_bar.add_theme_stylebox_override("fill", fill_color)

	return morale_bar

func create_manpower_label(size: Vector2) -> RichTextLabel:
	var manpower_label: RichTextLabel = RichTextLabel.new()
	manpower_label.name = "Manpower_Label"
	manpower_label.size = size
	manpower_label.bbcode_enabled = true
	manpower_label.text = "[center]0[/center]"
	manpower_label.position = Vector2(-size.x / 2, size.y * 2)
	return manpower_label

func select_army(coords: Vector2i, player_id: int) -> void:
	unhightlight_name()
	if selected_army != null and selected_army.get_location() == coords:
		cycle_army_selection(coords)
		$select_unit_sound.play(0.5)
		state_machine.click_unit()
	elif tile_has_friendly_army(coords, player_id):
		selected_army = get_top_army(coords)
		$select_unit_sound.play(0.5)
		state_machine.click_unit()
	else:
		selected_army = null
		state_machine.unclick_unit()
		map.close_unit_box()
	highlight_dest()
	highlight_name()


func cycle_army_selection(coords: Vector2i) -> void:
	var next_unit: bool = false
	var army_stack: Array = army_locations[coords]
	for i: int in army_stack.size():
		var army_id: int = army_stack[i]
		#Will select new army after looping once
		if next_unit:
			selected_army = get_army(army_id)
			next_unit = false
			break
		
		if army_id == selected_army.get_army_id():
			next_unit = true
	#On last army, cycle back to first
	if next_unit:
		selected_army = get_top_army(coords)

func highlight_name() -> void:
	apply_color_to_selected_unit(Color(1, 0, 0, 1))

func unhightlight_name() -> void:
	apply_color_to_selected_unit(Color(1, 1, 1, 1))

func apply_color_to_selected_unit(color: Color) -> void:
	if selected_army == null:
		return
	var node: Control = get_control_node(selected_army.get_army_id())
	var unit_name: Label = node.get_node("Label")
	unit_name.add_theme_color_override("font_color", color)

func highlight_cell(coords: Vector2i) -> void:
	map.highlight_cell(coords)

func highlight_dest() -> void:
	if selected_army != null:
		if selected_army.get_destination() != null and army_is_owned(selected_army):
			map.highlight_cell(selected_army.get_destination())
		else:
			map.clear_highlights()
	else:
		map.clear_highlights()

# Moving Armies
@rpc("any_peer", "call_local", "unreliable")
func request_set_army_route(army_id: int, move_to: Vector2i) -> void:
	var army_obj: army = get_army(army_id)
	if army_obj != null:
		set_army_route.rpc(army_id, move_to)

@rpc("authority", "call_local", "unreliable")
func set_army_route(army_id: int, move_to: Vector2i) -> void:
	var army_obj: army = get_army(army_id)
	set_army_route_locally(army_id, army_obj.get_location(), move_to)

#Coords here for desync checking
func set_army_route_locally(army_id: int, _coords: Vector2i, move_to: Vector2i) -> void:
	var army_obj: army = get_army(army_id)
	army_obj.set_route([move_to])
	if army_obj == selected_army and army_obj.get_player_id() == multiplayer.get_unique_id():
		$dest_sound.play(0.3)
		highlight_dest()

@rpc("authority", "call_local", "unreliable")
func move_army(army_id: int, coords: Vector2i, move_to: Vector2i) -> void:
	var army_obj: army = get_army(army_id)
	
	do_army_arrival(army_obj, move_to)
	do_army_leave(coords)
	move_control(army_id, move_to)

# Move Helpers
func do_army_arrival(army_obj: army, move_to: Vector2i) -> void:
	var unit_atlas: Vector2i = army_obj.get_atlas_coord()
	if get_cell_atlas_coords(move_to) == Vector2i(-1, -1):
		set_cell(move_to, 0, unit_atlas)
	#Gets rid of old army location
	var army_stack: Array = army_locations[army_obj.get_location()]
	for index: int in army_stack.size():
		var army_id: int = army_stack[index]
		if army_id == army_obj.get_army_id():
			army_stack.remove_at(index)
			break
	#Adds new location
	army_obj.set_location(move_to)
	if !army_locations.has(move_to):
		army_locations[move_to] = []
	army_locations[move_to].append(army_obj.get_army_id())

func do_army_leave(coords: Vector2i) -> void:
	if army_locations[coords].is_empty():
		erase_cell(coords)

func get_selected_army() -> army:
	return selected_army
	
func army_is_owned(army_obj: army) -> bool:
	return army_obj.get_player_id() == multiplayer.get_unique_id()

@rpc("authority", "call_local", "unreliable")
func kill_army(army_id: int, coords: Vector2i) -> void:
	var node: Control = get_control_node(army_id)
	army_data.erase(army_id)
	check_and_clean_army(army_id, coords)
	check_and_clean_attacking_army(army_id, coords)
	clean_up_node(node)
	
	if tile_has_no_army(coords):
		if tile_has_attacking_army(coords):
			move_attacking_armies_to_normal(coords)
		else:
			erase_cell(coords)

func clean_up_node(node: Node) -> void:
	for child: Node in node.get_children():
		child.queue_free()
	node.queue_free()

func check_and_clean_army(army_id: int, coords: Vector2i) -> void:
	#Cleans regular armies
	var army_stack: Array = army_locations[coords]
	for index: int in army_stack.size():
		if army_stack[index] == army_id:
			army_stack.remove_at(index)
			break

func check_and_clean_attacking_army(army_id: int, coords: Vector2i) -> void:
	#Cleans attacking armies
	var army_stack: Array = attacking_army_locations[coords]
	for index: int in army_stack.size():
		if army_stack[index] == army_id:
			army_stack.remove_at(index)
			break

func move_attacking_armies_to_normal(coords: Vector2i) -> void:
	for army_id: int in attacking_army_locations[coords]:
		army_locations[coords].append(army_id)
	attacking_army_locations[coords].clear()
