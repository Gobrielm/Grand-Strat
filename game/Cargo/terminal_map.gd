class_name terminal_map extends Node
#Represents Singleton
#TODO: Fully convert into singleton
static var amount_of_primary_goods: int

static var serviced: Dictionary = {}
static var cargo_map_terminals: Dictionary = {} #Maps coords -> hold
static var cargo_types: Array = [
	"clay", "sand", "sulfur", "lead", "iron", "coal", "copper", "zinc", "wood", "salt", 
	"grain", "livestock", "fish", "fruit", "cotton", "silk", "spices", "coffee", "tea", "tobacco", 
	"gold",
	
	"bricks", "glass", "lumber", "paper", "tools", "steel", "brass", "dynamite",
	"flour", "fabric", "liquor", "bread", "leather", "meat", "clothes",
	"wine", "luxury_clothes", "cigarettes", "perserved_fruit", "porcelain",
	"furniture", "wagons", "boats", "lanterns", "trains",
	"ammo", "guns", "artillery", "preserved_meat", "canned_food", "rations", "luxury_rations",
]
static var cargo_names_to_types: Dictionary = {}
static var base_prices: Dictionary = {
	"clay" = 10, "sand" = 10, "sulfur" = 10, "lead" = 10, "iron" = 10, "coal" = 10, "copper" = 10, "zinc" = 10,
	"wood" = 10, "salt" = 10, "grain" = 10, "livestock" = 10, "fish" = 10, "fruit" = 10, "cotton" = 10,
	"silk" = 10, "spices" = 10, "coffee" = 10, "tea" = 10, "tobacco" = 10, "gold" = 10,
	
	"bricks" = 10, "glass" = 10, "lumber" = 10, "paper" = 10, "tools" = 10, "steel" = 10, "brass" = 10, "dynamite" = 10,
	"flour" = 10, "fabric" = 10, "liquor" = 10, "bread" = 10, "leather" = 10, "meat" = 10, "clothes" = 10,
	"wine" = 10, "luxury_clothes" = 10, "cigarettes" = 10, "perserved_fruit" = 10, "porcelain" = 10,
	"furniture" = 10, "wagons" = 10, "boats" = 10, "lanterns" = 10, "trains" = 10,
	"ammo" = 10, "guns" = 10, "artillery" = 10, "preserved_meat" = 10, "canned_food" = 10, "rations" = 10, "luxury_rations" = 10,
}


static var map: TileMapLayer
static var cargo_map: TileMapLayer

static func _static_init() -> void:
	create_cargo_types()
	create_base_prices()
	create_amount_of_primary_goods()

static func _on_day_tick_timeout() -> void:
	for obj: terminal in cargo_map_terminals.values():
		if obj.has_method("day_tick"):
			obj.day_tick()

static func _on_month_tick_timeout() -> void:
	for obj: terminal in cargo_map_terminals.values():
		if obj.has_method("month_tick"):
			obj.month_tick()

static func create(_map: TileMapLayer) -> void:
	map = _map

static func create_amount_of_primary_goods() -> void:
	for i: int in cargo_types.size():
		var cargo_name: String = cargo_types[i]
		if cargo_name == "gold":
			amount_of_primary_goods = i + 1

static func get_available_resources(coords: Vector2i) -> Dictionary:
	return cargo_map.cargo_values.get_available_resources(coords)

static func assign_cargo_map(_cargo_map: TileMapLayer) -> void:
	cargo_map = _cargo_map

static func create_station(coords: Vector2i, new_owner: int) -> void:
	var new_station: station = station.new(coords, new_owner)
	create_terminal(new_station)

static func create_road_depot(coords: Vector2i, player_id: int) -> void:
	if !cargo_map_terminals.has(coords):
		cargo_map_terminals[coords] = road_depot.new(coords, player_id)
		create_terminal(cargo_map_terminals[coords])

static func create_terminal(p_terminal: terminal) -> void:
	var coords: Vector2i = p_terminal.get_location()
	cargo_map_terminals[coords] = p_terminal
	add_connected_terminals(p_terminal)

static func add_connected_terminals(p_terminal: terminal) -> void:
	var connected_terms: Array[Vector2i] = map.get_surrounding_cells(p_terminal.location)
	for tile: Vector2i in connected_terms:
		var o_terminal: terminal = get_terminal(tile)
		if o_terminal == null:
			continue
		if p_terminal.has_method("add_connected_terminal"):
			p_terminal.add_connected_terminal(o_terminal)
		if o_terminal.has_method("add_connected_terminal"):
			o_terminal.add_connected_terminal(p_terminal)

static func is_hold(coords: Vector2i) -> bool:
	return cargo_map_terminals.has(coords) and cargo_map_terminals[coords] is hold

static func is_tile_taken(coords: Vector2i) -> bool:
	return cargo_map_terminals.has(coords) or !map.is_tile_traversable(coords)

static func get_hold(coords: Vector2i) -> Dictionary:
	if is_hold(coords):
		return cargo_map_terminals[coords].get_current_hold()
	return {}

static func is_owned_recipeless_construction_site(coords: Vector2i) -> bool:
	return cargo_map_terminals.has(coords) and cargo_map_terminals[coords] is construction_site and !cargo_map_terminals[coords].has_recipe()

static func is_owned_construction_site(coords: Vector2i) -> bool:
	return cargo_map_terminals.has(coords) and cargo_map_terminals[coords] is construction_site

static func set_construction_site_recipe(coords: Vector2i, selected_recipe: Array) -> void:
	if is_owned_recipeless_construction_site(coords):
		cargo_map_terminals[coords].set_recipe(selected_recipe)

static func get_construction_site_recipe(coords: Vector2i) -> Array:
	if is_owned_construction_site(coords):
		return cargo_map_terminals[coords].get_recipe()
	return [{}, {}]

static func destory_recipe(coords: Vector2i) -> void:
	if is_owned_construction_site(coords):
		cargo_map_terminals[coords].destroy_recipe()

static func get_construction_materials(coords: Vector2i) -> Dictionary:
	if is_owned_construction_site(coords):
		return cargo_map_terminals[coords].get_construction_materials()
	return {}

static func is_factory(coords: Vector2i) -> bool:
	if cargo_map_terminals.has(coords):
		return cargo_map_terminals[coords] is factory_template
	return false

static func get_cash_of_firm(coords: Vector2i) -> int:
	if cargo_map_terminals.has(coords):
		var firm_inst: terminal = cargo_map_terminals[coords]
		if firm_inst is firm:
			return firm_inst.get_cash()
	return 0

static func transform_construction_site_to_factory(coords: Vector2i) -> void:
	var old_site: construction_site = cargo_map_terminals[coords]
	var obj_recipe: Array = old_site.get_recipe()
	cargo_map_terminals[coords] = create_factory(coords, old_site.get_player_owner(), obj_recipe[0], obj_recipe[1])
	old_site.queue_free()
	cargo_map.transform_construction_site_to_factory(coords)

static func create_factory(p_location: Vector2i, p_player_owner: int, p_inputs: Dictionary, p_outputs: Dictionary) -> factory:
	if p_player_owner > 0:
		return player_factory.new(p_location, p_player_owner, p_inputs, p_outputs)
	return ai_factory.new(p_location, p_player_owner, p_inputs, p_outputs)

static func get_local_prices(coords: Vector2i) -> Dictionary:
	if cargo_map_terminals.has(coords):
		var fact: terminal = cargo_map_terminals[coords]
		if fact is factory_template:
			return fact.get_local_prices()
	return {}

static func get_terminal(coords: Vector2i) -> terminal:
	if cargo_map_terminals.has(coords):
		return cargo_map_terminals[coords]
	return null

static func get_broker(coords: Vector2i) -> broker:
	if is_broker(coords):
		return cargo_map_terminals[coords]
	return null

static func is_station(coords: Vector2i) -> bool:
	return cargo_map_terminals.has(coords) and cargo_map_terminals[coords] is station

static func is_ai_station(coords: Vector2i) -> bool:
	return cargo_map_terminals.has(coords) and cargo_map_terminals[coords] is ai_station

static func is_broker(coords: Vector2i) -> bool:
	return cargo_map_terminals.has(coords) and cargo_map_terminals[coords] is broker

static func get_station(coords: Vector2i) -> station:
	if is_station(coords):
		return cargo_map_terminals[coords]
	return null

static func get_ai_station(coords: Vector2i) -> ai_station:
	if is_ai_station(coords):
		return cargo_map_terminals[coords]
	return null

static func get_station_orders(coords: Vector2i) -> Dictionary:
	var toReturn: Dictionary = {}
	if is_station(coords):
		var orders: Dictionary = cargo_map_terminals[coords].get_orders()
		for type: int in orders:
			toReturn[type] = orders[type].convert_to_array()
	return toReturn

static func edit_order_station(coords: Vector2i, type: int, amount: int, buy: bool, max_price: float) -> void:
	if is_station(coords):
		cargo_map_terminals[coords].edit_order(type, amount, buy, max_price)

static func remove_order_station(coords: Vector2i, type: int) -> void:
	if is_station(coords):
		cargo_map_terminals[coords].remove_order(type)

static func create_cargo_types() -> void:
	for type: int in cargo_types.size():
		cargo_names_to_types[cargo_types[type]] = type

static func create_base_prices() -> void:
	var new_base_prices: Dictionary = {}
	for good_name: String in base_prices:
		new_base_prices[cargo_names_to_types[good_name]] = base_prices[good_name]
	local_price_controller.set_base_prices(new_base_prices)
	assert(base_prices.size() == cargo_types.size())

static func get_number_of_goods() -> int:
	return cargo_types.size()

static func get_cargo_name(index: int) -> String:
	return cargo_types[index]

static func get_cargo_type(cargo_name: String) -> int:
	if cargo_names_to_types.has(cargo_name):
		return cargo_names_to_types[cargo_name]
	return -1

static func get_cargo_array_at_location(coords: Vector2i) -> Dictionary:
	return get_terminal(coords).get_current_hold()

static func get_cargo_array() -> Array:
	return cargo_types

static func is_cargo_primary(cargo_type: int) -> bool:
	return cargo_type < amount_of_primary_goods

static func get_available_primary_recipes(coords: Vector2i) -> Array:
	return cargo_map.get_available_primary_recipes(coords)

static func is_town(coords: Vector2i) -> bool:
	var term: terminal = get_terminal(coords)
	return term != null and term is town

static func get_town_fulfillment(coords: Vector2i, type: int) -> float:
	var term: terminal = get_terminal(coords)
	if is_town(coords):
		return term.get_fulfillment(type)
	return 0.0

static func get_town_wants(coords: Vector2i) -> Array:
	var term: terminal = get_terminal(coords)
	if is_town(coords):
		return term.get_town_wants()
	return []
