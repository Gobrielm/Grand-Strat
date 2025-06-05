extends Window

var coords: Vector2i
@onready var market: ItemList = $Control/market


func _ready() -> void:
	hide()

func open_window(p_coords: Vector2i) -> void:
	coords = p_coords
	update_window()
	popup()

func update_window() -> void:
	request_market_info.rpc_id(1, coords)

# === Requests ===
@rpc("any_peer", "call_local", "unreliable")
func request_market_info(p_coords: Vector2i) -> void:
	var town: Town = terminal_map.get_instance().get_town(p_coords)
	if town != null:
		set_market_info.rpc_id(multiplayer.get_remote_sender_id(), town.get_current_hold(), town.get_supply(), town.get_demand())
	set_market_info.rpc_id(multiplayer.get_remote_sender_id(), {}, {}, {})


# === Recieves ===
@rpc("authority", "call_local", "unreliable")
func set_market_info(info: Dictionary, supply: Dictionary, demand: Dictionary) -> void:
	if supply.is_empty():
		return
	var term_singleton: terminal_map = terminal_map.get_instance()
	for index: int in range(0, info.size()):
		var text: String = term_singleton.get_cargo_name(index) + ": " + str(supply[index]) + "/" + str(demand[index]) + ": " +str(info[index])
		if market.item_count > index:
			market.set_item_text(index, text)
		else:
			market.add_item(text, null, false)

func _on_close_pressed() -> void:
	hide()

func _on_close_requested() -> void:
	hide()
