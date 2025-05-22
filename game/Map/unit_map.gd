extends TileMapLayer

# === Variables ===
var map: TileMapLayer
var unit_creator: Node
var battle_script: Node
var selected_army: army

#Assuming only matching player_id armies can stand on same tile, will change later
var army_locations: Dictionary[Vector2i, Array] = {} #Array[army]
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
		map.update_info_window(get_unit_client_array(get_selected_coords()))

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
		refresh_army.rpc_id(sender_id, army_obj.convert_to_client_array())

@rpc("authority", "call_local", "unreliable")
func refresh_army(info_array: Array) -> void:
	#PBUG: Should refresh whole army then update labels instead of individually
	for unit_array: Array in info_array:
		var coords: Vector2i = unit_array[1]
		var node: Node = get_node(str(coords))
		refresh_unit(unit_array, node)

func refresh_unit(info_array: Array, node: Node) -> void:
	var coords: Vector2i = info_array[1]
	if selected_army != null and coords == selected_army.get_location():
		map.update_info_window(info_array)

	var morale_bar: ProgressBar = node.get_node("MoraleBar")
	morale_bar.value = info_array[4]

	var manpower_label: RichTextLabel = node.get_node("Manpower_Label")
	manpower_label.text = "[center]" + str(info_array[3]) + "[/center]"

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
	return army_locations.has(tile_to_check) and get_top_army(tile_to_check).get_player_id() != player_id

func tile_has_no_unit(tile_to_check: Vector2i) -> bool:
	return !army_locations.has(tile_to_check)

func tile_has_friendly_unit(coords: Vector2i, player_id: int) -> bool:
	#TODO: Change to allow alliances
	return army_locations.has(coords) and get_top_army(coords).get_player_id() == player_id

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
	var new_army: army = army.new(player_id)
	army_locations[coords].append(new_army.army_id)
	army_data[new_army.army_id] = new_army
	new_army.add_unit(unit_class.new(coords, player_id))

	create_label(new_army.get_army_id(), coords, str(new_army))
	refresh_army(new_army.convert_to_client_array())

func get_unit_class(type: int) -> GDScript:
	return unit_creator.get_unit_class(type)

# === Label Creation ===
func create_label(army_id: int, coords: Vector2i, text: String) -> void:
	var label: Label = Label.new()
	label.name = "Label"

	var node: Control = Control.new()
	add_child(node)
	node.add_child(label)
	node.name = "Control:" + str(army_id)

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
	var id: int = multiplayer.get_remote_sender_id()
	var army_obj: army = get_army(army_id)
	if army_obj != null:
		set_army_route_locally.rpc(army_id, army_obj.get_location(), move_to)

func set_army_route_locally(army_id: int, move_to: Vector2i) -> void:
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
	
	if get_cell_atlas_coords(move_to) != Vector2i(-1, -1):
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
	army_locations[army_obj.get_location()].append(army_obj.get_army_id())

func do_army_leave(coords: Vector2i) -> void:
	if army_locations[coords].is_empty():
		erase_cell(coords)

func get_control_node(army_id: int) -> Control:
	return get_node("Control:" + str(army_id))

func move_control(army_id: int, move_to: Vector2i) -> void:
	var node: Control = get_control_node(army_id)
	node.position = map_to_local(move_to)

# Availability Checking
func location_is_attack(coords_of_defender: Vector2i, player_id: int) -> bool:
	return tile_has_enemy_army(coords_of_defender, player_id)

func bfs_to_destination(army_obj: army, destination: Vector2i) -> Array:
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
			if map.get_cell_tile_data(tile) == null:
				continue
			var terrain: int = map.get_cell_tile_data(tile).terrain
			var temp_dist: float =  map.map_to_local(current).distance_to(map.map_to_local(tile))
			var current_dist: float = visited[current] + (temp_dist / army.get_speed_mult(terrain))
			if map.is_tile_traversable(tile) and !location_is_attack(tile, army_obj.get_player_id()) and !visited.has(tile):
				queue.insert_element(tile, 100.0 / current_dist)
				visited[tile] = current_dist
				tile_to_prev[tile] = current
	if found:
		return create_route_from_tile_to_prev(army_obj.get_location(), destination, tile_to_prev)
	else:
		return []

func create_route_from_tile_to_prev(start: Vector2i, destination: Vector2i, tile_to_prev: Dictionary) -> Array:
	var current: Vector2i = destination
	var route: Array = []
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
		cycle_unit_selection(coords)
		$select_unit_sound.play(0.5)
		state_machine.click_unit()
	elif tile_has_friendly_unit(coords, player_id):
		selected_army = get_top_army(coords)
		var soldier_atlas: Vector2i = get_cell_atlas_coords(coords)
		$select_unit_sound.play(0.5)
		state_machine.click_unit()
	else:
		selected_army = null
		map.close_unit_box()
	highlight_dest()
	highlight_name()

func cycle_unit_selection(coords: Vector2i) -> void:
	var index: int = -1
	var army_stack: Array = army_locations[coords]
	for i: int in army_stack.size():
		var army_id: int = army_stack[i]
		#Will select new army after looping once
		if index != -1:
			selected_army = get_army(army_id)
			break
		
		if army_id == selected_army.get_army_id():
			index = i
	#On last army, cycle back to first
	if index != -1:
		selected_army = get_top_army(coords)

func highlight_name() -> void:
	apply_color_to_selected_unit(Color(1, 0, 0, 1))

func unhightlight_name() -> void:
	apply_color_to_selected_unit(Color(1, 1, 1, 1))

func apply_color_to_selected_unit(color: Color) -> void:
	if selected_army == null:
		return
	var coords: Vector2i = selected_army.get_location()
	var node: Control = get_control_node(selected_army.get_army_id())
	if army_is_owned(selected_army):
		var unit_name: Label = node.get_node("Label")
		unit_name.add_theme_color_override("font_color", color)

func highlight_dest() -> void:
	if selected_army != null:
		var coords: Vector2i = selected_army.get_location()
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
			
	for location: Vector2i in extra_unit_data:
		var unit: base_unit = extra_unit_data[location]
		unit.update_progress(delta)
		var next_location: Vector2i = unit.get_next_location()
		var terrain: int = map.get_cell_tile_data(next_location).terrain
		@warning_ignore("static_called_on_instance")
		if next_location != location and unit.ready_to_move(100.0 / unit.get_speed() / unit.get_speed_mult(terrain)):
			var next_next_location: Vector2i = unit.get_next_location(1)
			if (location_is_attack(next_next_location, unit) and unit.get_unit_range() >= 2) or location_is_attack(next_location, unit):
				unit.stop()
			else:
				check_move(unit)
			prepare_refresh_unit(unit)
	retreat_units()
	clean_up_killed_units()

func update_army_progress(army_obj: army, delta: float) -> void:
	army_obj.update_progress(delta)
	var location: Vector2i = army_obj.get_location()
	var next_location: Vector2i = army_obj.get_next_location()
	if next_location != location:
		var terrain: int = map.get_cell_tile_data(next_location).terrain
		if unit.ready_to_move(100.0 / army_obj.get_speed() / army_obj.get_speed_mult(terrain)):
			var next_next_location: Vector2i = unit.get_next_location(1)
			if location_is_attack(next_next_location, unit) and unit.get_unit_range() >= 2:
				unit_battle(unit, unit_data[next_next_location])
				prepare_refresh_unit(unit_data[next_next_location])
			elif location_is_attack(next_location, unit):
				unit_battle(unit, unit_data[next_location])
				prepare_refresh_unit(unit_data[next_location])
			else:
				check_move(unit)
			prepare_refresh_unit(unit)


func prepare_refresh_army(army_obj: army) -> void:
	var info_array: Array = army_obj.convert_to_client_array()
	refresh_army.rpc(info_array)

func get_unit_client_array(coords: Vector2i) -> Array:
	if unit_data.has(coords):
		return unit_data[coords].convert_to_client_array()
	return []

func clean_up_killed_units() -> void:
	for unit: base_unit in units_to_kill:
		if unit_is_bottom(unit):
			kill_extra_unit.rpc(unit.get_location())
		else:
			kill_normal_unit.rpc(unit.get_location())
	units_to_kill.clear()

@rpc("authority", "call_local", "unreliable")
func kill_normal_unit(coords: Vector2i) -> void:
	var node: Control = get_node(str(coords))
	unit_data[coords].queue_free()
	unit_data.erase(coords)
	clean_up_node(node)
	erase_cell(coords)

@rpc("authority", "call_local", "unreliable")
func kill_extra_unit(coords: Vector2i) -> void:
	var node: Control = get_node(str(coords) + "extra")
	extra_unit_data[coords].queue_free()
	extra_unit_data.erase(coords)
	clean_up_node(node)

func clean_up_node(node: Node) -> void:
	for child: Node in node.get_children():
		child.queue_free()
	node.queue_free()

func retreat_units() -> void:
	for unit: base_unit in units_to_retreat:
		var location: Vector2i = unit.get_location()
		var player_id: int = unit.get_player_id()
		var destination: Vector2i = find_surrounding_open_tile(location, player_id)
		if destination != location:
			set_unit_route(unit, destination)
		else:
			units_to_kill.append(unit)
	units_to_retreat.clear()

func find_surrounding_open_tile(coords: Vector2i, player_id: int) -> Vector2i:
	for cell: Vector2i in map.get_surrounding_cells(coords):
		if map.is_tile_traversable(cell) and (tile_has_no_unit(cell) or tile_has_no_extra_unit(cell)) and not tile_has_enemy_unit(cell, player_id):
			return cell
	return coords

func get_additional_aura_boost() -> Dictionary:
	var adjacent_officer: Dictionary = {}
	for cell: Vector2i in get_used_cells():
		if unit_data.has(cell) and unit_data[cell] is officer:
			for adj_cell: Vector2i in get_surrounding_cells(cell):
				if adjacent_officer.has(adj_cell) and adjacent_officer[adj_cell] > unit_data[cell].get_aura_boost():
					continue
				adjacent_officer[adj_cell] = unit_data[cell].get_aura_boost()
	return adjacent_officer

func _on_regen_timer_timeout() -> void:
	var boosts: Dictionary = get_additional_aura_boost()
	for unit: base_unit in unit_data.values():
		regen_tick(unit, boosts)
	for unit: base_unit in extra_unit_data.values():
		extra_regen_tick(unit)

func regen_tick(unit: base_unit, boosts: Dictionary) -> void:
	var multiple: int = 1
	if boosts.has(unit.get_location()):
		multiple = boosts[unit.get_location()]
	unit.add_experience(multiple)
	manpower_and_morale_tick(unit)
	unit.use_supplies()
	refresh_normal_unit.rpc(unit.convert_to_client_array())

func extra_regen_tick(unit: base_unit) -> void:
	manpower_and_morale_tick(unit)
	refresh_extra_unit.rpc(unit.convert_to_client_array())

func manpower_and_morale_tick(unit: base_unit) -> void:
	var player_id: int = unit.get_player_id()
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
