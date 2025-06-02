class_name province extends RefCounted

var province_id: int
var population: int
var tiles: Array
var terminal_tiles: Dictionary[Vector2i, Terminal]
var country_id: int = -1
var pops: Array[BasePop] = []

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

func add_terminal(coords: Vector2i, term: Terminal) -> void:
	terminal_tiles[coords] = term

func remove_terminal(coords: Vector2i) -> void:
	#BUG: Never gets called when deleting terminals
	terminal_tiles.erase(coords)

func get_terminals() -> Array[Terminal]:
	return terminal_tiles.values()

# === Pops ===

func create_pops() -> void:
	var number_of_rural_pops: int = floor(population * 0.8 / BasePop.get_people_per_pop())
	var number_of_city_pops: int = floor(population * 0.2 / BasePop.get_people_per_pop())
	for i: int in number_of_rural_pops:
		pops.push_back(BasePop.create(province_id, null))
	var cities: Array[Town] = get_cities()
	#If no cities, then turn rest of population into rural pops
	if cities.size() == 0:
		for i: int in number_of_city_pops:
			pops.push_back(BasePop.create(province_id, null))
		return
	var index: int = 0
	for i: int in number_of_city_pops:
		var city: Town = cities[index]
		city.add_pop(BasePop.create(province_id, 0))
		index = (index + 1) % cities.size()

func get_cities() -> Array[Town]:
	var toReturn: Array[Town] = []
	for term: Terminal in terminal_tiles.values():
		if term is Town:
			toReturn.append(term)
	return toReturn

func count_pops() -> int:
	return pops.size()

func find_employment(_pop: BasePop) -> FactoryTemplate:
	#if pop is city_pop:
		#pass
		##TODO: Add buildings to cities
	#elif pop is rural_pop:
		#for term: Terminal in get_terminals():
			#if term is factory_template and will_work_here(pop, term):
				#return term
	return null

func will_work_here(pop: BasePop, industry: FactoryTemplate) -> bool:
	if !industry.is_hiring():
		return false
	var income: float = industry.get_wage()
	if pop.is_income_acceptable(income):
		return true
	return false
