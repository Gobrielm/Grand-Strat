class_name terminal_map extends Node
#Represents Singleton

static var singleton_instance: terminal_map
	
func _init(_map: TileMapLayer) -> void:
	assert(singleton_instance == null, "Cannot create multiple instances of singleton!")
	singleton_instance = self
	map = _map
	create_cargo_types()
	create_base_prices()
	create_amount_of_primary_goods()
	BasePop.create_base_needs({
		get_cargo_type("grain"): 1, get_cargo_type("wood"): 0.3, get_cargo_type("salt"): 0.1, 
		get_cargo_type("fish"): 0.2, get_cargo_type("fruit"): 0.2, get_cargo_type("meat"): 0.2,
		get_cargo_type("bread"): 0.3, get_cargo_type("clothes"): 0.3, get_cargo_type("furniture"): 0.3
	})

static func get_instance() -> terminal_map:
	assert(singleton_instance != null, "Terminal_map has not be created, and has been accessed")
	return singleton_instance

var amount_of_primary_goods: int
var mutex: Mutex = Mutex.new()
var object_mutexs: Dictionary[Vector2i, Mutex] = {}
var month_threads: Array[Thread] = []
var day_tick_priority: bool = false
var cargo_map_terminals: Dictionary[Vector2i, Terminal] = {} #Maps coords -> hold
var cargo_types: Array = [
	"clay", "sand", "sulfur", "lead", "iron", "coal", "copper", "zinc", "wood", "salt", 
	"grain", "livestock", "fish", "fruit", "cotton", "silk", "spices", "coffee", "tea", "tobacco", 
	"gold",
	
	"bricks", "glass", "lumber", "paper", "tools", "steel", "brass", "dynamite",
	"flour", "fabric", "liquor", "bread", "leather", "meat", "clothes",
	"wine", "luxury_clothes", "perserved_fruit", "porcelain",
	"furniture", "wagons", "boats", "lanterns", "trains",
	"ammo", "guns", "artillery", "preserved_meat", "canned_food", "rations", "luxury_rations",
]
var cargo_names_to_types: Dictionary = {}
var base_prices: Dictionary = {
	"clay" = 10, "sand" = 10, "sulfur" = 10, "lead" = 10, "iron" = 10, "coal" = 10, "copper" = 10, "zinc" = 10,
	"wood" = 10, "salt" = 10, "grain" = 10, "livestock" = 10, "fish" = 10, "fruit" = 10, "cotton" = 10,
	"silk" = 10, "spices" = 10, "coffee" = 10, "tea" = 10, "tobacco" = 10, "gold" = 10,
	
	"bricks" = 10, "glass" = 10, "lumber" = 10, "paper" = 10, "tools" = 10, "steel" = 10, "brass" = 10, "dynamite" = 10,
	"flour" = 10, "fabric" = 10, "liquor" = 10, "bread" = 10, "leather" = 10, "meat" = 10, "clothes" = 10,
	"wine" = 10, "luxury_clothes" = 10, "perserved_fruit" = 10, "porcelain" = 10,
	"furniture" = 10, "wagons" = 10, "boats" = 10, "lanterns" = 10, "trains" = 10,
	"ammo" = 10, "guns" = 10, "artillery" = 10, "preserved_meat" = 10, "canned_food" = 10, "rations" = 10, "luxury_rations" = 10,
}

var map: TileMapLayer
var cargo_map: TileMapLayer

func _on_day_tick_timeout() -> void:
	mutex.lock()
	day_tick_priority = true
	mutex.unlock()
	for coords: Vector2i in cargo_map_terminals:
		var obj: Terminal = cargo_map_terminals[coords]
		var obj_mutex: Mutex = object_mutexs[coords]
		obj_mutex.lock()
		if obj.has_method("day_tick"):
			obj.day_tick()
		obj_mutex.unlock()
	mutex.lock()
	day_tick_priority = false
	mutex.unlock()

func _on_month_tick_timeout() -> void:
	#for thread: Thread in month_threads:
		#thread.wait_to_finish()
	#month_threads.clear()
	var total_size: int = cargo_map_terminals.size()
	_on_month_tick_timeout_helper(cargo_map_terminals.keys(), 0, total_size)
	#var threads: int = 6
	#for i: int in range(0, threads):
		#var temp_thread: Thread = Thread.new()
		#temp_thread.start(_on_month_tick_timeout_helper.bind(cargo_map_terminals.keys(), total_size * float(i) / threads, total_size * float(i + 1) / threads))
		#month_threads.push_back(temp_thread)

func _on_month_tick_timeout_helper(keys: Array, from: int, to: int) -> void:
	for i: int in range(from, to):
		var coords: Vector2i = keys[i]
		#If index is near the day_tick then give up priority
		while day_tick_priority:
			OS.delay_msec(2)
		var obj: Terminal = cargo_map_terminals[coords]
		var obj_mutex: Mutex = object_mutexs[coords]
		obj_mutex.lock()
		if obj.has_method("month_tick"):
			obj.month_tick()
		obj_mutex.unlock()

func clear() -> void:
	mutex.lock()
	cargo_map_terminals.clear()
	object_mutexs.clear()
	mutex.unlock()

func create_amount_of_primary_goods() -> void:
	mutex.lock()
	for i: int in cargo_types.size():
		var cargo_name: String = cargo_types[i]
		if cargo_name == "gold":
			amount_of_primary_goods = i + 1
	mutex.unlock()

func get_available_resources(coords: Vector2i) -> Dictionary:
	mutex.lock()
	var toReturn: Dictionary = cargo_map.cargo_values.get_available_resources(coords)
	mutex.unlock()
	return toReturn

func assign_cargo_map(_cargo_map: TileMapLayer) -> void:
	cargo_map = _cargo_map

func create_station(coords: Vector2i, new_owner: int) -> void:
	var new_station: Station = Station.new(coords, new_owner)
	create_terminal(new_station)

func create_road_depot(_coords: Vector2i, _player_id: int) -> void:
	pass #Road depot unfinished
	

func create_terminal(p_terminal: Terminal) -> void:
	var coords: Vector2i = p_terminal.get_location()
	mutex.lock()
	object_mutexs[coords] = Mutex.new()
	cargo_map_terminals[coords] = p_terminal
	if p_terminal is Broker:
		add_connected_brokers(p_terminal)
	mutex.unlock()

func add_connected_brokers(p_broker: Broker) -> void:
	var connected_terms: Array[Vector2i] = map.thread_get_surrounding_cells(p_broker.get_location())
	for tile: Vector2i in connected_terms:
		var o_broker: Broker = get_broker(tile)
		if o_broker == null:
			continue
		p_broker.add_connected_broker(o_broker)
		o_broker.add_connected_broker(p_broker)

func is_hold(coords: Vector2i) -> bool:
	mutex.lock()
	var toReturn: bool = cargo_map_terminals.has(coords) and cargo_map_terminals[coords] is Hold
	mutex.unlock()
	return toReturn

func is_tile_taken(coords: Vector2i) -> bool:
	mutex.lock()
	var toReturn: bool = cargo_map_terminals.has(coords) or !map.is_tile_traversable(coords)
	mutex.unlock()
	return toReturn

func get_hold(coords: Vector2i) -> Dictionary:
	if is_hold(coords):
		mutex.lock()
		var toReturn: Dictionary = cargo_map_terminals[coords].get_current_hold()
		mutex.unlock()
		return toReturn
	return {}

func is_owned_recipeless_construction_site(coords: Vector2i) -> bool:
	mutex.lock()
	var toReturn: bool = cargo_map_terminals.has(coords) and cargo_map_terminals[coords] is ConstructionSite and !cargo_map_terminals[coords].has_recipe()
	mutex.unlock()
	return toReturn

func is_building(coords: Vector2i) -> bool:
	var toReturn: bool = false
	mutex.lock()
	if cargo_map_terminals.has(coords):
		var building: Terminal = cargo_map_terminals[coords]
		toReturn = building is ConstructionSite or building is Town or building is FactoryTemplate
	mutex.unlock()
	return toReturn

func is_owned_building(coords: Vector2i, id: int) -> bool:
	var temp: Terminal = get_terminal(coords)
	if temp != null and temp.player_owner == id:
		return true
	return false

func is_owned_construction_site(coords: Vector2i) -> bool:
	mutex.lock()
	var toReturn: bool = cargo_map_terminals.has(coords) and cargo_map_terminals[coords] is ConstructionSite
	mutex.unlock()
	return toReturn

func set_construction_site_recipe(coords: Vector2i, selected_recipe: Array) -> void:
	if is_owned_recipeless_construction_site(coords):
		mutex.lock()
		(cargo_map_terminals[coords] as ConstructionSite).set_recipe(selected_recipe)
		mutex.unlock()

func get_construction_site_recipe(coords: Vector2i) -> Array:
	var toReturn: Array = [{}, {}]
	if is_owned_construction_site(coords):
		mutex.lock()
		toReturn =  (cargo_map_terminals[coords] as ConstructionSite).get_recipe()
		mutex.unlock()
	return toReturn

func destory_recipe(coords: Vector2i) -> void:
	if is_owned_construction_site(coords):
		mutex.lock()
		(cargo_map_terminals[coords] as ConstructionSite).destroy_recipe()
		mutex.unlock()

func get_construction_materials(coords: Vector2i) -> Dictionary:
	var toReturn: Dictionary = {}
	if is_owned_construction_site(coords):
		mutex.lock()
		toReturn = (cargo_map_terminals[coords] as ConstructionSite).get_construction_materials()
		mutex.unlock()
	return toReturn

func is_factory(coords: Vector2i) -> bool:
	var toReturn: bool = false
	mutex.lock()
	if cargo_map_terminals.has(coords):
		toReturn = cargo_map_terminals[coords] is FactoryTemplate
	mutex.unlock()
	return toReturn

func get_cash_of_firm(coords: Vector2i) -> int:
	var toReturn: int = 0
	mutex.lock()
	if cargo_map_terminals.has(coords):
		var firm_inst: Terminal = cargo_map_terminals[coords]
		if firm_inst is Firm:
			toReturn = firm_inst.get_cash()
	mutex.unlock()
	return toReturn

func transform_construction_site_to_factory(coords: Vector2i) -> void:
	mutex.lock()
	var old_site: ConstructionSite = cargo_map_terminals[coords]
	var obj_recipe: Array = old_site.get_recipe()
	cargo_map_terminals[coords] = create_factory(coords, old_site.get_player_owner(), obj_recipe[0], obj_recipe[1])
	old_site.free()
	mutex.unlock()
	cargo_map.transform_construction_site_to_factory(coords)

func create_factory(p_location: Vector2i, p_player_owner: int, p_inputs: Dictionary, p_outputs: Dictionary) -> Factory:
	if p_player_owner > 0:
		return Factory.create(p_location, p_player_owner, p_inputs, p_outputs)
	return AiFactory.create(p_location, p_player_owner, p_inputs, p_outputs)

func get_local_prices(coords: Vector2i) -> Dictionary:
	var toReturn: Dictionary = {}
	mutex.lock()
	if cargo_map_terminals.has(coords):
		var fact: Terminal = cargo_map_terminals[coords]
		if fact is FactoryTemplate:
			toReturn = fact.get_local_prices()
	mutex.unlock()
	return toReturn

func get_terminal(coords: Vector2i) -> Terminal:
	var toReturn: Terminal = null
	mutex.lock()
	if cargo_map_terminals.has(coords):
		toReturn = cargo_map_terminals[coords]
	mutex.unlock()
	return toReturn

func get_broker(coords: Vector2i) -> Broker:
	var toReturn: Terminal = null
	mutex.lock()
	if is_broker(coords):
		toReturn = cargo_map_terminals[coords]
	mutex.unlock()
	return toReturn

func is_station(coords: Vector2i) -> bool:
	return get_terminal(coords) is Station

func is_owned_station(coords: Vector2i, player_id: int) -> bool:
	var temp: Station = get_station(coords)
	return temp != null and temp.get_player_owner() == player_id

func is_ai_station(coords: Vector2i) -> bool:
	return get_terminal(coords) is AiStation

func is_owned_ai_station(coords: Vector2i, id: int) -> bool:
	var temp: AiStation = get_ai_station(coords)
	if temp != null:
		return get_ai_station(coords).player_owner == id
	return false

func is_broker(coords: Vector2i) -> bool:
	return get_terminal(coords) is Broker

func get_station(coords: Vector2i) -> Station:
	var toReturn: Terminal = null
	if is_station(coords):
		mutex.lock()
		toReturn = cargo_map_terminals[coords]
		mutex.unlock()
	return toReturn

func create_ai_station(coords: Vector2i, orientation: int, p_owner: int) -> void:
	mutex.lock()
	if !cargo_map_terminals.has(coords):
		cargo_map_terminals[coords] = AiStation.new(coords, p_owner)
		rail_placer.get_instance().place_station.rpc(coords, orientation)
		rail_placer.get_instance().place_station.rpc(coords, (orientation + 3) % 6)
	mutex.unlock()

func get_ai_station(coords: Vector2i) -> AiStation:
	var toReturn: Terminal = null
	if is_ai_station(coords):
		mutex.lock()
		toReturn = cargo_map_terminals[coords]
		mutex.unlock()
	return toReturn

func get_station_orders(coords: Vector2i) -> Dictionary:
	var toReturn: Dictionary = {}
	if is_station(coords):
		mutex.lock()
		var orders: Dictionary = (cargo_map_terminals[coords] as Station).get_orders_dict()
		mutex.unlock()
		for type: int in orders:
			toReturn[type] = (orders[type] as TradeOrder).convert_to_array()
	return toReturn

func edit_order_station(coords: Vector2i, type: int, amount: int, buy: bool, max_price: float) -> void:
	if is_station(coords):
		mutex.lock()
		(cargo_map_terminals[coords] as Station).edit_order(type, amount, buy, max_price)
		mutex.unlock()

func remove_order_station(coords: Vector2i, type: int) -> void:
	if is_station(coords):
		mutex.lock()
		(cargo_map_terminals[coords] as Station).remove_order(type)
		mutex.unlock()

func create_cargo_types() -> void:
	for type: int in cargo_types.size():
		cargo_names_to_types[cargo_types[type]] = type

func create_base_prices() -> void:
	#LocalPriceController.set_base_prices()
	assert(base_prices.size() == cargo_types.size())

func get_number_of_goods() -> int:
	mutex.lock()
	var toReturn: int = cargo_types.size()
	mutex.unlock()
	return toReturn

func get_cargo_name(index: int) -> String:
	mutex.lock()
	var toReturn: String = cargo_types[index]
	mutex.unlock()
	return toReturn

func get_cargo_type(cargo_name: String) -> int:
	var toReturn: int = -1
	mutex.lock()
	if cargo_names_to_types.has(cargo_name):
		toReturn = cargo_names_to_types[cargo_name]
	mutex.unlock()
	assert(toReturn != -1)
	return toReturn

func get_cargo_array_at_location(coords: Vector2i) -> Dictionary:
	return (get_terminal(coords) as Hold).get_current_hold()

func get_cargo_array() -> Array:
	mutex.lock()
	var toReturn: Array = cargo_types.duplicate()
	mutex.unlock()
	return toReturn

func is_cargo_primary(cargo_type: int) -> bool:
	return cargo_type < amount_of_primary_goods

func get_available_primary_recipes(coords: Vector2i) -> Array:
	return cargo_map.get_available_primary_recipes(coords)

func is_town(coords: Vector2i) -> bool:
	return get_terminal(coords) is Town

func get_town(coords: Vector2i) -> Town:
	var toReturn: Town = null
	if is_town(coords):
		mutex.lock()
		toReturn = cargo_map_terminals[coords]
		mutex.unlock()
	return toReturn

func get_town_fulfillment(coords: Vector2i) -> Dictionary[int, float]:
	var toReturn: Dictionary[int, float] = {}
	var term: Town = get_terminal(coords)
	mutex.lock()
	if is_town(coords):
		toReturn = term.get_fulfillment_dict()
	mutex.unlock()
	return toReturn
