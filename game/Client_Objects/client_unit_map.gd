extends TileMapLayer

var unit_creator: RefCounted
var map: TileMapLayer
var selected_armies: Array[army]
var last_click: Vector2
var click_valid: bool = false
var shift_held: bool = false


var army_locations: Dictionary[Vector2i, Array] = {} #Array[army]
var attacking_army_locations: Dictionary[Vector2i, Array] = {} #Array[army]
var army_data: Dictionary[int, client_army] #Army id -> Army

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	unit_creator = preload("res://Units/unit_managers/army_creator.gd").new()
	map = get_parent() as TileMapLayer

func _process(_delta: float) -> void:
	process_gui()

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
		request_set_army_route.rpc_id(1, get_selected_army_ids(), map.get_cell_position())
		if is_selecting_one_army():
			map.update_info_window(get_selected_army())
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
	if has_node("selection_box"):
		var box: ColorRect = get_node("selection_box")
		remove_child(box)
		box.queue_free()

func show_army_info_window() -> void:
	if !is_selecting_one_army():
		return
	var army_obj: army = get_selected_army()
	map.show_army_info_window(army_obj)

# === Syncing Methods ===
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
func refresh_army(army_id: int, info_dict: Dictionary) -> void:
	get_army(army_id).update_stats(info_dict)
	move_control(army_id, get_army(army_id).get_location())
	var node: Node = get_control_node(army_id)
	var morale_bar: ProgressBar = node.get_node("MoraleBar")
	morale_bar.value = info_dict["morale"]

	var manpower_label: RichTextLabel = node.get_node("Manpower_Label")
	manpower_label.text = "[center]" + str(info_dict["manpower"]) + "[/center]"

	var name_label: Label = node.get_node("Label")
	name_label.text = str(get_army(army_id))

@rpc("authority", "call_remote", "unreliable")
func refresh_unit_in_army(army_id: int, index: int, unit_array: Array) -> void:
	var army_obj: client_army = get_army(army_id)
	if army_obj != null:
		army_obj.refresh_unit(index, unit_array)

# === Unit Checks ===
func is_selecting_one_army() -> bool:
	return selected_armies.size() == 1

func get_selected_army_ids() -> Array[int]:
	var toReturn: Array[int] = []
	for selected_army: army in selected_armies:
		toReturn.append(selected_army.get_army_id())
	return toReturn

func get_army_depth(army_id: int, coords: Vector2i) -> int:
	var index: int = 0
	for id: int in army_locations[coords]:
		if id == army_id:
			return index
		index += 1
	return -1

func get_army(army_id: int) -> client_army:
	if !army_data.has(army_id):
		return null
		#assert(false, "Desync")
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

# === Army Utilities ===
@rpc("any_peer", "call_local", "unreliable")
func request_merge_armies(_selected_army_ids: Array) -> void:
	pass

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

@rpc("any_peer", "call_local", "unreliable")
func request_split_armies(_selected_army_ids: Array) -> void:
	pass

@rpc("authority", "call_local", "unreliable")
func split_armies(selected_army_ids: Array) -> void:
	for army_id: int in selected_army_ids:
		var selected_army: army = get_army(army_id)
		if selected_army.can_split():
			var new_army: army = selected_army.split()
			create_army_from_object(new_army)

# === Unit creation === 
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

func create_army_from_object(new_army: army) -> void:
	var coords: Vector2i = new_army.get_location()
	var player_id: int = new_army.get_player_id()
	army_locations[coords].append(new_army.army_id)
	army_data[new_army.army_id] = new_army

	create_label(new_army.get_army_id(), coords, str(new_army))
	refresh_army(new_army.get_army_id(), new_army.get_army_client_dict())
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

# === Labels === 

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

# === Army Selecting ===

func get_selected_army() -> army:
	if selected_armies.size() == 1:
		return selected_armies[0]
	return null

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
	if selected_armies.has(army_obj) and army_obj.get_player_id() == multiplayer.get_unique_id():
		$dest_sound.play(0.3)
		highlight_dest()

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

func move_attacking_armies_to_normal(coords: Vector2i) -> void:
	for army_id: int in attacking_army_locations[coords]:
		army_locations[coords].append(army_id)
	attacking_army_locations[coords].clear()
