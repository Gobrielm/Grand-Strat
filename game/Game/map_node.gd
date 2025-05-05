extends Node

@onready var tile_ownership_obj: tile_ownership = $tile_ownership
@onready var main_map: TileMapLayer = $main_map
@onready var camera: Camera2D = $main_map/player_camera
@onready var factory_window: Window = $main_map/factory_window
@onready var depot_window: Window = $main_map/depot_window
@onready var station_window: Window = $main_map/station_window
@onready var unit_creator_window: Window = $main_map/unit_creator_window
@onready var factory_recipe_window: Window = $main_map/factory_recipe_gui
@onready var factory_construction_window: Window = $main_map/factory_construction_gui
@onready var cargo_map: Node = $cargo_map

var unique_id: int

func _ready() -> void:
	randomize()
	unique_id = multiplayer.get_unique_id()
	if unique_id != 1:
		remove_child(cargo_map)
		cargo_map.queue_free()
		cargo_map = preload("res://Client_Objects/client_cargo_map.tscn").instantiate()
		add_child(cargo_map)
		remove_child(tile_ownership_obj)
		tile_ownership_obj.queue_free()
		tile_ownership_obj = preload("res://Client_Objects/client_tile_ownership.tscn").instantiate()
		tile_ownership_obj.name = "tile_ownership"
		add_child(tile_ownership_obj)
	else:
		#Singleton Creation
		train_manager.new()
		ai_manager.new()
		Utils.assign_world_map(main_map)
		terminal_map.assign_cargo_map(cargo_map)
		money_controller.new(multiplayer.get_peers())
	enable_nation_picker()
	cargo_map.place_resources(main_map)

func _input(event: InputEvent) -> void:
	main_map.update_hover()
	camera.update_coord_label(get_cell_position())
	if event.is_action_pressed("click"):
		if state_machine.is_picking_nation():
			pick_nation()
		elif state_machine.is_building():
			main_map.record_hover_click()
		elif state_machine.is_building_many_rails():
			main_map.record_start_rail()
		elif state_machine.is_building_units():
			main_map.create_unit()
		elif state_machine.is_building_factory():
			create_factory()
		elif state_machine.is_building_road_depot():
			main_map.place_road_depot()
		elif state_machine.is_selecting_unit() and main_map.is_unit_double_clicked():
			main_map.show_unit_info_window()
		else:
			if state_machine.is_selecting_unit():
				state_machine.unclick_unit()
			main_map.unit_map.select_unit(get_cell_position(), unique_id)
	elif event.is_action_released("click"):
		if state_machine.is_controlling_camera() and main_map.is_owned_station(get_cell_position()):
			station_window.open_window(get_cell_position())
		elif state_machine.is_controlling_camera() and terminal_map.is_owned_recipeless_construction_site(get_cell_position()):
			factory_recipe_window.open_window(get_cell_position())
		elif state_machine.is_controlling_camera() and terminal_map.is_owned_construction_site(get_cell_position()):
			factory_construction_window.open_window(get_cell_position())
		elif state_machine.is_controlling_camera() and main_map.is_factory(get_cell_position()):
			factory_window.open_window(get_cell_position())
		elif state_machine.is_controlling_camera() and main_map.is_owned_depot(get_cell_position()):
			depot_window.open_window(get_cell_position())
		elif state_machine.is_building_many_rails():
			main_map.place_rail_to_start()
		elif state_machine.is_controlling_camera():
			main_map.open_tile_window(get_cell_position())
		main_map.reset_start()
	elif event.is_action_pressed("deselect"):
		main_map.clear_all_temps()
		if !state_machine.is_picking_nation() and !state_machine.is_selecting_unit():
			camera.unpress_all_buttons()
		state_machine.default()
	elif event.is_action_pressed("debug_place_train") and state_machine.is_controlling_camera():
		main_map.create_train.rpc(get_cell_position())
	elif event.is_action_pressed("debug_print") and state_machine.is_controlling_camera():
		unit_creator_window.popup()

func _on_day_tick_timeout() -> void:
	pass

func _on_month_tick_timeout() -> void:
	pass

#Factory
func create_factory() -> void:
	create_factory_server.rpc_id(1, unique_id, get_cell_position())

@rpc("any_peer", "call_local", "unreliable")
func create_factory_server(building_id: int, coords: Vector2i) -> void:
	cargo_map.create_construction_site(building_id, coords)

#Tile_Ownership
func is_owned(player_id: int, coords: Vector2i) -> bool:
	return tile_ownership_obj.is_owned(player_id, coords)

#Nation_Picker
func enable_nation_picker() -> void:
	camera.get_node("CanvasLayer").visible = false
	state_machine.start_picking_nation()

func disable_nation_picker() -> void:
	camera.get_node("CanvasLayer").visible = true
	state_machine.stop_picking_nation()

func pick_nation() -> void:
	var coords: Vector2i = main_map.get_cell_position()
	tile_ownership_obj.add_player_to_country.rpc_id(1, unique_id, coords)

#Map Commands
func get_cell_position() -> Vector2i:
	return main_map.get_cell_position()

#BackBuff
func idk() -> void:
	pass
	#$BackBufferCopy
