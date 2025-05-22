extends TileMapLayer

# === Variables ===
var map: TileMapLayer
var unit_creator: Node
var battle_script: Node
var selected_army: army

#Assuming only matching player_id armies can stand on same tile, will change later
var army_locations: Dictionary[Vector2i, Array] = {} #Array[army]
var attacking_army_locations: Dictionary[Vector2i, Array] = {} #Array[army]
var army_data: Dictionary[int, army] #Army id -> Army

var armies_to_kill: Dictionary[int, bool] = {}
var armies_to_retreat: Dictionary[int, bool] = {}

# === Built-ins ===
func _ready() -> void:
	battle_script = load("res://Units/unit_managers/battle_script.gd").new(self)
	unit_creator = load("res://Units/unit_managers/unit_creator.gd").new()
	map = get_parent()
	Utils.assign_unit_map(self)

func _input(event: InputEvent) -> void:
	if event.is_action_pressed("deselect") and state_machine.is_selecting_unit():
		set_army_route(selected_army.get_army_id(), map.get_cell_position())
		map.update_info_window(selected_army.get_army_client_array())

# === Networking ===
func send_data_to_clients() -> void:
	map.refresh_unit_map.rpc(get_used_cells_dictionary())

@rpc("any_peer", "call_remote", "unreliable")
func request_refresh_map() -> void:
	map.refresh_unit_map.rpc_id(multiplayer.get_remote_sender_id(), get_used_cells_dictionary())

@rpc("authority", "call_remote", "reliable")
func refresh_map(_visible_tiles: Array, _unit_atlas: Dictionary) -> void:
	pass

@rpc("any_peer", "call_remote", "unreliable")
func request_refresh(tile: Vector2i) -> void:
	var sender_id: int = multiplayer.get_remote_sender_id()
	if army_locations.has(tile):
		refresh_all_armies(army_locations[tile], sender_id)

func refresh_all_armies(armies: Array[int], sender_id: int) -> void:
	for army_id: int in armies:
		var army_obj: army = army_data[army_id]
		refresh_army.rpc_id(sender_id, army_obj.get_army_id(), army_obj.get_army_client_array())

@rpc("authority", "call_local", "unreliable")
func refresh_army(army_id: int, info_array: Array) -> void:
	var node: Node = get_control_node(army_id)
	var morale_bar: ProgressBar = node.get_node("MoraleBar")
	morale_bar.value = info_array[1]

	var manpower_label: RichTextLabel = node.get_node("Manpower_Label")
	manpower_label.text = "[center]" + str(info_array[0]) + "[/center]"

# === Utility ===
func get_used_cells_dictionary() -> Dictionary:
	var to_return: Dictionary = {}
	for tile: Vector2i in get_used_cells():
		to_return[tile] = get_cell_atlas_coords(tile)
	return to_return

# === Unit Checks ===
func get_army(army_id: int) -> army:
	return army_data[army_id]

func get_top_army(tile: Vector2i) -> army:
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

# === Creating Units ===
@rpc("any_peer", "call_local", "unreliable")
func check_before_create(coords: Vector2i, type: int, player_id: int) -> void:
	if type == -1:
		return
	var unit_class: GDScript = get_unit_class(type)
	var cost: int = unit_class.get_cost()
	var money_cntrl: money_controller = money_controller.get_instance()
	
	if !army_locations.has(coords) and money_cntrl.player_has_enough_money(player_id, cost):
		money_cntrl.remove_money_from_player(player_id, cost)
		create_army.rpc(coords, type, player_id)

@rpc("authority", "call_local", "unreliable")
func create_army(coords: Vector2i, type: int, player_id: int) -> void:
	if !army_locations.has(coords):
		army_locations[coords] = []
		set_cell(coords, 0, Vector2i(0, type))

	var unit_class: GDScript = get_unit_class(type)
	var new_army: army = army.new(player_id, coords)
	army_locations[coords].append(new_army.army_id)
	army_data[new_army.army_id] = new_army
	new_army.add_unit(unit_class.new())

	create_label(new_army.get_army_id(), coords, str(new_army))
	refresh_army(new_army.get_army_id(), new_army.get_army_client_array())

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

# Moving Units
@rpc("any_peer", "call_local", "unreliable")
func set_army_route(army_id: int, move_to: Vector2i) -> void:
	var army_obj: army = get_army(army_id)
	if army_obj != null:
		set_army_route_locally(army_id, army_obj.get_location(), move_to)

#Coords here for desync checking
func set_army_route_locally(army_id: int, _coords: Vector2i, move_to: Vector2i) -> void:
	var army_obj: army = get_army(army_id)
	army_obj.set_route(bfs_to_destination(army_obj, move_to))
	if army_obj == selected_army and army_obj.get_player_id() == multiplayer.get_unique_id():
		$dest_sound.play(0.3)
		highlight_dest()

func get_selected_coords() -> Vector2i:
	return selected_army.get_location()

# Movement Logic
func check_move(army_obj: army) -> void:
	if army_obj.has_destination():
		var coords: Vector2i = army_obj.get_location()
		var move_to: Vector2i = army_obj.pop_next_location()
		if !location_is_attack(move_to, army_obj.get_player_id()):
			move_army.rpc(army_obj.get_army_id(), coords, move_to)

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

func get_control_node(army_id: int) -> Control:
	return get_node("Control_" + str(army_id))

func move_control(army_id: int, move_to: Vector2i) -> void:
	var node: Control = get_control_node(army_id)
	node.position = map_to_local(move_to)

# Availability Checking
func location_is_attack(coords_of_defender: Vector2i, player_id: int) -> bool:
	return tile_has_enemy_army(coords_of_defender, player_id)

func bfs_to_destination(army_obj: army, destination: Vector2i) -> Array[Vector2i]:
	var current: Vector2i
	var queue: priority_queue = priority_queue.new()
	var tile_to_prev: Dictionary = {}
	var visited: Dictionary = {}
	var found: bool = false
	queue.insert_element(army_obj.get_location(), 0)
	visited[army_obj.get_location()] = 0
	while !queue.is_empty():
		current = queue.pop_back()
		if current == destination:
			found = true
			break
		for tile: Vector2i in map.get_surrounding_cells(current):
			var terrain_mult: int = map.get_terrain_speed_mult(tile)
			var temp_dist: float =  map.map_to_local(current).distance_to(map.map_to_local(tile))
			var current_dist: float = visited[current] + (temp_dist / terrain_mult)
			if map.is_tile_traversable(tile) and !visited.has(tile):
				queue.insert_element(tile, current_dist)
				visited[tile] = current_dist
				tile_to_prev[tile] = current
	if found:
		return create_route_from_tile_to_prev(army_obj.get_location(), destination, tile_to_prev)
	else:
		return []

func create_route_from_tile_to_prev(start: Vector2i, destination: Vector2i, tile_to_prev: Dictionary) -> Array[Vector2i]:
	var current: Vector2i = destination
	var route: Array[Vector2i] = []
	while current != start:
		route.push_front(current)
		current = tile_to_prev[current]
	return route

func unit_battle(attacker: army, defender: army) -> void:
	var result: int = battle_script.unit_battle(attacker, defender)
	if result == 0:
		armies_to_kill[defender.get_army_id()] = true
	elif result == 1:
		armies_to_retreat[defender.get_army_id()] = true
	elif result == 2:
		armies_to_kill[attacker.get_army_id()] = true
	elif result == 3:
		armies_to_retreat[attacker.get_army_id()] = true

func get_selected_army() -> army:
	return selected_army

func army_is_owned(army_obj: army) -> bool:
	return army_obj.get_player_id() == multiplayer.get_unique_id()

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

func highlight_dest() -> void:
	if selected_army != null:
		if selected_army.get_destination() != null and army_is_owned(selected_army):
			map.highlight_cell(selected_army.get_destination())
		else:
			map.clear_highlights()
	else:
		map.clear_highlights()

func _process(delta: float) -> void:
	for location: Vector2i in army_locations:
		var army_stack: Array = army_locations[location]
		for army_id: int in army_stack:
			var army_obj: army = army_data[army_id]
			if armies_to_kill.has(army_id) or armies_to_retreat.has(army_id):
				continue
			update_army_progress(army_obj, delta)
			
	retreat_units()
	clean_up_killed_units()

func update_army_progress(army_obj: army, delta: float) -> void:
	army_obj.update_progress(delta)
	var location: Vector2i = army_obj.get_location()
	var next_location: Vector2i = army_obj.get_next_location()
	if next_location != location:
		var terrain_mult: int = map.get_terrain_speed_mult(next_location)
		if army_obj.ready_to_move(100.0 / army_obj.get_speed() / terrain_mult):
			check_move(army_obj)
			prepare_refresh_army(army_obj)

func prepare_refresh_army(army_obj: army) -> void:
	var info_array: Array = army_obj.get_army_client_array()
	refresh_army.rpc(army_obj.get_army_id(), info_array)

func retreat_units() -> void:
	for army_id: int in armies_to_retreat:
		var army_obj: army = get_army(army_id)
		var location: Vector2i = army_obj.get_location()
		var player_id: int = army_obj.get_player_id()
		var destination: Vector2i = find_surrounding_open_tile(location, player_id)
		if destination != location:
			set_army_route(army_id, destination)
		else:
			armies_to_kill[army_id] = true
	armies_to_retreat.clear()

func find_surrounding_open_tile(coords: Vector2i, player_id: int) -> Vector2i:
	for cell: Vector2i in map.thread_get_surrounding_cells(coords):
		if map.is_tile_traversable(cell) and not tile_has_enemy_army(cell, player_id):
			return cell
	return coords

func clean_up_killed_units() -> void:
	for army_id: int in armies_to_kill:
		kill_army.rpc(army_id)
	armies_to_kill.clear()

@rpc("authority", "call_local", "unreliable")
func kill_army(army_id: int, coords: Vector2i) -> void:
	var node: Control = get_control_node(army_id)
	army_data.erase(army_id)
	check_and_clean_army(army_id, coords)
	chech_and_clean_attacking_army(army_id, coords)
	clean_up_node(node)
	
	if tile_has_no_army(coords):
		if tile_has_attacking_army(coords):
			move_attacking_armies_to_normal(coords)
		else:
			erase_cell(coords)

func move_attacking_armies_to_normal(coords: Vector2i) -> void:
	for army_id: int in attacking_army_locations[coords]:
		army_locations[coords].append(army_id)
	attacking_army_locations[coords].clear()

func check_and_clean_army(army_id: int, coords: Vector2i) -> void:
	#Cleans regular armies
	var army_stack: Array = army_locations[coords]
	for index: int in army_stack.size():
		if army_stack[index] == army_id:
			army_stack.remove_at(index)
			break

func chech_and_clean_attacking_army(army_id: int, coords: Vector2i) -> void:
	#Cleans attacking armies
	var army_stack: Array = attacking_army_locations[coords]
	for index: int in army_stack.size():
		if army_stack[index] == army_id:
			army_stack.remove_at(index)
			break

func clean_up_node(node: Node) -> void:
	for child: Node in node.get_children():
		child.queue_free()
	node.queue_free()

func _on_regen_timer_timeout() -> void:
	for army_obj: army in army_data.values():
		regen_tick(army_obj)

func regen_tick(army_obj: army) -> void:
	var multiple: int = 1
	army_obj.add_experience(multiple)
	manpower_and_morale_tick(army_obj)
	army_obj.use_supplies()
	refresh_army.rpc(army_obj.get_army_id(), army_obj.get_army_client_array())

func manpower_and_morale_tick(army_obj: army) -> void:
	#Works on each unit individually since they all regen differently
	var player_id: int = army_obj.get_player_id()
	for unit: base_unit in army_obj.get_units():
		var amount: int = round(float(unit.get_max_manpower()) / 80.0 + 12)
		var max_cost: int = round(amount * 0.3)
		var money_cntrl: money_controller = money_controller.get_instance()
		if money_cntrl.player_has_enough_money(player_id, max_cost):
			@warning_ignore("narrowing_conversion")
			var manpower_used: int = ceil(unit.add_manpower(amount / 2.0))
			var cost: int = round(manpower_used * 0.3)
			money_cntrl.remove_money_from_player(player_id, cost)
			unit.add_morale(5)
		else:
			unit.remove_morale(10)
