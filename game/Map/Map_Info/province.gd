class_name province extends Node

var province_id: int
var population: int
var tiles: Array
var terminal_tiles: Dictionary[Vector2i, terminal]
var country_id: int = -1
var pops: Array[base_pop] = []

func _init(_province_id: int) -> void:
	province_id = _province_id
	population = 0
	tiles = []
	terminal_tiles = {}

func add_tile(coords: Vector2i) -> void:
	tiles.append(coords)

func get_population() -> int:
	return population

func add_population(pop: int) -> void:
	population += pop

func set_population(new_pop: int) -> void:
	population = new_pop

func set_country_id(p_country_id: int) -> void:
	country_id = p_country_id

func get_tiles() -> Array:
	return tiles

func get_random_tile() -> Variant:
	var temp_tiles: Array = tiles.duplicate()
	var rand_index: int = randi() % temp_tiles.size()
	var random_tile: Vector2i = temp_tiles.pop_at(rand_index)
	while terminal_tiles.has(random_tile):
		if temp_tiles.size() == 0:
			#Will probs never get called
			return Vector2i(0, 0)
		rand_index = randi() % temp_tiles.size()
		random_tile = temp_tiles.pop_at(rand_index)
	return random_tile

func add_terminal(coords: Vector2i, term: terminal) -> void:
	terminal_tiles[coords] = term

func remove_terminal(coords: Vector2i) -> void:
	#BUG: Never gets called when deleting terminals
	terminal_tiles.erase(coords)

func get_terminals() -> Array[terminal]:
	return terminal_tiles.values()

# === Pops ===

func create_pops() -> void:
	var number_of_rural_pops: int = floor(population * 0.8 / base_pop.PEOPLE_PER_POP)
	var number_of_city_pops: int = floor(population * 0.2 / base_pop.PEOPLE_PER_POP)
	for i: int in number_of_rural_pops:
		pops.push_back(rural_pop.new(province_id))
	var cities: Array[town] = get_cities()
	#If no cities, then turn rest of population into rural pops
	if cities.size() == 0:
		for i: int in number_of_city_pops:
			pops.push_back(rural_pop.new(province_id))
		return
	var index: int = 0
	for i: int in number_of_city_pops:
		var city: town = cities[index]
		city.add_pop(city_pop.new(province_id))
		index = (index + 1) % cities.size()

func get_cities() -> Array[town]:
	var toReturn: Array[town] = []
	for term: terminal in terminal_tiles.values():
		if term is town:
			toReturn.append(term)
	return toReturn

func count_pops() -> int:
	return pops.size()

func find_employment(pop: base_pop) -> factory_template:
	if pop is city_pop:
		pass
		#TODO: Add buildings to cities
	elif pop is rural_pop:
		for term: terminal in get_terminals():
			if term is factory_template and will_work_here(pop, term):
				return term
	return null

func will_work_here(pop: base_pop, industry: factory_template) -> bool:
	if !industry.is_hiring():
		return false
	var income: float = industry.get_wage()
	if pop.is_income_acceptable(income):
		return true
	return false
