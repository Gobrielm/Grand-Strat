extends Window
var location: Variant = null
var hold_name: String
var current_cargo: Dictionary
var current_prices: Dictionary
var current_cash: int
var current_pops: int
var type_hovered: int = -1
var inside_price_list: bool = false

const time_every_update: int = 1
var progress: float = 0.0

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	hide()


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	if visible:
		progress += delta
		if progress > time_every_update:
			progress = 0
			refresh_window()
		refresh_hover()
	

func _on_close_requested() -> void:
	hide()

func open_window(new_location: Vector2i) -> void:
	location = new_location
	refresh_window()
	popup()

func refresh_window() -> void:
	if location != null:
		request_current_name.rpc_id(1, location)
		request_current_prices.rpc_id(1, location)
		request_current_cash.rpc_id(1, location)
		request_current_pops.rpc_id(1, location)
		request_factories.rpc_id(1, location)

@rpc("any_peer", "call_local", "unreliable")
func request_current_name(coords: Vector2i) -> void:
	var current_name: String = map_data.get_instance().get_hold_name(coords)
	update_current_name.rpc_id(multiplayer.get_remote_sender_id(), current_name)

@rpc("any_peer", "call_local", "unreliable")
func request_current_prices(coords: Vector2i) -> void:
	var dict: Dictionary = TerminalMap.get_instance().get_local_prices(coords)
	update_current_prices.rpc_id(multiplayer.get_remote_sender_id(), dict)

@rpc("any_peer", "call_local", "unreliable")
func request_current_cash(coords: Vector2i) -> void:
	var _current_cash: int = TerminalMap.get_instance().get_cash_of_firm(coords)
	update_current_cash.rpc_id(multiplayer.get_remote_sender_id(), _current_cash)

@rpc("any_peer", "call_local", "unreliable")
func request_current_pops(coords: Vector2i) -> void:
	var _current_pops: int = (TerminalMap.get_instance().get_town(coords)).get_total_pops()
	update_current_pops.rpc_id(multiplayer.get_remote_sender_id(), _current_pops)

@rpc("any_peer", "call_local", "unreliable")
func request_factories(coords: Vector2i) -> void:
	var factories: Array = MapObjectInfo.get_town_factories(coords)
	update_factories.rpc_id(multiplayer.get_remote_sender_id(), factories)

@rpc("authority", "call_local", "unreliable")
func update_current_name(new_name: String) -> void:
	hold_name = new_name
	$Name.text = "[center][font_size=30]" + hold_name + "[/font_size][/center]"

@rpc("authority", "call_local", "unreliable")
func update_current_cash(new_cash: int) -> void:
	current_cash = new_cash
	$Cash.text = "$" + str(current_cash)

@rpc("authority", "call_local", "unreliable")
func update_current_prices(new_prices: Dictionary) -> void:
	current_prices = new_prices
	display_current_prices()

@rpc("authority", "call_local", "unreliable")
func update_current_pops(new_pops: int) -> void:
	current_pops = new_pops
	$Pops.text = "Pops: " + str(current_pops)

@rpc("authority", "call_local", "unreliable")
func update_factories(info: Array) -> void:
	var fact_list: ItemList = $Factory_Node/Factory_List
	var num: int = 0
	for fact: Dictionary in info:
		var text: String = fact.recipe + "\n"
		text += "Level: " + str(fact.level) + " , Cash: " + str(fact.cash)
		
		if num < fact_list.item_count:
			fact_list.set_item_text(num, text)
		else:
			fact_list.add_item(text, null, false)
		
		num += 1

func display_current_prices() -> void:
	var price_list: ItemList = $Price_Node/Price_List
	var names: Array = CargoInfo.get_instance().get_cargo_array()
	var num: int = 0
	for type: int in current_prices:
		var text: String = names[type] + ": " + str(current_prices[type])
		if num < price_list.item_count:
			var prev: String = price_list.get_item_text(num).trim_prefix(names[type] + ": ")
			price_list.set_item_text(num, text)
			var prev_price: float = 0.0
			if prev.is_valid_float():
				prev_price = float(prev)
			set_color(num, current_prices[type], prev_price)
		else:
			price_list.add_item(text, null, false)
			$Price_Node/Price_List.set_item_tooltip_enabled(num, false)
		num += 1

func set_color(num: int, price: float, prev_price: float = 0.0) -> void:
	var price_list: ItemList = $Price_Node/Price_List
	if prev_price == 0.0 or abs(price - prev_price) < 0.01:
		price_list.set_item_custom_fg_color(num, Color(1, 1, 1))
		return
	if prev_price > price:
		price_list.set_item_custom_fg_color(num, Color(1, 0, 0))
	else:
		price_list.set_item_custom_fg_color(num, Color(0, 1, 0))

func refresh_hover() -> void:
	if inside_price_list:
		var local_pos: Vector2 = $Price_Node/Price_List.get_local_mouse_position()
		var type: int = $Price_Node/Price_List.get_item_at_position(local_pos, true)
		
		start_hovering_type(type)
	else:
		$CargoInfoPopUp.hide()

func start_hovering_type(type: int) -> void:
	if type != type_hovered:
		$CargoInfoPopUp.start_hover()
		type_hovered = type

func _on_cargo_info_pop_up_popup_requested() -> void:
	populate_info_window.rpc_id(1, type_hovered, location)

@rpc("any_peer", "call_local", "unreliable")
func populate_info_window(type: int, p_location: Vector2i) -> void:
	var info: Dictionary = MapObjectInfo.get_town_prices(p_location)[type]
	
	info.type = CargoInfo.get_instance().get_cargo_name(type)
	info.price = "$" + info["price"]
	info.amount = "Amount: " + info["supply"]
	info.market_info = "Demand: " + info["demand"]
	
	pop_up_info_window.rpc_id(multiplayer.get_remote_sender_id(), info)

@rpc("authority", "call_local", "unreliable")
func pop_up_info_window(info: Dictionary) -> void:
	var pop_up: cargo_info_popup = $CargoInfoPopUp
	pop_up.pop_up_info_window(info, get_mouse_position() + Vector2(position))

func _on_price_list_mouse_entered() -> void:
	inside_price_list = true

func _on_price_list_mouse_exited() -> void:
	inside_price_list = false
	$CargoInfoPopUp.stop_hover()
	type_hovered = -1
