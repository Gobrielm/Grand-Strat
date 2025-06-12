extends Window
var location: Variant = null
var hold_name: String
var current_cargo: Dictionary
var current_prices: Dictionary
var current_cash: int
var current_pops: int

const time_every_update: int = 1
var progress: float = 0.0

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	hide()
# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	progress += delta
	if progress > time_every_update:
		progress = 0
		refresh_window()

func _on_close_requested() -> void:
	hide()

func open_window(new_location: Vector2i) -> void:
	location = new_location
	refresh_window()
	popup()

func refresh_window() -> void:
	if location != null:
		request_current_cargo.rpc_id(1, location)
		request_current_name.rpc_id(1, location)
		request_current_prices.rpc_id(1, location)
		request_current_cash.rpc_id(1, location)
		request_current_pops.rpc_id(1, location)
		request_factories.rpc_id(1, location)

@rpc("any_peer", "call_local", "unreliable")
func request_current_cargo(coords: Vector2i) -> void:
	var dict: Dictionary = TerminalMap.get_instance().get_cargo_dict(coords)
	update_current_cargo.rpc_id(multiplayer.get_remote_sender_id(), dict)

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
	var toUpdate: Array[Array] = []
	var town: Town = TerminalMap.get_instance().get_town(coords)
	if town != null:
		for fact: FactoryTemplate in town.get_factories():
			var details: Array = []
			details.append(fact.get_level())				 #[0]
			details.append(fact.get_cash())					 #[1]
			details.append(fact.get_recipe_as_string())		 #[2]
			toUpdate.push_back(details)
	update_factories(toUpdate)

@rpc("authority", "call_local", "unreliable")
func update_current_cargo(new_current_cargo: Dictionary) -> void:
	current_cargo = new_current_cargo
	factory_window()

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
func update_factories(info: Array[Array]) -> void:
	var fact_list: ItemList = $Factory_Node/Factory_List
	var num: int = 0
	for fact: Array in info:
		var text: String = "Factory/Level: " + str(fact[0]) + " /Cash: " + str(fact[1]) + "/ "
		text += fact[2]
		if num < fact_list.item_count:
			fact_list.set_item_text(num, text)
		else:
			fact_list.add_item(text, null, false)
		
		num += 1
		

func factory_window() -> void:
	var cargo_list: ItemList = $Cargo_Node/Cargo_List
	var names: Array = CargoInfo.get_instance().get_cargo_array()
	var selected_name: String = get_selected_name()
	
	for i: int in cargo_list.item_count:
		cargo_list.remove_item(0)
	for cargo: int in current_cargo.size():
		if current_cargo[cargo] != 0:
			var cargo_name: String = names[cargo]
			cargo_list.add_item(cargo_name + ", " + str(current_cargo[cargo]))
			if cargo_name == selected_name:
				cargo_list.select(cargo)

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

func get_selected_name() -> String:
	var cargo_list: ItemList = $Cargo_Node/Cargo_List
	var selected_items: Array = cargo_list.get_selected_items()
	if selected_items.size() > 0:
		var toReturn: String = ""
		for i: String in cargo_list.get_item_text(selected_items[0]):
			if i == ',':
				break
			toReturn += i
		return toReturn
	return ""
	
