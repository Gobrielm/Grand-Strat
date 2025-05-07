extends Window

var coords: Vector2i
var current_tab: int

func open_window(_coords: Vector2i) -> void:
	position = Vector2i(0, 0)
	coords = _coords
	if current_tab == 0:
		open_province_window()
	else:
		open_state_window()
	popup()

func open_state_window() -> void:
	$Province_control.visible = false
	request_province_id.rpc_id(1, coords)
	request_province_pop.rpc_id(1, coords)
	$State_control.visible = true

func open_province_window() -> void:
	$State_control.visible = false
	#var resolution: Vector2i = get_tree().root.content_scale_size
	$Province_control/Coords.text = str(coords)
	request_biome.rpc_id(1, coords)
	request_resources_available.rpc_id(1, coords)
	$Province_control.visible = true

@rpc("any_peer", "call_local")
func request_province_id(p_coords: Vector2i) -> void:
	set_province_id.rpc_id(multiplayer.get_remote_sender_id(), map_data.get_instance().get_province_id(p_coords))

@rpc("authority", "call_local")
func set_province_id(id: int) -> void:
	$State_control/Province_ID.text = str(id)

@rpc("any_peer", "call_local")
func request_province_pop(p_coords: Vector2i) -> void:
	set_province_pop.rpc_id(multiplayer.get_remote_sender_id(), map_data.get_instance().get_province_population(p_coords))

@rpc("authority", "call_local")
func set_province_pop(pop: int) -> void:
	$State_control/Population.text = str(pop)

@rpc("any_peer", "call_local")
func request_biome(_coords: Vector2i) -> void:
	set_biome.rpc_id(multiplayer.get_remote_sender_id(), Utils.world_map.get_biome_name(_coords))

@rpc("authority", "call_local")
func set_biome(biome: String) -> void:
	$Province_control/Biome.text = biome

@rpc("any_peer", "call_local")
func request_population(_coords: Vector2i) -> void:
	pass

@rpc("authority", "call_local")
func set_population(num: int) -> void:
	$Province_control/Population.text = str(num)

@rpc("any_peer", "call_local")
func request_resources_available(_coords: Vector2i) -> void:
	var resource_dict: Dictionary = terminal_map.get_available_resources(_coords)
	set_resources_available.rpc_id(multiplayer.get_remote_sender_id(), resource_dict)

@rpc("authority", "call_local")
func set_resources_available(resource_dict: Dictionary) -> void:
	$Province_control/ItemList.clear()
	for type: int in resource_dict:
		var mag: int = resource_dict[type]
		$Province_control/ItemList.add_item(terminal_map.get_cargo_name(type) + " - " + str(mag))
	
func _on_close_requested() -> void:
	hide()

func _on_close_pressed() -> void:
	hide()

func _on_tab_bar_tab_changed(tab: int) -> void:
	current_tab = tab
	open_window(coords)
