extends Window

var coords: Vector2i
var current_tab: int

func open_window(_coords: Vector2i):
	position = Vector2i(0, 0)
	coords = _coords
	if current_tab == 0:
		open_province_window()
	else:
		open_state_window()
	popup()

func open_state_window():
	$Province_control.visible = false
	request_province_id.rpc_id(1, coords)
	request_province_pop.rpc_id(1, coords)
	$State_control.visible = true

func open_province_window():
	$State_control.visible = false
	#var resolution: Vector2i = get_tree().root.content_scale_size
	$Province_control/Coords.text = str(coords)
	request_biome.rpc_id(1, coords)
	request_resources_available.rpc_id(1, coords)
	$Province_control.visible = true

@rpc("any_peer", "call_local")
func request_province_id(coords: Vector2i):
	var tile_info = Utils.tile_info
	set_province_id.rpc_id(multiplayer.get_remote_sender_id(), tile_info.get_province_id(coords))

@rpc("authority", "call_local")
func set_province_id(id: int):
	$State_control/Province_ID.text = str(id)

@rpc("any_peer", "call_local")
func request_province_pop(coords: Vector2i):
	var tile_info = Utils.tile_info
	set_province_pop.rpc_id(multiplayer.get_remote_sender_id(), tile_info.get_province_population(coords))

@rpc("authority", "call_local")
func set_province_pop(pop: int):
	$State_control/Population.text = str(pop)

@rpc("any_peer", "call_local")
func request_biome(_coords: Vector2i):
	set_biome.rpc_id(multiplayer.get_remote_sender_id(), Utils.world_map.get_biome_name(_coords))

@rpc("authority", "call_local")
func set_biome(biome: String):
	$Province_control/Biome.text = biome

@rpc("any_peer", "call_local")
func request_population(_coords: Vector2i):
	pass

@rpc("authority", "call_local")
func set_population(num: int):
	$Province_control/Population.text = str(num)

@rpc("any_peer", "call_local")
func request_resources_available(_coords: Vector2i):
	var resource_dict := terminal_map.get_available_resources(_coords)
	set_resources_available.rpc_id(multiplayer.get_remote_sender_id(), resource_dict)

@rpc("authority", "call_local")
func set_resources_available(resource_dict: Dictionary):
	$Province_control/ItemList.clear()
	for type in resource_dict:
		var mag = resource_dict[type]
		$Province_control/ItemList.add_item(terminal_map.get_cargo_name(type) + " - " + str(mag))
	
func _on_close_requested():
	hide()

func _on_close_pressed():
	hide()

func _on_tab_bar_tab_changed(tab):
	current_tab = tab
	open_window(coords)
