extends TileMapLayer

# === Variables ===
var map: TileMapLayer
var unit_creator: Node
var battle_script: Node
var selected_unit: base_unit

var unit_data: Dictionary = {}
var extra_unit_data: Dictionary = {}
var temp_unit_data: Dictionary = {}

var units_to_kill: Array = []
var units_to_retreat: Array = []

# === Built-ins ===
func _ready() -> void:
	battle_script = load("res://Units/unit_managers/battle_script.gd").new(self)
	unit_creator = load("res://Units/unit_managers/unit_creator.gd").new()
	map = get_parent()

func _input(event: InputEvent) -> void:
	if event.is_action_pressed("deselect") and state_machine.is_selecting_unit():
		set_up_set_unit_route(selected_unit, map.get_cell_position())
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
	if unit_data.has(tile):
		refresh_normal_unit.rpc_id(sender_id, unit_data[tile].convert_to_client_array())
	if extra_unit_data.has(tile):
		refresh_extra_unit.rpc_id(sender_id, extra_unit_data[tile].convert_to_client_array())

@rpc("authority", "call_local", "unreliable")
func refresh_normal_unit(info_array: Array) -> void:
	var coords: Vector2i = info_array[1]
	var node: Node = get_node(str(coords))
	refresh_unit(info_array, node)

@rpc("authority", "call_local", "unreliable")
func refresh_extra_unit(info_array: Array) -> void:
	var coords: Vector2i = info_array[1]
	var node: Node = get_node(str(coords) + "extra")
	refresh_unit(info_array, node)

func refresh_unit(info_array: Array, node: Node) -> void:
	var coords: Vector2i = info_array[1]
	if selected_unit != null and coords == selected_unit.get_location():
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
func tile_has_enemy_unit(tile_to_check: Vector2i, player_id: int) -> bool:
	return unit_data.has(tile_to_check) and unit_data[tile_to_check].get_player_id() != player_id

func tile_has_no_unit(tile_to_check: Vector2i) -> bool:
	return !unit_data.has(tile_to_check)

func tile_has_no_extra_unit(tile_to_check: Vector2i) -> bool:
	return !extra_unit_data.has(tile_to_check)

func is_player_id_match(coords: Vector2i, player_id: int) -> bool:
	return unit_data.has(coords) and unit_data[coords].get_player_id() == player_id

# === Creating Units ===
@rpc("any_peer", "call_local", "unreliable")
func check_before_create(coords: Vector2i, type: int, player_id: int) -> void:
	var unit_class: GDScript = get_unit_class(type)
	var cost: int = unit_class.get_cost()

	if !unit_data.has(coords) and map.player_has_enough_money(player_id, cost):
		map.remove_money(player_id, cost)
		create_unit.rpc(coords, type, player_id)

@rpc("authority", "call_local", "unreliable")
func create_unit(coords: Vector2i, type: int, player_id: int) -> void:
	set_cell(coords, 0, Vector2i(0, type))

	var unit_class: GDScript = get_unit_class(type)
	unit_data[coords] = unit_class.new(coords, player_id)

	create_label(coords, str(unit_data[coords]))
	prepare_refresh_unit(unit_data[coords])

func get_unit_class(type: int) -> GDScript:
	return unit_creator.get_unit_class(type)

# === Label Creation ===
func create_label(coords: Vector2i, text: String) -> void:
	var label: Label = Label.new()
	label.name = "Label"

	var node: Control = Control.new()
	add_child(node)
	node.add_child(label)
	node.name = str(coords)

	label.text = text
	move_label_to_normal(coords, coords)
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
func set_up_set_unit_route(unit: base_unit, move_to: Vector2i) -> void:
	if unit == null:
		return
	var coords: Vector2i = unit.get_location()
	if unit_is_bottom(unit):
		set_selected_unit_route.rpc_id(1, coords, true, move_to)
	else:
		set_selected_unit_route.rpc_id(1, coords, false, move_to)

@rpc("any_peer", "call_local", "unreliable")
func set_selected_unit_route(coords: Vector2i, extra: bool, move_to: Vector2i) -> void:
	var id: int = multiplayer.get_remote_sender_id()
	var unit: base_unit = null
	if unit_data.has(coords):
		unit = unit_data[coords]
		if extra:
			unit = extra_unit_data[coords]
	if unit != null and unit.get_player_id() == id:
		if !unit_is_bottom(unit):
			set_normal_unit_route.rpc(coords, move_to)
		elif extra_unit_data[coords] == unit:
			set_extra_unit_route.rpc(coords, move_to)

@rpc("authority", "call_local", "unreliable")
func set_normal_unit_route(coords: Vector2i, move_to: Vector2i) -> void:
	if unit_data.has(coords):
		var unit: base_unit = unit_data[coords]
		set_unit_route(unit, move_to)
		refresh_normal_unit.rpc(unit.convert_to_client_array())

@rpc("authority", "call_local", "unreliable")
func set_extra_unit_route(coords: Vector2i, move_to: Vector2i) -> void:
	if extra_unit_data.has(coords):
		var unit: base_unit = extra_unit_data[coords]
		set_unit_route(unit, move_to)
		refresh_extra_unit.rpc(unit.convert_to_client_array())

func set_unit_route(unit: base_unit, move_to: Vector2i) -> void:
	unit.set_route(dfs_to_destination(unit.get_location(), move_to))
	if unit == selected_unit and unit.get_player_id() == multiplayer.get_unique_id():
		$dest_sound.play(0.3)
		highlight_dest()

func get_selected_coords() -> Vector2i:
	return selected_unit.get_location()

# Movement Logic
func check_move(unit: base_unit) -> void:
	var coords: Vector2i = unit.get_location()
	var move_to: Vector2i = unit.pop_next_location()
	if next_location_is_available(move_to):
		if unit_is_bottom(unit):
			move_extra_unit_to_regular.rpc(coords, move_to)
		else:
			move_unit_to_regular.rpc(coords, move_to)
	elif next_location_available_for_extra(move_to):
		if unit_is_bottom(unit):
			move_extra_unit_to_extra.rpc(coords, move_to)
		else:
			move_unit_to_extra.rpc(coords, move_to)
	elif !unit.is_route_empty():
		var end: Vector2i = unit.get_destination()
		if unit_is_bottom(unit):
			set_extra_unit_route.rpc(unit.get_location(), end)
		else:
			set_normal_unit_route.rpc(unit.get_location(), end)

@rpc("authority", "call_local", "unreliable")
func move_unit_to_regular(coords: Vector2i, move_to: Vector2i) -> void:
	var unit: base_unit = unit_data[coords]
	normal_move(coords)
	normal_arrival(unit, move_to)
	move_label_to_normal(coords, move_to)
	if unit.get_destination() == null:
		map.clear_highlights()
	check_extra(coords)

@rpc("authority", "call_local", "unreliable")
func move_unit_to_extra(coords: Vector2i, move_to: Vector2i) -> void:
	var unit: base_unit = unit_data[coords]
	normal_move(coords)
	extra_arrival(unit, move_to)
	move_label_to_extra(coords, move_to)
	if unit.get_destination() == null:
		map.clear_highlights()
	check_extra(coords)

@rpc("authority", "call_local", "unreliable")
func move_extra_unit_to_regular(coords: Vector2i, move_to: Vector2i) -> void:
	var unit: base_unit = extra_unit_data[coords]
	extra_move(coords)
	normal_arrival(unit, move_to)
	move_extra_label_to_normal(coords, move_to)
	if unit.get_destination() == null:
		map.clear_highlights()

@rpc("authority", "call_local", "unreliable")
func move_extra_unit_to_extra(coords: Vector2i, move_to: Vector2i) -> void:
	var unit: base_unit = extra_unit_data[coords]
	extra_move(coords)
	extra_arrival(unit, move_to)
	move_extra_label_to_extra(coords, move_to)
	if unit.get_destination() == null:
		map.clear_highlights()

# Move Helpers
func normal_move(coords: Vector2i) -> void:
	erase_cell(coords)
	unit_data.erase(coords)

func extra_move(coords: Vector2i) -> void:
	extra_unit_data.erase(coords)

func normal_arrival(unit: base_unit, move_to: Vector2i) -> void:
	var unit_atlas: Vector2i = unit.get_atlas_coord()
	unit.set_location(move_to)
	set_cell(move_to, 0, unit_atlas)
	unit_data[move_to] = unit

func extra_arrival(unit: base_unit, move_to: Vector2i) -> void:
	unit.set_location(move_to)
	extra_unit_data[move_to] = unit

func check_extra(coords: Vector2i) -> void:
	if !unit_data.has(coords) and extra_unit_data.has(coords):
		var unit: base_unit = extra_unit_data[coords]
		extra_move(coords)
		normal_arrival(unit, coords)
		move_extra_label_to_normal(coords, coords)

func move_label_to_normal(coords: Vector2i, move_to: Vector2i) -> void:
	var node: Node = get_node(str(coords))
	node.name = str(move_to)
	node.position = map_to_local(move_to)

func move_label_to_extra(coords: Vector2i, move_to: Vector2i) -> void:
	var node: Node = get_node(str(coords))
	node.name = str(move_to) + "extra"
	node.position = map_to_local(move_to)
	node.position.y += 50

func move_extra_label_to_normal(coords: Vector2i, move_to: Vector2i) -> void:
	var node: Node = get_node(str(coords) + "extra")
	node.name = str(move_to)
	node.position = map_to_local(move_to)

func move_extra_label_to_extra(coords: Vector2i, move_to: Vector2i) -> void:
	var node: Node = get_node(str(coords) + "extra")
	node.name = str(move_to) + "extra"
	node.position = map_to_local(move_to)
	node.position.y += 50

# Availability Checking
func location_is_attack(coords_of_defender: Vector2i, attacker: base_unit) -> bool:
	return unit_data.has(coords_of_defender) and unit_data[coords_of_defender].get_player_id() != attacker.get_player_id()

func next_location_is_available_general(coords: Vector2i) -> bool:
	return next_location_is_available(coords) or next_location_available_for_extra(coords)

func next_location_is_available(coords: Vector2i) -> bool:
	return !unit_data.has(coords)

func next_location_available_for_extra(coords: Vector2i) -> bool:
	return !extra_unit_data.has(coords)

func dfs_to_destination(coords: Vector2i, destination: Vector2i) -> Array:
	#TODO: Super broken, not how priority_queue works
	var current: Vector2i
	var queue: priority_queue = priority_queue.new()
	var tile_to_prev: Dictionary = {}
	var visited: Dictionary = {}
	var found: bool = false
	queue.add_item(0, coords)
	visited[coords] = 0
	while !queue.is_empty():
		current = queue.pop_top()
		if current == destination or found:
			found = true
			break
		for tile: Vector2i in map.get_surrounding_cells(current):
			if map.get_cell_tile_data(tile) == null:
				continue
			var terrain: int = map.get_cell_tile_data(tile).terrain
			var current_dist: int = visited[current] + (1 / base_unit.get_speed_mult(terrain))
			if found_closer_route(visited, tile, current_dist) and map.is_tile_traversable(tile) and next_location_is_available_general(tile):
				queue.add_item(current_dist, tile)
				visited[tile] = current_dist
				tile_to_prev[tile] = current
			elif tile == destination:
				tile_to_prev[tile] = current
				found = true
				break
	if found:
		return create_route_from_tile_to_prev(coords, destination, tile_to_prev)
	else:
		return []

func found_closer_route(visited: Dictionary, coords: Vector2i, current_dist: int) -> bool:
	if !visited.has(coords):
		return true
	return visited[coords] > current_dist

func create_route_from_tile_to_prev(start: Vector2i, destination: Vector2i, tile_to_prev: Dictionary) -> Array:
	var current: Vector2i = destination
	var route: Array = []
	while current != start:
		route.push_front(current)
		current = tile_to_prev[current]
	return route

func unit_battle(attacker: base_unit, defender: base_unit) -> void:
	var result: int = battle_script.unit_battle(attacker, defender)
	if result == 0:
		units_to_kill.append(defender)
	elif result == 1:
		units_to_retreat.append(defender)
	elif result == 2:
		units_to_kill.append(attacker)
	elif result == 3:
		units_to_retreat.append(attacker)

func get_selected_unit() -> base_unit:
	return selected_unit

func unit_is_owned(coords: Vector2i) -> bool:
	return unit_data.has(coords) and unit_data[coords].get_player_id() == multiplayer.get_unique_id()

func select_unit(coords: Vector2i, player_id: int) -> void:
	unhightlight_name()
	if selected_unit != null and selected_unit.get_location() == coords:
		cycle_unit_selection(coords)
		$select_unit_sound.play(0.5)
		state_machine.click_unit()
	elif is_player_id_match(coords, player_id):
		selected_unit = unit_data[coords]
		var soldier_atlas: Vector2i = get_cell_atlas_coords(coords)
		if soldier_atlas != Vector2i(-1, -1):
			$select_unit_sound.play(0.5)
			state_machine.click_unit()
	else:
		selected_unit = null
		map.close_unit_box()
	highlight_dest()
	highlight_name()

func cycle_unit_selection(coords: Vector2i) -> void:
	if selected_unit != null and selected_unit.get_location() == coords:
		if unit_is_bottom(selected_unit):
			selected_unit = unit_data[coords]
		else:
			selected_unit = extra_unit_data[coords]

func highlight_name() -> void:
	apply_color_to_selected_unit(Color(1, 0, 0, 1))

func unhightlight_name() -> void:
	apply_color_to_selected_unit(Color(1, 1, 1, 1))

func apply_color_to_selected_unit(color: Color) -> void:
	if selected_unit == null:
		return
	var coords: Vector2i = selected_unit.get_location()
	var node: Control = get_node(str(coords))
	if unit_is_bottom(selected_unit):
		node = get_node(str(coords) + "extra")
	if unit_is_owned(coords):
		var unit_name: Label = node.get_node("Label")
		unit_name.add_theme_color_override("font_color", color)

func unit_is_bottom(unit: base_unit) -> bool:
	if unit != null:
		var coords: Vector2i = unit.get_location()
		return extra_unit_data.has(coords) and extra_unit_data[coords] == unit
	return false

func highlight_dest() -> void:
	if selected_unit != null:
		var coords: Vector2i = selected_unit.get_location()
		if selected_unit.get_destination() != null and unit_is_owned(coords):
			map.highlight_cell(selected_unit.get_destination())
		else:
			map.clear_highlights()
	else:
		map.clear_highlights()

func is_unit_double_clicked(coords: Vector2i, player_id: int) -> bool:
	if extra_unit_data.has(coords):
		return false
	return is_player_id_match(coords, player_id) and selected_unit.get_location() == coords

func _process(delta: float) -> void:
	for location: Vector2i in unit_data:
		var unit: base_unit = unit_data[location]
		if units_to_kill.has(unit) or units_to_retreat.has(unit):
			continue
		unit.update_progress(delta)
		var next_location: Vector2i = unit.get_next_location()
		if next_location != location:
			var terrain: int = map.get_cell_tile_data(next_location).terrain
			@warning_ignore("static_called_on_instance")
			if unit.ready_to_move(100.0 / unit.get_speed() / unit.get_speed_mult(terrain)):
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

func prepare_refresh_unit(unit: base_unit) -> void:
	var info_array: Array = unit.convert_to_client_array()
	if unit_is_bottom(unit):
		refresh_extra_unit.rpc(info_array)
	else:
		refresh_normal_unit.rpc(info_array)

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
	refresh_normal_unit.rpc(unit.convert_to_client_array())

func extra_regen_tick(unit: base_unit) -> void:
	manpower_and_morale_tick(unit)
	refresh_extra_unit.rpc(unit.convert_to_client_array())

func manpower_and_morale_tick(unit: base_unit) -> void:
	var player_id: int = unit.get_player_id()
	var amount: int = round(float(unit.get_max_manpower()) / 80.0 + 12)
	var max_cost: int = round(amount * 0.3)
	if map.player_has_enough_money(player_id, max_cost):
		@warning_ignore("narrowing_conversion")
		var manpower_used: int = ceil(unit.add_manpower(amount / 2.0))
		var cost: int = round(manpower_used * 0.3)
		map.remove_money(player_id, cost)
		unit.add_morale(5)
	else:
		unit.remove_morale(10)
