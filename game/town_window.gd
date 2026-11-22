extends Window
var location: Variant = null
var hold_name: String
var current_cargo: Dictionary

var town_info: Dictionary
var ind_to_type: Array = []

var current_pops: int
var type_hovered: int = -1
var type_selected: int = -1
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
	create_graph_pdh()

func refresh_window() -> void:
	if location != null:
		request_current_name.rpc_id(1, location)
		request_current_prices.rpc_id(1, location)
		#request_current_pops.rpc_id(1, location)
		request_factories.rpc_id(1, location)

@rpc("any_peer", "call_local", "unreliable")
func request_current_name(coords: Vector2i) -> void:
	var current_name: String = map_data.get_instance().get_hold_name(coords)
	update_current_name.rpc_id(multiplayer.get_remote_sender_id(), current_name)

@rpc("any_peer", "call_local", "unreliable")
func request_current_prices(coords: Vector2i) -> void:
	var dict: Dictionary = ProvinceManager.get_instance().get_town_phps(coords)
	update_current_prices.rpc_id(multiplayer.get_remote_sender_id(), dict)

@rpc("any_peer", "call_local", "unreliable")
func request_current_pops(coords: Vector2i) -> void:
	var _current_pops: int = (TerminalMap.get_instance().get_town(coords)).get_total_pops()
	update_current_pops.rpc_id(multiplayer.get_remote_sender_id(), _current_pops)

@rpc("any_peer", "call_local", "unreliable")
func request_factories(coords: Vector2i) -> void:
	var factories: Array = ProvinceManager.get_instance().get_town_factories(coords)
	update_factories.rpc_id(multiplayer.get_remote_sender_id(), factories)

@rpc("authority", "call_local", "unreliable")
func update_current_name(new_name: String) -> void:
	hold_name = new_name
	$Name.text = "[center][font_size=30]" + hold_name + "[/font_size][/center]"

@rpc("authority", "call_local", "unreliable")
func update_current_prices(new_prices: Dictionary) -> void:
	town_info = new_prices
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
	ind_to_type.clear()
	for type: int in town_info:
		var pdh: PDH = town_info[type]
		ind_to_type.push_back(type)
		var text: String = names[type] + ": " + pdh.to_string()
		if num < price_list.item_count:
			var prev: String = price_list.get_item_text(num).trim_prefix(names[type] + ": ")
			price_list.set_item_text(num, text)
			var prev_price: float = 0.0
			if prev.is_valid_float():
				prev_price = float(prev)
			set_color(num, pdh.price, prev_price)
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
		var ind: int = $Price_Node/Price_List.get_item_at_position(local_pos, true)
		if ind == -1:
			return
		
		start_hovering_type(ind_to_type[ind])
	else:
		$CargoInfoPopUp.hide()

func start_hovering_type(type: int) -> void:
	if type != type_hovered:
		$CargoInfoPopUp.start_hover()
		type_hovered = type

func _on_cargo_info_pop_up_popup_requested() -> void:
	populate_info_window.rpc_id(1, type_hovered)

@rpc("any_peer", "call_local", "unreliable")
func populate_info_window(type: int) -> void:
	var info: Dictionary = {}
	var pdh: PDH = town_info[type]
	
	info.type = CargoInfo.get_instance().get_cargo_name(type)
	
	info.price = "$" + str(Utils.round(pdh.price, 2))
	info.supply = "Supply: " + str(pdh.supply)
	info.demand = "Demand: " + str(pdh.demand)
	
	pop_up_info_window.rpc_id(multiplayer.get_remote_sender_id(), info)

func create_graph_pdh() -> void:
	var graph: ColorRect = $Graph
	for child: Node in graph.get_children():
		if (child is ColorRect):
			graph.remove_child(child)
			child.free()
	
	if type_selected == -1:
		return
	
	graph.get_node("TypeLabel").text = CargoInfo.get_instance().get_cargo_name(type_selected)
	
	var price_bounds: Vector2 = Vector2(INF, 0)
	var amount_bounds: Vector2 = Vector2(INF, 0)
	
	var pdh: PDH = town_info[type_selected]
	for i: int in pdh.sale_history.size():
		var amt: float = pdh.sale_history[i]
		if amt == 0: continue
		var price: float = i / 2.0
		amount_bounds.x = min(amount_bounds.x, amt)
		amount_bounds.y = max(amount_bounds.y, amt)
		price_bounds.x = min(price_bounds.x, price)
		price_bounds.y = max(price_bounds.y, price)
	
	amount_bounds.x *= 0.8
	price_bounds.x *= 0.8
	
	amount_bounds.y *= 1.25
	price_bounds.y *= 1.25
	
	for i: int in pdh.sale_history.size():
		var amt: float = pdh.sale_history[i]
		if amt == 0: continue
		var price: float = i / 2.0
		var point: Vector2 = Vector2(amt, price)
		create_point(point, false, price_bounds, amount_bounds)
	
	var bottomBorder: ColorRect = $Graph/Borders/BottomBorder
	var leftBorder: ColorRect = $Graph/Borders/LeftBorder
	
	var amt_diff: float = amount_bounds.y - amount_bounds.x
	(bottomBorder.get_node("1Mark/Label") as Label).text = str(Utils.round(amount_bounds.y, 2))
	(bottomBorder.get_node("3_4Mark/Label") as Label).text = str(Utils.round(amount_bounds.y - amt_diff * 1/4.0, 2))
	(bottomBorder.get_node("1_2Mark/Label") as Label).text = str(Utils.round(amount_bounds.y - amt_diff * 1/2.0, 2))
	(bottomBorder.get_node("1_4Mark/Label") as Label).text = str(Utils.round(amount_bounds.y - amt_diff * 3/4.0, 2))
	
	var price_diff: float = price_bounds.y - price_bounds.x
	(leftBorder.get_node("1Mark/Label") as Label).text = str(Utils.round(price_bounds.y, 2))
	(leftBorder.get_node("3_4Mark/Label") as Label).text = str(Utils.round(price_bounds.y - price_diff * 1/4.0, 2))
	(leftBorder.get_node("1_2Mark/Label") as Label).text = str(Utils.round(price_bounds.y - price_diff * 1/2.0, 2))
	(leftBorder.get_node("1_4Mark/Label") as Label).text = str(Utils.round(price_bounds.y - price_diff * 3/4.0, 2))

func create_graph_pdp() -> void:
	var graph: ColorRect = $Graph
	for child: Node in graph.get_children():
		if (child is ColorRect):
			graph.remove_child(child)
			child.free()
	
	if type_selected == -1:
		return
	
	graph.get_node("TypeLabel").text = CargoInfo.get_instance().get_cargo_name(type_selected)
	
	var price_bounds: Vector2 = Vector2(INF, 0)
	var amount_bounds: Vector2 = Vector2(INF, 0)
	
	var pdp: PDP = town_info[type_selected]
	for vec: Vector2 in pdp.buy_orders:
		var amt: float = vec.x
		var price: float = vec.y
		amount_bounds.x = min(amount_bounds.x, amt)
		amount_bounds.y = max(amount_bounds.y, amt)
		price_bounds.x = min(price_bounds.x, price)
		price_bounds.y = max(price_bounds.y, price)
		
	for vec: Vector2 in pdp.sell_orders:
		var amt: float = vec.x
		var price: float = vec.y
		amount_bounds.x = min(amount_bounds.x, amt)
		amount_bounds.y = max(amount_bounds.y, amt)
		price_bounds.x = min(price_bounds.x, price)
		price_bounds.y = max(price_bounds.y, price)
	
	amount_bounds.x *= 0.8
	price_bounds.x *= 0.8
	
	amount_bounds.y *= 1.25
	price_bounds.y *= 1.25
	
	for vec: Vector2 in pdp.buy_orders:
		create_point(vec, true, price_bounds, amount_bounds)
	
	for vec: Vector2 in pdp.sell_orders:
		create_point(vec, false, price_bounds, amount_bounds)
	
	var bottomBorder: ColorRect = $Graph/Borders/BottomBorder
	var leftBorder: ColorRect = $Graph/Borders/LeftBorder
	
	var amt_diff: float = amount_bounds.y - amount_bounds.x
	(bottomBorder.get_node("1Mark/Label") as Label).text = str(amount_bounds.y)
	(bottomBorder.get_node("3_4Mark/Label") as Label).text = str(amount_bounds.y - amt_diff * 1/4.0)
	(bottomBorder.get_node("1_2Mark/Label") as Label).text = str(amount_bounds.y - amt_diff * 1/2.0)
	(bottomBorder.get_node("1_4Mark/Label") as Label).text = str(amount_bounds.y - amt_diff * 3/4.0)
	
	var price_diff: float = price_bounds.y - price_bounds.x
	(leftBorder.get_node("1Mark/Label") as Label).text = str(Utils.round(price_bounds.y, 2))
	(leftBorder.get_node("3_4Mark/Label") as Label).text = str(Utils.round(price_bounds.y - price_diff * 1/4.0, 2))
	(leftBorder.get_node("1_2Mark/Label") as Label).text = str(Utils.round(price_bounds.y - price_diff * 1/2.0, 2))
	(leftBorder.get_node("1_4Mark/Label") as Label).text = str(Utils.round(price_bounds.y - price_diff * 3/4.0, 2))

func create_point(point: Vector2, buy: bool, price_bounds: Vector2, amount_bounds: Vector2) -> void:
	var graph: ColorRect = $Graph
	var graph_size: Vector2 = graph.size
	#var graph_bl_point: Vector2 = graph.position - Vector2(0, graph_size.y)
	
	var point_texture: ColorRect = ColorRect.new()
	point_texture.size = Vector2(6, 6)
	
	point_texture.position -= point_texture.size / 2
	point_texture.position.y += graph_size.y
	
	point_texture.position.x += (point.x - amount_bounds.x) / (amount_bounds.y - amount_bounds.x) * graph_size.x
	point_texture.position.y -= (point.y - price_bounds.x) / (price_bounds.y - price_bounds.x) * graph_size.y
	
	point_texture.color = Color.GREEN if buy else Color.RED
	
	graph.add_child(point_texture)

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

func _on_price_list_item_clicked(index: int, _at_position: Vector2, _mouse_button_index: int) -> void:
	var diff: bool = type_selected != ind_to_type[index]
	type_selected = ind_to_type[index]
	if diff:
		create_graph_pdh()

func _on_sub_farm_button_pressed() -> void:
	hide()
	Utils.world_map.get_node("subsistence_farm_window").open_window(location)
