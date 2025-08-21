extends Node

signal map_creation_finished
signal map_creation_progress

var creation_thread: Thread

@onready var tile_ownership_obj: tile_ownership = $tile_ownership
@onready var main_map: TileMapLayer = $main_map
@onready var camera: Camera2D = $main_map/player_camera
@onready var factory_window: Window = $main_map/factory_window
@onready var depot_window: Window = $main_map/depot_window
@onready var station_window: Window = $main_map/station_window
@onready var viewer_station_window: Window = $main_map/viewer_station_window
@onready var unit_creator_window: Window = $main_map/unit_creator_window
@onready var factory_recipe_window: Window = $main_map/factory_recipe_gui
@onready var factory_construction_window: Window = $main_map/factory_construction_gui
@onready var town_window: Window = $main_map/town_window
@onready var cargo_map: Node = $cargo_map

var unique_id: int

func _ready() -> void:
	randomize()
	unique_id = multiplayer.get_unique_id()
	await get_tree().process_frame
	await get_tree().process_frame
	main_map.initialize_game()
	set_up()
	await get_tree().process_frame
	await get_tree().process_frame
	creation_thread = Thread.new()
	creation_thread.start(initialize_game)
	#initialize_game()

func sync_creation_thread() -> void:
	creation_thread.wait_to_finish()
	map_creation_finished.emit()

func update_map_creation_progress(progress: float) -> void:
	map_creation_progress.emit(progress)

func set_up() -> void:
	#Singleton Creation
	train_manager.new()
	ai_manager.new()
	TerminalMap.get_instance().assign_cargo_map(cargo_map)
	cargo_map.cargo_values.create_magnitude_layers()

func initialize_game() -> void:
	#Main map initialization is very basic, do first
	call_deferred("update_map_creation_progress", 20)
	if unique_id != 1:
		remove_child(cargo_map)
		cargo_map.queue_free()
		cargo_map = preload("res://Singletons/Client_Singletons/client_cargo_map.tscn").instantiate()
		add_child(cargo_map)
		remove_child(tile_ownership_obj)
		tile_ownership_obj.queue_free()
		tile_ownership_obj = preload("res://Singletons/Client_Singletons/client_tile_ownership.tscn").instantiate()
		tile_ownership_obj.name = "tile_ownership"
		add_child(tile_ownership_obj)
	else:
		#First provences and population
		Utils.cargo_values.create_provinces_and_pop()
		call_deferred("update_map_creation_progress", 37)
		#Then countries
		tile_ownership_obj.create_countries()
		call_deferred("update_map_creation_progress", 65)
		#Then resources and industries that need both of those
		cargo_map.place_resources(main_map) # Creates resources and towns
		call_deferred("update_map_creation_progress", 85)
		#cargo_map.test()
		#Then create pops which needs towns
		ProvinceManager.get_instance().create_pops()
		call_deferred("update_map_creation_progress", 95)
		cargo_map.add_industries_to_towns()
	enable_nation_picker()
	call_deferred("update_map_creation_progress", 100)
	call_deferred("sync_creation_thread")

func _input(event: InputEvent) -> void:
	if !TerminalMap.is_instance_created():
		return
	var cell_position: Vector2i = get_cell_position()
	main_map.update_hover()
	camera.update_coord_label(cell_position)
	if event.is_action_pressed("click"):
		main_map.record_start()
		if state_machine.is_picking_nation():
			pick_nation()
		elif state_machine.is_building():
			main_map.record_hover_click()
		elif state_machine.is_building_units():
			main_map.create_unit()
		elif state_machine.is_building_factory():
			create_factory()
		elif state_machine.is_building_road_depot():
			main_map.place_road_depot()
	elif event.is_action_released("click"):
		if state_machine.is_controlling_camera() and TerminalMap.get_instance().is_road_depot(cell_position):
			pass
		elif state_machine.is_controlling_camera() and TerminalMap.get_instance().is_owned_station(cell_position, unique_id):
			station_window.open_window(cell_position)
		elif state_machine.is_controlling_camera() and TerminalMap.get_instance().is_station(cell_position):
			viewer_station_window.open_window(cell_position)
		elif state_machine.is_controlling_camera() and TerminalMap.get_instance().is_owned_recipeless_construction_site(cell_position):
			factory_recipe_window.open_window(cell_position)
		elif state_machine.is_controlling_camera() and TerminalMap.get_instance().is_owned_construction_site(cell_position):
			factory_construction_window.open_window(cell_position)
		elif state_machine.is_controlling_camera() and TerminalMap.get_instance().is_factory(cell_position):
			factory_window.open_window(cell_position)
		elif state_machine.is_controlling_camera() and TerminalMap.get_instance().is_town(cell_position):
			town_window.open_window(cell_position)
		elif state_machine.is_controlling_camera() and map_data.get_instance().is_owned_depot(cell_position, unique_id):
			factory_window.open_window(cell_position)
			#depot_window.open_window(cell_position)
		elif state_machine.is_building_many_rails():
			main_map.place_rail_to_start()
		elif state_machine.is_building_roads():
			main_map.place_road_to_start()
		elif state_machine.is_controlling_camera():
			main_map.open_tile_window(cell_position)
		main_map.reset_start()
	elif event.is_action_pressed("deselect"):
		main_map.clear_all_temps()
		if !state_machine.is_picking_nation() and !state_machine.is_selecting_unit():
			camera.unpress_all_buttons()
			state_machine.default()
	elif event.is_action_pressed("debug_place_train") and state_machine.is_controlling_camera():
		main_map.create_train.rpc(cell_position)
	elif event.is_action_pressed("debug_print") and state_machine.is_controlling_camera():
		unit_creator_window.popup()
	elif event.is_action_pressed("debug_ai_cycle") and state_machine.is_controlling_camera():
		cargo_map.ai_cycle()
		#ai_manager.get_instance()._on_ai_timer_timeout()

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
	tile_ownership_obj.add_player_to_country.rpc_id(1, coords)

#Map Commands
func get_cell_position() -> Vector2i:
	return main_map.get_cell_position()

#BackBuff
func idk() -> void:
	pass
	#$BackBufferCopy
