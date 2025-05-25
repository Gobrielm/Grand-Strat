extends TileMapLayer

# === Variables ===
var map: TileMapLayer
var unit_creator: Node
var battle_script: Node
var selected_armies: Array[army] = []
var last_click: Vector2
var click_valid: bool = false
var shift_held: bool = false

#Assuming only matching player_id armies can stand on same tile, will change later
var army_locations: Dictionary[Vector2i, Array] = {} #Array[army]
var attacking_army_locations: Dictionary[Vector2i, Array] = {} #Array[army]
var army_data: Dictionary[int, army] #Army id -> Army

var armies_to_kill: Dictionary[int, bool] = {}
var armies_to_retreat: Dictionary[int, bool] = {}

# === Built-ins ===
func _ready() -> void:
	battle_script = load("res://Units/unit_managers/battle_script.gd").new(self)
	unit_creator = load("res://Units/unit_managers/army_creator.gd").new()
	map = get_parent()
	Utils.assign_unit_map(self)

func _input(event: InputEvent) -> void:
	if event.is_action_pressed("click") and (state_machine.is_controlling_camera() or state_machine.is_selecting_unit()):
		last_click = map.get_mouse_local_to_map()
		click_valid = true
	elif event.is_action_released("click") and (state_machine.is_controlling_camera() or state_machine.is_selecting_unit()):
		select_many_armies(map.get_mouse_local_to_map(), last_click , multiplayer.get_unique_id(), shift_held)
		click_valid = false
		remove_selection_box()
		show_army_info_window()
	
	elif event.is_action_pressed("deselect") and state_machine.is_selecting_unit():
		request_set_army_route(get_selected_army_ids(), map.get_cell_position())
		if is_selecting_one_army():
			map.update_info_window(get_selected_army().get_army_client_array())
	elif event.is_action_pressed("merge_armies") and state_machine.is_selecting_unit():
		request_merge_armies.rpc_id(1, get_selected_army_ids())
	elif event.is_action_pressed("split_armies") and state_machine.is_selecting_unit():
		request_split_armies.rpc_id(1, get_selected_army_ids())

# === Gui ===
func process_gui() -> void:
	if Input.is_action_pressed("click") and click_valid:
		update_selection_box()
	if Input.is_action_pressed("shift"):
		shift_held = true
	else:
		shift_held = false

func update_selection_box() -> void:
	var rectangle: ColorRect
	if has_node("selection_box"):
		rectangle = get_node("selection_box")
	else:
		rectangle = ColorRect.new()
		rectangle.name = "selection_box"
		rectangle.color = Color(0.5, 0.5, 0.5, 0.3) 
		add_child(rectangle)
	var coords1: Vector2 = last_click
	var coords2: Vector2 = map.get_mouse_local_to_map()
	var top_left: Vector2 = coords1.min(coords2)
	var size: Vector2 = (coords1 - coords2).abs()

	rectangle.position = top_left
	rectangle.size = size

func remove_selection_box() -> void:
	var box: ColorRect = get_node("selection_box")
	remove_child(box)
	box.queue_free()

func show_army_info_window() -> void:
	if !is_selecting_one_army():
		return
	var unit_info_array: Array = (get_selected_army() as army).get_army_client_array()
	map.show_army_info_window(unit_info_array)

# === Networking ===
func is_selecting_one_army() -> bool:
	return selected_armies.size() == 1

func get_selected_army_ids() -> Array[int]:
	var toReturn: Array[int] = []
	for selected_army: army in selected_armies:
		toReturn.append(selected_army.get_army_id())
	return toReturn

func send_data_to_clients() -> void:
	map.refresh_unit_map.rpc(get_used_cells_dictionary())

@rpc("any_peer", "call_remote", "unreliable")
func request_refresh_map() -> void:
	map.refresh_unit_map.rpc_id(multiplayer.get_remote_sender_id(), get_used_cells_dictionary())

@rpc("authority", "call_remote", "reliable")
func refresh_map(_visible_tiles: Array, _unit_atlas: Dictionary) -> void:
	pass

func force_refresh_all(tile: Vector2i) -> void:
	var ids: Array = multiplayer.get_peers()
	ids.append(1)
	for sender_id: int in ids:
		if army_locations.has(tile):
			var temp: Array[int] = []
			temp.assign(army_locations[tile])
			refresh_all_armies(temp, sender_id)

@rpc("any_peer", "call_remote", "unreliable")
func request_refresh(tile: Vector2i) -> void:
	var sender_id: int = multiplayer.get_remote_sender_id()
	if army_locations.has(tile):
		var temp: Array[int] = []
		temp.assign(army_locations[tile])
		refresh_all_armies(temp, sender_id)

#TODO: Optimize function to send individual rpcs for each unit indexed for each army to just replace data
func refresh_all_armies(armies: Array[int], sender_id: int) -> void:
	for army_id: int in armies:
		var army_obj: army = army_data[army_id]
		refresh_army.rpc_id(sender_id, army_id, army_obj.get_army_client_array(), army_obj.get_units_client_arrays())

@rpc("authority", "call_local", "unreliable")
func refresh_army(army_id: int, info_array: Array, _units_array: Array) -> void:
	var node: Node = get_control_node(army_id)
	var coords: Vector2i = get_army(army_id).get_location()
	move_control(army_id, coords)
	
	var morale_bar: ProgressBar = node.get_node("MoraleBar")
	morale_bar.value = info_array[1]

	var manpower_label: RichTextLabel = node.get_node("Manpower_Label")
	manpower_label.text = "[center]" + str(info_array[0]) + "[/center]"

func get_army_depth(army_id: int, coords: Vector2i) -> int:
	var index: int = 0
	for id: int in army_locations[coords]:
		if id == army_id:
			return index
		index += 1
	return -1

# === Utility ===
func get_used_cells_dictionary() -> Dictionary:
	var to_return: Dictionary = {}
	for tile: Vector2i in get_used_cells():
		to_return[tile] = get_cell_atlas_coords(tile)
	return to_return

# === Army Utilities ===
@rpc("any_peer", "call_local", "unreliable")
func request_merge_armies(selected_army_ids: Array) -> void:
	merge_armies.rpc(selected_army_ids, multiplayer.get_remote_sender_id())

@rpc("authority", "call_local", "unreliable")
func merge_armies(selected_army_ids: Array, unique_id: int) -> void:
	if selected_army_ids.size() < 2:
		return
	var coords: Vector2i = get_army(selected_army_ids[0]).get_location()
	for army_id: int in selected_army_ids:
		var selected_army: army = get_army(army_id)
		if selected_army.get_location() != coords:
			return
	var first_army: army = get_army(selected_army_ids[0])
	for index: int in range(1, selected_army_ids.size()):
		var selected_army: army = get_army(selected_army_ids[index])
		first_army.merge(selected_army)
		kill_army(selected_army.army_id, coords)
	if multiplayer.get_unique_id() == unique_id:
		selected_armies.clear()
		selected_armies.push_back(first_army)
	force_refresh_all(coords)

@rpc("any_peer", "call_local", "unreliable")
func request_split_armies(selected_army_ids: Array) -> void:
	split_armies.rpc(selected_army_ids)

@rpc("authority", "call_local", "unreliable")
func split_armies(selected_army_ids: Array) -> void:
	var tiles_refresh: Dictionary[Vector2i, bool] = {}
	for army_id: int in selected_army_ids:
		var selected_army: army = get_army(army_id)
		if selected_army.can_split():
			var new_army: army = selected_army.split()
			create_army_from_object(new_army)
			var coords: Vector2i = selected_army.get_location()
			if !tiles_refresh.has(coords):
				force_refresh_all(coords)
				tiles_refresh[coords] = true
			

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
	
	if money_cntrl.player_has_enough_money(player_id, cost):
		money_cntrl.remove_money_from_player(player_id, cost)
		create_army_locally(coords, type, player_id)
		create_army.rpc(coords, type, player_id, army_locations[coords].back())

@rpc("authority", "call_remote", "unreliable")
func create_army(_coords: Vector2i, _type: int, _player_id: int, _army_id: int) -> void:
	pass

func create_army_locally(coords: Vector2i, type: int, player_id: int) -> void:
	if !army_locations.has(coords):
		army_locations[coords] = []
		set_cell(coords, 0, Vector2i(0, type))

	var unit_class: GDScript = get_unit_class(type)
	var new_army: army = army.new(player_id, coords)
	army_locations[coords].append(new_army.army_id)
	army_data[new_army.army_id] = new_army
	new_army.add_unit(unit_class.new())

	create_label(new_army.get_army_id(), coords, str(new_army))
	refresh_army(new_army.get_army_id(), new_army.get_army_client_array(), new_army.get_units_client_arrays())

func create_army_from_object(new_army: army) -> void:
	var coords: Vector2i = new_army.get_location()
	var player_id: int = new_army.get_player_id()
	army_locations[coords].append(new_army.army_id)
	army_data[new_army.army_id] = new_army

	create_label(new_army.get_army_id(), coords, str(new_army))
	refresh_army(new_army.get_army_id(), new_army.get_army_client_array(), new_army.get_units_client_arrays())
	create_army.rpc(coords, -1, player_id, army_locations[coords].back())

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
func request_set_army_route(army_ids: Array[int], move_to: Vector2i) -> void:
	for army_id: int in army_ids:
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
	army_obj.set_route(bfs_to_destination(army_obj, move_to))
	if selected_armies.has(army_obj) and army_obj.get_player_id() == multiplayer.get_unique_id():
		$dest_sound.play(0.3)
		highlight_dest()

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
	highlight_dest()

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
	fix_control_gui(army_id, move_to)
	
	node.position = map_to_local(move_to)

func fix_control_gui(army_id: int, coords: Vector2i) -> void:
	var node: Control = get_control_node(army_id)
	var move_from: Vector2i = map.local_to_map(node.position)
	#Fixes moving army gui
	change_gui_with_stack_size(coords)
	#Fixes army that was potentially left
	change_gui_with_stack_size(move_from)
	unhightlight_name()
	highlight_name()
	var depth: int = get_army_depth(army_id, coords)
	if depth != 0:
		node.visible = false
	else:
		node.visible = true

func change_gui_with_stack_size(coords: Vector2i) -> void:
	if !army_locations.has(coords):
		return
	if army_locations[coords].size() == 1:
		#Gets rid of old tokens
		var top_army_id: int = army_locations[coords][0]
		make_control_normal(top_army_id)
		#Brings back bar
		var node: Control = get_control_node(top_army_id)
		node.visible = true
	elif army_locations[coords].size() > 1:
		make_control_top_level(army_locations[coords][0], coords)

func make_control_normal(army_id: int) -> void:
	var node: Control = get_control_node(army_id)
	for child: Node in node.get_children():
		if child.name.begins_with("Rect"):
			node.remove_child(child)
			child.queue_free()

func make_control_top_level(army_id: int, coords: Vector2i) -> void:
	var depth: int = army_locations[coords].size()
	var node: Control = get_control_node(army_id)
	var depth_before: int = 0
	for child: Node in node.get_children():
		if child is ColorRect:
			depth_before += 1
	
	for i: int in range(max(depth, depth_before)):
		if (i < depth):
			var rect: ColorRect
			if !node.has_node("Rect" + str(i)):
				rect = ColorRect.new()
				rect.name = "Rect" + str(i)
				rect.size = Vector2(10, 10)
				node.add_child(rect)
			else:
				rect = node.get_node("Rect" + str(i))
			rect.color = Color(0.3, 0.3, 0.3, 0.5)
			rect.position = Vector2(-35 + i * 13, 70)
		else:
			var rect: ColorRect = node.get_node("Rect" + str(i))
			node.remove_child(rect)
			rect.queue_free()

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

# === Army Selecting === 

func get_selected_army() -> army:
	if selected_armies.size() == 1:
		return selected_armies[0]
	return null

func army_is_owned(army_obj: army) -> bool:
	return army_obj.get_player_id() == multiplayer.get_unique_id()

func select_many_armies(coords1: Vector2, coords2: Vector2, player_id: int, additive: bool = false) -> void:
	var tile1: Vector2i = map.local_to_map(coords1)
	if tile1 == map.local_to_map(coords2):
		select_army(tile1, player_id, additive)
		return
	unhightlight_name()
	if !additive:
		selected_armies.clear()
	if (coords1.x > coords2.x):
		var temp: float = coords1.x
		coords1.x = coords2.x
		coords2.x = temp
	if (coords1.y > coords2.y):
		var temp: float = coords1.y
		coords1.y = coords2.y
		coords2.y = temp
	var added_armies: Dictionary[int, bool] = {}
	#Steps are width/height of a tile
	for x: float in range(coords1.x, coords2.x, 60):
		for y: float in range(coords1.y, coords2.y, 55):
			var coords: Vector2i = map.local_to_map(Vector2(x, y))
			if tile_has_friendly_army(coords, player_id):
				for army_id: int in army_locations[coords]:
					if !added_armies.has(army_id):
						selected_armies.append(get_army(army_id))
					added_armies[army_id] = true
				$select_unit_sound.play(0.5)
				
	if selected_armies.is_empty():
		state_machine.unclick_unit()
		map.close_unit_box()
	else:
		state_machine.click_unit()
		highlight_dest()
		highlight_name()

func select_army(coords: Vector2i, player_id: int, additive: bool = false) -> void:
	var toAdd: army = null
	unhightlight_name()
	if is_selecting_one_army() and get_selected_army().get_location() == coords:
		toAdd = cycle_army_selection(coords)
		$select_unit_sound.play(0.5)
		state_machine.click_unit()
	elif tile_has_friendly_army(coords, player_id):
		toAdd = get_top_army(coords)
		$select_unit_sound.play(0.5)
		state_machine.click_unit()
	else:
		
		state_machine.unclick_unit()
		map.close_unit_box()
	if !additive:
		selected_armies.clear()
	if toAdd != null:
		selected_armies.append(toAdd)
		highlight_dest()
		highlight_name()

func cycle_army_selection(coords: Vector2i) -> army:
	var next_unit: bool = false
	var selected_army: army = get_selected_army()
	var army_stack: Array = army_locations[coords]
	for i: int in army_stack.size():
		var army_id: int = army_stack[i]
		#Will select new army after looping once
		if next_unit:
			return get_army(army_id)
		
		if army_id == selected_army.get_army_id():
			next_unit = true
	
	#On last army, cycle back to first
	return get_top_army(coords)

func highlight_name() -> void:
	apply_color_to_selected_unit(Color(1, 0, 0, 1))
	apply_color_to_stack_tokens(Color(1, 0.3, 0.3, 0.5))

func unhightlight_name() -> void:
	apply_color_to_selected_unit(Color(1, 1, 1, 1))
	apply_color_to_stack_tokens(Color(0.3, 0.3, 0.3, 0.5))

func apply_color_to_selected_unit(color: Color) -> void:
	for selected_army: army in selected_armies:
		var node: Control = get_control_node(selected_army.get_army_id())
		var unit_name: Label = node.get_node("Label")
		unit_name.add_theme_color_override("font_color", color)

func apply_color_to_stack_tokens(color: Color) -> void:
	for selected_army: army in selected_armies:
		var coords: Vector2i = selected_army.get_location()
		var depth: int = get_army_depth(selected_army.get_army_id(), coords)
		var c_node: Control = get_control_node(get_top_army(coords).get_army_id())
		if c_node.has_node("Rect" + str(depth)):
			var rect: ColorRect = c_node.get_node("Rect" + str(depth))
			rect.color = color

func highlight_dest() -> void:
	map.clear_highlights()
	for selected_army: army in selected_armies:
		if selected_army.get_destination() != null:
			map.highlight_cell(selected_army.get_destination())
		

# === Processses ===

func _process(delta: float) -> void:
	process_gui()
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
	var units_array: Array = army_obj.get_units_client_arrays()
	refresh_army.rpc(army_obj.get_army_id(), info_array, units_array)

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
	check_and_clean_attacking_army(army_id, coords)
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
	if army_locations.has(coords):
		var army_stack: Array = army_locations[coords]
		for index: int in army_stack.size():
			if army_stack[index] == army_id:
				army_stack.remove_at(index)
				break

func check_and_clean_attacking_army(army_id: int, coords: Vector2i) -> void:
	#Cleans attacking armies
	if attacking_army_locations.has(coords):
		var army_stack: Array = attacking_army_locations[coords]
		for index: int in army_stack.size():
			if army_stack[index] == army_id:
				army_stack.remove_at(index)
				break

func clean_up_node(node: Node) -> void:
	for child: Node in node.get_children():
		child.queue_free()
	node.queue_free()

func _on_month_tick_timeout() -> void:
	for army_obj: army in army_data.values():
		regen_tick(army_obj)

func regen_tick(army_obj: army) -> void:
	var multiple: int = 1
	army_obj.add_experience(multiple)
	manpower_and_morale_tick(army_obj)
	army_obj.use_supplies()
	refresh_army.rpc(army_obj.get_army_id(), army_obj.get_army_client_array(), army_obj.get_units_client_arrays())

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
