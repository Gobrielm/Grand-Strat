extends Window

var coords: Vector2i
var current_tab: int

func _ready() -> void:
	hide()

func open_window(_coords: Vector2i) -> void:
	coords = _coords
	if current_tab == 0:
		open_tile_window()
	else:
		open_province_window()
	popup()

func open_tile_window() -> void:
	$Province_control.visible = false
	#var resolution: Vector2i = get_tree().root.content_scale_size
	$Tile_control/Coords.text = str(coords)
	request_biome.rpc_id(1, coords)
	request_resources_available.rpc_id(1, coords)
	$Tile_control.visible = true

func open_province_window() -> void:
	$Tile_control.visible = false
	request_province_id.rpc_id(1, coords)
	request_province_pop.rpc_id(1, coords)
	request_province_pops.rpc_id(1, coords)
	$Province_control.visible = true

@rpc("any_peer", "call_local")
func request_province_id(p_coords: Vector2i) -> void:
	set_province_id.rpc_id(multiplayer.get_remote_sender_id(), map_data.get_instance().get_province_id(p_coords))

@rpc("authority", "call_local")
func set_province_id(id: int) -> void:
	$Province_control/Province_ID.text = str(id)

@rpc("any_peer", "call_local")
func request_province_pop(p_coords: Vector2i) -> void:
	set_province_pop.rpc_id(multiplayer.get_remote_sender_id(), map_data.get_instance().get_province_population(p_coords))

@rpc("any_peer", "call_local")
func request_province_pops(p_coords: Vector2i) -> void:
	var map_data_obj: map_data = map_data.get_instance()
	var prov_id: int = map_data_obj.get_province_id(p_coords)
	var prov: province = map_data_obj.get_province(prov_id)
	set_province_pops.rpc_id(multiplayer.get_remote_sender_id(), prov.count_pops())

@rpc("authority", "call_local")
func set_province_pop(pop: int) -> void:
	$Province_control/Population.text = str(pop)

@rpc("authority", "call_local")
func set_province_pops(num: int) -> void:
	$Province_control/Pops.text = "Pops: " + str(num)

@rpc("any_peer", "call_local")
func request_biome(_coords: Vector2i) -> void:
	set_biome.rpc_id(multiplayer.get_remote_sender_id(), Utils.world_map.get_biome_name(_coords))

@rpc("authority", "call_local")
func set_biome(biome: String) -> void:
	$Tile_control/Biome.text = biome

@rpc("any_peer", "call_local")
func request_resources_available(_coords: Vector2i) -> void:
	var resource_dict: Dictionary = terminal_map.get_instance().get_available_resources(_coords)
	set_resources_available.rpc_id(multiplayer.get_remote_sender_id(), resource_dict)

@rpc("authority", "call_local")
func set_resources_available(resource_dict: Dictionary) -> void:
	$Tile_control/ItemList.clear()
	for type: int in resource_dict:
		var mag: int = resource_dict[type]
		$Tile_control/ItemList.add_item(terminal_map.get_instance().get_cargo_name(type) + " - " + str(mag))
	
func _on_close_requested() -> void:
	hide()

func _on_close_pressed() -> void:
	hide()

func _on_tab_bar_tab_changed(tab: int) -> void:
	current_tab = tab
	open_window(coords)
