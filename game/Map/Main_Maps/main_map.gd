extends TileMapLayer
var start: Vector2i = Vector2i(0, 0)
var is_start_valid: bool = false
var unique_id: int
#Buttons
@onready var camera: Camera2D = $player_camera
@onready var track_button: Button = $player_camera/CanvasLayer/track_button
@onready var station_button: Button = $player_camera/CanvasLayer/station_button
@onready var depot_button: Button = $player_camera/CanvasLayer/depot_button
@onready var single_track_button: Button = $player_camera/CanvasLayer/single_track_button
@onready var unit_creator_window: Window = $unit_creator_window
@onready var tile_window: Window = $tile_window
@onready var game: Node = get_parent().get_parent()
@onready var rail_placer: Node = $Rail_Placer
@onready var map_node: Node = get_parent()
var unit_map: TileMapLayer
var untraversable_tiles: Dictionary = {}
var visible_tiles: Array = []

const train_scene_client: PackedScene = preload('res://Client_Objects/client_train.tscn')
const depot: Script = preload("res://Cargo/vehicle_depot.gd")

var testing: Node

var heart_beat: Dictionary = {}


func _on_timer_timeout() -> void:
	if unique_id == 1:
		for peer: int in multiplayer.get_peers():
			if heart_beat[peer] != 0:
				camera.update_desync_label(heart_beat[peer])
			heart_beat[peer] += 1
		server_heart_beat()

func server_heart_beat() -> void:
	send_heart_beat.rpc()

@rpc("authority", "call_remote", "unreliable")
func send_heart_beat() -> void:
	recognize_heart_beat.rpc_id(1)

@rpc("any_peer", "call_remote", "unreliable")
func recognize_heart_beat() -> void:
	heart_beat[multiplayer.get_remote_sender_id()] -= 1

func _ready() -> void:
	unique_id = multiplayer.get_unique_id()
	map_data.new(self)
	if unique_id == 1:
		Utils.assign_world_map(self)
		create_untraversable_tiles()
		for peer: int in multiplayer.get_peers():
			heart_beat[peer] = 0
			update_money_label.rpc_id(peer, get_money(peer))
		update_money_label.rpc_id(1, get_money(1))
		unit_map = load("res://Map/unit_map.tscn").instantiate()
		add_child(unit_map)
		for cell: Vector2i in get_used_cells():
			rail_placer.init_track_connection.rpc(cell)
		var cargo_controller: Node = load("res://Cargo/cargo_controller.tscn").instantiate()
		cargo_controller.assign_map_node(map_node)
		add_child(cargo_controller)
		terminal_map.create(self)
		recipe.create_set_recipes()
		$player_camera/CanvasLayer/Desync_Label.visible = true
		#testing = preload("res://Test/testing.gd").new(self)
		#start_test()
	else:
		unit_map = load("res://Client_Objects/client_unit_map.tscn").instantiate()
		unit_map.name = "unit_map"
		add_child(unit_map)
		for tile: Vector2i in get_used_cells():
			visible_tiles.append(tile)
	

#Testing
func is_testing() -> bool:
	return testing != null

func start_test() -> void:
	if Utils.is_ready():
		await get_tree().process_frame
		clear()
		create_testing_map()
		testing.test()
	else:
		call_deferred("start_test")

func create_testing_map() -> void:
	var tile_ownership_obj: tile_ownership = tile_ownership.get_instance()
	for i: int in range(-500, 500):
		for j: int in range(-500, 500):
			tile_ownership_obj.add_tile_to_country(Vector2i(i, j), 1)
			set_cell(Vector2i(i, j), 0, Vector2i(0, 0))

#Constants
func is_owned(player_id: int, coords: Vector2i) -> bool:
	return map_node.is_owned(player_id, coords)

func start_building_units() -> void:
	state_machine.start_building_units()

func is_controlling_camera() -> bool:
	return state_machine.is_controlling_camera()


#Units
func create_untraversable_tiles() -> void:
	untraversable_tiles[Vector2i(5, 0)] = 1
	untraversable_tiles[Vector2i(6, 0)] = 1
	untraversable_tiles[Vector2i(7, 0)] = 1
	untraversable_tiles[Vector2i(-1, -1)] = 1

func is_tile_traversable(tile_to_check: Vector2i) -> bool:
	var atlas_coords: Vector2i = get_cell_atlas_coords(tile_to_check)
	return !untraversable_tiles.has(atlas_coords)

func show_unit_info_window() -> void:
	var unit_info_array: Array = unit_map.get_unit_client_array(get_cell_position())
	$unit_info_window.show_unit(unit_info_array)

func update_info_window(unit_info_array: Array) -> void:
	$unit_info_window.update_unit(unit_info_array)

func create_unit() -> void:
	unit_map.check_before_create.rpc_id(1, get_cell_position(), unit_creator_window.get_type_selected(), unique_id)

@rpc("authority", "call_remote", "unreliable")
func refresh_unit_map(unit_tiles: Dictionary) -> void:
	unit_map.refresh_map(visible_tiles, unit_tiles)

func close_unit_box() -> void:
	$unit_info_window.hide()

func is_unit_double_clicked() -> bool:
	return unit_map.is_unit_double_clicked(get_cell_position(), unique_id)

#Tracks
func get_rail_type_selected() -> int:
	if single_track_button.active:
		return 0
	elif depot_button.active:
		return 1
	elif station_button.active:
		return 2
	return 3

func clear_all_temps() -> void:
	rail_placer.clear_all_temps()

func update_hover() -> void:
	var rail_type: int = get_rail_type_selected()
	if get_rail_type_selected() < 3:
		rail_placer.hover(get_cell_position(), rail_type, map_to_local(get_cell_position()), get_mouse_local_to_map())
	elif is_start_valid and track_button.active:
		get_rail_to_hover()

func record_hover_click() -> void:
	var coords: Vector2i = rail_placer.get_coordinates()
	var orientation: int = rail_placer.get_orientation()
	if single_track_button.active:
		place_rail_general(coords, orientation, 0)
	elif depot_button.active:
		place_rail_general(coords, orientation, 1)
	elif station_button.active:
		place_rail_general(coords, orientation, 2)

func get_depot_direction(coords: Vector2i) -> int:
	return rail_placer.get_depot_direction(coords)

func place_road_depot() -> void:
	if !state_machine.is_hovering_over_gui():
		rail_placer.place_road_depot(get_cell_position(), unique_id)

#Cargo
func is_depot(coords: Vector2i) -> bool:
	return map_data.get_instance().is_depot(coords)

func is_owned_depot(coords: Vector2i) -> bool:
	return map_data.get_instance().is_owned_depot(coords, unique_id)

func is_hold(coords: Vector2i) -> bool:
	return terminal_map.is_hold(coords)

func is_owned_hold(coords: Vector2i) -> bool:
	return map_data.get_instance().is_owned_hold(coords, unique_id)

func is_factory(coords: Vector2i) -> bool:
	return terminal_map.is_factory(coords)

func is_owned_station(coords: Vector2i) -> bool:
	return terminal_map.is_station(coords)

func is_location_valid_stop(coords: Vector2i) -> bool:
	return map_data.get_instance().is_hold(coords) or map_data.get_instance().is_depot(coords)

func get_depot_or_terminal(coords: Vector2i) -> terminal:
	var new_depot: terminal = map_data.get_instance().get_depot(coords)
	if new_depot != null:
		return new_depot
	return terminal_map.get_terminal(coords)

#Trains
@rpc("any_peer", "reliable", "call_local")
func create_train(coords: Vector2i) -> void:
	var caller: int = multiplayer.get_remote_sender_id()
	if !caller:
		caller = 1
	if unique_id == 1:
		train_manager.get_instance().create_train(coords, caller)
	else:
		var train_obj: Sprite2D = train_scene_client.instantiate()
		train_obj.name = "Train" + str(get_number_of_trains())
		add_child(train_obj)
		train_obj.create(coords)

func get_number_of_trains() -> int:
	var children: Array = get_children()
	var count: int = 0
	for child: Node in children:
		if child is train:
			count += 1
	return count

func get_trains() -> Array:
	var children: Array = get_children()
	var toReturn: Array = []
	for child: Node in children:
		if child is train:
			toReturn.push_back(child)
	return toReturn

func get_trains_in_depot(coords: Vector2i) -> Array:
	if map_data.get_instance().is_depot(coords):
		return map_data.get_instance().get_depot(coords).get_trains_simplified()
	return []

#Rail Builder
func record_start_rail() -> void:
	start = get_cell_position()
	is_start_valid = true

func reset_start() -> void:
	start = Vector2i(0, 0)
	is_start_valid = false

func place_rail_to_start() -> void:
	place_to_end_rail(start, get_cell_position())

func place_to_end_rail(new_start: Vector2i, new_end: Vector2i) -> void:
	start = new_start
	var end: Vector2i = new_end
	var queue: Array = []
	
	var current: Vector2i
	var prev: Vector2i
	queue.push_back(start)
	while !queue.is_empty():
		current = queue.pop_front()
		if current != start:
			var orientation: int = get_orientation(current, prev)
			place_rail_general(current, orientation, 0)
			place_rail_general(prev, (orientation + 3) % 6, 0)
		if current == end:
			break
		queue.push_back(find_tile_with_min_distance(get_surrounding_cells(current), end))
		prev = current
	reset_start()

func get_rail_to_hover() -> void:
	if !is_start_valid:
		return
	var end: Vector2i = get_cell_position()
	var toReturn: Dictionary = get_rails_from(start, end)
	rail_placer.hover_many_tiles(toReturn)

func get_rails_from(begin: Vector2i, end: Vector2i) -> Dictionary:
	var queue: Array = []
	var toReturn: Dictionary = {}
	var current: Vector2i
	var prev: Vector2i
	queue.push_back(begin)
	while !queue.is_empty():
		current = queue.pop_front()
		if current != begin:
			var orientation: int = get_orientation(current, prev)
			toReturn[current] = [Vector2i(orientation, 0)]
			if toReturn.has(prev):
				toReturn[prev].append(Vector2i((orientation + 3) % 6, 0))
			else:
				toReturn[prev] = [Vector2i((orientation + 3) % 6, 0)]
		if current == end:
			break
		queue.push_back(find_tile_with_min_distance(get_surrounding_cells(current), end))
		prev = current
	return toReturn

func find_tile_with_min_distance(tiles_to_check: Array[Vector2i], target_tile: Vector2i) -> Vector2i:
	var min_distance: float = 10000.0
	var to_return: Vector2i
	for cell: Vector2i in tiles_to_check:
		if cell.distance_to(target_tile) < min_distance:
			min_distance = cell.distance_to(target_tile)
			to_return = cell
	return to_return

func get_orientation(current: Vector2i, prev: Vector2i) -> int:
	var difference: Vector2i = map_to_local(current) - map_to_local(prev)
	difference.y *= -1
	if (difference.x == 0 and difference.y < 0):
		return 0
	elif (difference.x == 0 and difference.y > 0):
		return 3
	elif (difference.y >= 0 and difference.x < 0):
		return 2
	elif (difference.y < 0 and difference.x > 0):
		return 5
	elif (difference.y >= 0 and difference.x > 0):
		return 4
	elif (difference.y < 0 and difference.x < 0):
		return 1
	assert(false)
	return -1

#Map Functions
func get_cell_position() -> Vector2i:
	var cell_position: Vector2i = local_to_map(get_mouse_local_to_map())
	return cell_position

func get_mouse_local_to_map() -> Vector2:
	var camera_pos: Vector2 = camera.position
	return camera_pos + get_mouse_local_to_camera()

func get_mouse_local_to_camera() -> Vector2:
	var camera_middle: Vector2 = get_viewport_rect().size / 2
	var centered_at_top_left: Vector2 = get_viewport().get_mouse_position()
	var final: Vector2 = -camera_middle + centered_at_top_left
	return final / camera.zoom

func get_biome_name(coords: Vector2i) -> String:
	var biome_name: String = ""
	if is_desert(coords):
		biome_name += "Desert"
	elif is_tundra(coords):
		biome_name += "Tundra"
	elif is_dry(coords):
		biome_name += "Grasslands"
	elif is_water(coords):
		biome_name += "Ocean"
	else:
		biome_name += "Meadow"
	
	if is_forested(coords):
		biome_name += " Forest"
	
	if is_hilly(coords):
		biome_name += " Hills"
	elif is_mountainous(coords):
		if is_desert(coords):
			biome_name = "Desert Mountains"
		else:
			biome_name = "Mountains"
	return biome_name

func is_forested(coords: Vector2i) -> bool:
	var atlas: Vector2i = get_cell_atlas_coords(coords)
	if (atlas.y >= 0 and atlas.y <= 2) and (atlas.x == 1 or atlas.x == 2 or atlas.x == 4):
		return true
	return false

func is_hilly(coords: Vector2i) -> bool:
	var atlas: Vector2i = get_cell_atlas_coords(coords)
	if (atlas == Vector2i(3, 0) or atlas == Vector2i(4, 0) or atlas == Vector2i(5, 1) or atlas == Vector2i(3, 2) or atlas == Vector2i(4, 2) or atlas == Vector2i(1, 3)):
		return true
	return false

func is_mountainous(coords: Vector2i) -> bool:
	var atlas: Vector2i = get_cell_atlas_coords(coords)
	if (atlas == Vector2i(5, 0) or atlas == Vector2i(3, 3)):
		return true
	return false

func is_desert(coords: Vector2i) -> bool:
	var atlas: Vector2i = get_cell_atlas_coords(coords)
	if (atlas.y == 3):
		return true
	return false

func is_tundra(coords: Vector2i) -> bool:
	var atlas: Vector2i = get_cell_atlas_coords(coords)
	if (atlas.y == 2):
		return true
	return false

func is_water(coords: Vector2i) -> bool:
	var atlas: Vector2i = get_cell_atlas_coords(coords)
	if (atlas.y == 0 and atlas.x >= 6):
		return true
	return false

func is_dry(coords: Vector2i) -> bool:
	var atlas: Vector2i = get_cell_atlas_coords(coords)
	if (atlas.y == 1):
		return true
	return false

#Tile Effects
func highlight_cell(coords: Vector2i) -> void:
	clear_highlights()
	var highlight: Sprite2D = Sprite2D.new()
	highlight.texture = load("res://Map_Icons/selected.png")
	add_child(highlight)
	highlight.name = "highlight"
	highlight.position = map_to_local(coords)

func clear_highlights() -> void:
	if has_node("highlight"):
		var node: Sprite2D = get_node("highlight")
		remove_child(node)
		node.queue_free()

func make_cell_invisible(coords: Vector2i) -> void:
	set_cell(coords, 1, get_cell_atlas_coords(coords))

func make_cell_visible(coords: Vector2i) -> void:
	set_cell(coords, 0, get_cell_atlas_coords(coords))

#Money Stuff
func get_cash_of_firm(coords: Vector2i) -> int:
	return terminal_map.get_cash_of_firm(coords)

func get_money(id: int) -> int:
	return money_controller.get_instance().get_money(id)

@rpc("authority", "unreliable", "call_local")
func update_money_label(amount: int) -> void:
	camera.update_cash_label(amount)

#Tile Data
func get_tile_connections(coords: Vector2i) -> int:
	return rail_placer.get_track_connections(coords)

func request_tile_data(coordinates: Vector2i) -> TileData:
	return get_cell_tile_data(coordinates)

func do_tiles_connect(coord1: Vector2i, coord2: Vector2i) -> bool:
	return rail_placer.are_tiles_connected_by_rail(coord1, coord2, get_surrounding_cells(coord1))

func open_tile_window(coords: Vector2i) -> void:
	if !is_water(coords):
		tile_window.open_window(coords)
	else:
		tile_window.hide()

#Rail General
func remove_rail(coords: Vector2i, orientation: int, p_type: int) -> void:
	#TODO: Needed for testing, but should be updated and/or removed
	if unique_id == 1:
		rail_placer.remove_tile(coords, orientation, p_type)

@rpc("authority", "call_local", "unreliable")
func place_tile(coords: Vector2i, orientation: int, type: int, _new_owner: int) -> void:
	rail_placer.place_tile(coords, orientation, type)

func set_cell_rail_placer_request(coords: Vector2i, orientation: int, type: int, new_owner: int) -> void:
	set_cell_rail_placer_server.rpc_id(1, coords, orientation, type, new_owner)

@rpc("any_peer", "call_remote", "unreliable")
func set_cell_rail_placer_server(coords: Vector2i, orientation: int, type: int, new_owner: int) -> void:
	ai_manager.get_instance().acknowledge_pending_deferred_call(new_owner)
	if rail_check(coords) and is_owned(new_owner, coords) and !terminal_map.is_station(coords):
		place_tile.rpc(coords, (orientation) % 6, type, new_owner)
		if type == 1:
			encode_depot(coords, new_owner)
			var depot_name: String = map_data.get_instance().get_depot_name(coords)
			encode_depot_client.rpc(coords, depot_name, new_owner)
		elif type == 2:
			place_tile.rpc(coords, (orientation + 3) % 6, type, new_owner)
			encode_station.rpc(coords, new_owner)
			terminal_map.create_station(coords, new_owner)
	else:
		rail_placer.clear_all_temps()

@rpc("authority", "call_local", "unreliable")
func encode_depot(coords: Vector2i, new_owner: int) -> void:
	map_data.get_instance().add_depot(coords, depot.new(coords, new_owner))

@rpc("authority", "call_remote", "unreliable")
func encode_depot_client(coords: Vector2i, depot_name: String, new_owner: int) -> void:
	#TODO: Client map_data
	pass
	map_data.get_instance().add_depot(coords, depot.new(depot_name, new_owner))

@rpc("authority", "call_local", "unreliable")
func encode_station(coords: Vector2i, new_owner: int) -> void:
	map_data.get_instance().add_hold(coords, "Station", new_owner)

#Rails, Depot, Station
func place_rail_general(coords: Vector2i, orientation: int, type: int) -> void:
	if unique_id == 1:
		set_cell_rail_placer_server(coords, orientation, type, unique_id)
	else:
		set_cell_rail_placer_request(coords, orientation, type, unique_id)
	
func rail_check(coords: Vector2i) -> bool:
	var atlas_coords: Vector2i = get_cell_atlas_coords(coords)
	return !untraversable_tiles.has(atlas_coords)
