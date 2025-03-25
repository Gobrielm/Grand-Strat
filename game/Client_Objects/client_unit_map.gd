extends TileMapLayer

var unit_creator
var selected_unit: base_unit
var map: TileMapLayer
var unit_data: Dictionary = {}
var extra_unit_data: Dictionary = {}

const client_unit = preload("res://Client_Objects/client_base_unit.gd")
# Called when the node enters the scene tree for the first time.
func _ready():
	unit_creator = load("res://Units/unit_managers/unit_creator.gd").new()
	map = get_parent()

func _input(event):
	if event.is_action_pressed("deselect"):
		if state_machine.is_selecting_unit():
			set_up_set_unit_route(selected_unit, map.get_cell_position())
			map.update_info_window(get_unit_client_array(get_selected_coords()))

@rpc("any_peer", "call_remote", "unreliable")
func request_refresh_map():
	pass

@rpc("authority", "call_remote", "reliable")
func refresh_map(visible_tiles: Array, unit_atlas: Dictionary):
	for coords in visible_tiles:
		if !unit_atlas.has(coords):
			kill_normal_unit(coords)
		elif get_cell_atlas_coords(coords) == unit_atlas[coords]:
			request_refresh.rpc_id(1, coords)
		else:
			create_unit(coords, unit_atlas[coords].y, 0)
			request_refresh.rpc_id(1, coords)

@rpc("any_peer", "call_remote", "unreliable")
func request_refresh(tile: Vector2i):
	pass

@rpc("authority", "call_local", "unreliable")
func refresh_normal_unit(info_array: Array):
	var coords = info_array[1]
	var node = get_node(str(coords))
	var unit = unit_data[coords]
	refresh_unit(info_array, unit, node)

@rpc("authority", "call_local", "unreliable")
func refresh_extra_unit(info_array: Array):
	var coords = info_array[1]
	var node = get_node(str(coords) + "extra")
	var unit = extra_unit_data[coords]
	refresh_unit(info_array, unit, node)

func refresh_unit(info_array: Array, unit: base_unit, node):
	var coords = info_array[1]
	if selected_unit != null and coords == selected_unit.get_location():
		map.update_info_window(info_array)
	unit.update_stats(info_array)
	var morale_bar: ProgressBar = node.get_node("MoraleBar")
	morale_bar.value = info_array[4]
	var manpower_label: RichTextLabel = node.get_node("Manpower_Label")
	manpower_label.text = "[center]" + str(info_array[3]) + "[/center]"

func get_unit_client_array(coords: Vector2i) -> Array:
	if unit_data.has(coords):
		return unit_data[coords].convert_to_client_array()
	return []

@rpc("any_peer", "call_local", "unreliable")
func check_before_create(coords: Vector2i, type: int, player_id: int):
	pass

@rpc("authority", "call_remote", "unreliable")
func create_unit(coords: Vector2i, type: int, player_id: int):
	set_cell(coords, 0, Vector2i(0, type))
	unit_data[coords] = client_unit.new(coords, player_id, Vector2i(0, type))
	var unit_class = unit_creator.get_unit_class(type)
	create_label(coords, str(unit_class.toString()))
	request_refresh.rpc_id(1, coords)

func create_label(coords: Vector2i, text: String):
	var label: Label = Label.new()
	label.name = "Label"
	var node = Control.new()
	add_child(node)
	node.add_child(label)
	node.name = str(coords)
	label.text = text
	move_label_to_normal(coords, coords)
	label.position = Vector2(-label.size.x / 2, label.size.y)
	var morale_bar = create_morale_bar(label.size)
	node.add_child(morale_bar)
	var manpower_label = create_manpower_label(label.size)
	node.add_child(manpower_label)

func create_morale_bar(size: Vector2) -> ProgressBar:
	var morale_bar: ProgressBar = ProgressBar.new()
	morale_bar.name = "MoraleBar"
	morale_bar.show_percentage = false
	morale_bar.value = 100
	morale_bar.size = size
	morale_bar.position = Vector2(-size.x / 2, size.y * 2)
	var background_color = StyleBoxFlat.new()
	background_color.bg_color = Color(1, 1, 1, 0)
	var fill_color = StyleBoxFlat.new()
	fill_color.bg_color = Color(0.5, 1, 0.5, 1)
	morale_bar.add_theme_stylebox_override("background", background_color)
	morale_bar.add_theme_stylebox_override("fill", fill_color)
	return morale_bar

func create_manpower_label(size: Vector2) -> RichTextLabel:
	var manpower_label: RichTextLabel = RichTextLabel.new()
	manpower_label.name = "Manpower_Label"
	manpower_label.size = size
	manpower_label.bbcode_enabled = true
	manpower_label.text = "[center]" + str(0) + "[/center]"
	manpower_label.position = Vector2(-size.x / 2, size.y * 2)
	return manpower_label

@rpc("authority", "call_local", "unreliable")
func move_unit_to_regular(coords: Vector2i, move_to: Vector2i):
	var unit: base_unit = unit_data[coords]
	normal_move(coords)
	normal_arrival(unit, move_to)
	move_label_to_normal(coords, move_to)
	if unit.get_destination() == null:
		map.clear_highlights()
	check_extra(coords)

@rpc("authority", "call_local", "unreliable")
func move_unit_to_extra(coords: Vector2i, move_to: Vector2i):
	var unit: base_unit = unit_data[coords]
	normal_move(coords)
	extra_arrival(unit, move_to)
	move_label_to_extra(coords, move_to)
	if unit.get_destination() == null:
		map.clear_highlights()
	check_extra(coords)

@rpc("authority", "call_local", "unreliable")
func move_extra_unit_to_regular(coords: Vector2i, move_to: Vector2i):
	var unit: base_unit = extra_unit_data[coords]
	extra_move(coords)
	normal_arrival(unit, move_to)
	move_extra_label_to_normal(coords, move_to)
	if unit.get_destination() == null:
		map.clear_highlights()

@rpc("authority", "call_local", "unreliable")
func move_extra_unit_to_extra(coords: Vector2i, move_to: Vector2i):
	var unit: base_unit = extra_unit_data[coords]
	extra_move(coords)
	extra_arrival(unit, move_to)
	move_extra_label_to_extra(coords, move_to)
	if unit.get_destination() == null:
		map.clear_highlights()

func normal_move(coords: Vector2i):
	erase_cell(coords)
	unit_data.erase(coords)

func extra_move(coords: Vector2i):
	extra_unit_data.erase(coords)

func normal_arrival(unit: base_unit, move_to: Vector2i):
	var unit_atlas = unit.get_atlas_coord()
	unit.set_location(move_to)
	set_cell(move_to, 0, unit_atlas)
	unit_data[move_to] = unit

func extra_arrival(unit: base_unit, move_to: Vector2i):
	unit.set_location(move_to)
	extra_unit_data[move_to] = unit

func check_extra(coords: Vector2i):
	if !unit_data.has(coords) and extra_unit_data.has(coords):
		var unit = extra_unit_data[coords]
		extra_move(coords)
		normal_arrival(unit, coords)
		move_extra_label_to_normal(coords, coords)

func move_label_to_normal(coords: Vector2i, move_to: Vector2i):
	var node = get_node(str(coords))
	node.name = str(move_to)
	node.position = map_to_local(move_to)

func move_label_to_extra(coords: Vector2i, move_to: Vector2i):
	var node = get_node(str(coords))
	node.name = str(move_to) + "extra"
	node.position = map_to_local(move_to)
	node.position.y += 50

func move_extra_label_to_normal(coords: Vector2i, move_to: Vector2i):
	var node = get_node(str(coords) + "extra")
	node.name = str(move_to)
	node.position = map_to_local(move_to)

func move_extra_label_to_extra(coords: Vector2i, move_to: Vector2i):
	var node = get_node(str(coords) + "extra")
	node.name = str(move_to) + "extra"
	node.position = map_to_local(move_to)
	node.position.y += 50

func select_unit(coords: Vector2i, player_id: int):
	unhightlight_name()
	if selected_unit != null and selected_unit.get_location() == coords:
		cycle_unit_selection(coords)
		$select_unit_sound.play(0.5)
		state_machine.click_unit()
	elif unit_is_owned(coords):
		selected_unit = unit_data[coords]
		var soldier_atlas = get_cell_atlas_coords(coords)
		if soldier_atlas != Vector2i(-1, -1):
			$select_unit_sound.play(0.5)
			state_machine.click_unit()
	else:
		selected_unit = null
		map.close_unit_box()
	highlight_dest()
	highlight_name()

func cycle_unit_selection(coords: Vector2i):
	if unit_is_bottom(selected_unit):
		selected_unit = unit_data[coords]
	else:
		selected_unit = extra_unit_data[coords]

func unit_is_bottom(unit: base_unit) -> bool:
	if unit != null:
		var coords = unit.get_location()
		return extra_unit_data.has(coords) and extra_unit_data[coords] == unit
	return false

func highlight_name():
	apply_color_to_selected_unit(Color(1, 0, 0, 1))

func unhightlight_name():
	apply_color_to_selected_unit(Color(1, 1, 1, 1))

func apply_color_to_selected_unit(color: Color):
	if selected_unit == null:
		return
	var coords = selected_unit.get_location()
	var node = get_node(str(coords))
	if unit_is_bottom(selected_unit):
		node = get_node(str(coords) + "extra")
	if unit_is_owned(coords):
		var unit_name: Label = node.get_node("Label")
		unit_name.add_theme_color_override("font_color", color)

func highlight_cell(coords: Vector2i):
	map.highlight_cell(coords)


func highlight_dest():
	if selected_unit != null:
		var coords = selected_unit.get_location()
		if selected_unit.get_destination() != null and unit_is_owned(coords):
			map.highlight_cell(selected_unit.get_destination())
		else:
			map.clear_highlights()
	else:
		map.clear_highlights()

func unit_is_owned(coords: Vector2i) -> bool:
	return unit_data.has(coords) and unit_data[coords].get_player_id() == multiplayer.get_unique_id()

func selected_unit_exists_and_owned(unique_id: int) -> bool:
	return selected_unit != null and selected_unit.get_player_id() == unique_id

#Moving Units
func set_up_set_unit_route(unit: base_unit, move_to: Vector2i):
	var coords = unit.get_location()
	if unit == null:
		return
	elif unit_is_bottom(unit):
		set_selected_unit_route.rpc_id(1, coords, true, move_to)
		set_selected_unit_route(coords, true, move_to)
	else:
		set_selected_unit_route.rpc_id(1, coords, false, move_to)
		set_selected_unit_route(coords, false, move_to)

@rpc("any_peer", "call_local", "unreliable")
func set_selected_unit_route(_coords: Vector2i, _extra: bool, move_to: Vector2i):
	$dest_sound.play(0.3)
	highlight_cell(move_to)

@rpc("authority", "call_local", "unreliable")
func set_normal_unit_route(coords: Vector2i, move_to: Vector2i):
	request_refresh.rpc_id(1, coords)

@rpc("authority", "call_local", "unreliable")
func set_extra_unit_route(coords: Vector2i, move_to: Vector2i):
	request_refresh.rpc_id(1, coords)

func get_selected_coords() -> Vector2i:
	return selected_unit.get_location()

func is_unit_double_clicked(coords: Vector2i, unique_id: int):
	var toReturn = get_selected_coords() == coords and selected_unit_exists_and_owned(unique_id)
	if toReturn:
		request_refresh.rpc_id(1, coords)
	if extra_unit_data.has(coords):
		return false
	return toReturn

@rpc("authority", "call_local", "unreliable")
func kill_normal_unit(coords: Vector2i):
	var node: Control = get_node(str(coords))
	unit_data[coords].queue_free()
	unit_data.erase(coords)
	clean_up_node(node)
	erase_cell(coords)

@rpc("authority", "call_local", "unreliable")
func kill_extra_unit(coords: Vector2i):
	var node: Control = get_node(str(coords) + "extra")
	extra_unit_data[coords].queue_free()
	extra_unit_data.erase(coords)
	clean_up_node(node)

func clean_up_node(node):
	for child in node.get_children():
		child.queue_free()
	node.queue_free()
