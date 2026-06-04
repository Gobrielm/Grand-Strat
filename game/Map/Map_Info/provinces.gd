extends TileMapLayer

var adjacency_list: Dictionary[int, Dictionary] = {}
var tile_to_province: Dictionary[Vector2i, int] = {}
var province_to_color: Dictionary[int, Vector2i] = {}

func get_unselected_atlas(barred_atlases: Dictionary[Vector2i, bool]) -> Vector2i:
	if !barred_atlases.has(Vector2i(2, 5)):
		return Vector2i(2, 5)
	elif !barred_atlases.has(Vector2i(7, 5)):
		return Vector2i(7, 5)
	elif !barred_atlases.has(Vector2i(5, 3)):
		return Vector2i(5, 3)
	elif !barred_atlases.has(Vector2i(5, 2)):
		return Vector2i(5, 2)
	return Vector2i(0, 0)

func add_tile_to_province(tile: Vector2i, id: int) -> void:
	if !adjacency_list.has(id):
		adjacency_list[id] = {}
	tile_to_province[tile] = id
	
	for cell: Vector2i in get_surrounding_cells(tile):
		if !tile_to_province.has(cell): continue
		var other_prov: int = tile_to_province[cell]
		if other_prov == id: continue
		
		adjacency_list[id][other_prov] = true
		adjacency_list[other_prov][id] = true

#func double_assign() -> void:
	#
	#for tile: Vector2i in tile_to_province:
		#var id: int = tile_to_province[tile]
		#for cell: Vector2i in get_surrounding_cells(tile):
			#if !tile_to_province.has(cell): continue
			#var other_prov: int = tile_to_province[cell]
			#if other_prov == id: continue
			#
			#adjacency_list[id][other_prov] = true
			#adjacency_list[other_prov][id] = true

func get_next_id() -> int:
	for id: int in adjacency_list.size():
		if !province_to_color.has(id):
			return id
	return -1

func assign_province_colors() -> void:
	#double_assign()
	var ids: Array = adjacency_list.keys()
	ids.sort()
	print(ids.size())
	
	var provinces_to_colors: Array = [0]
	while true:
		var id: int = provinces_to_colors.pop_front() if !provinces_to_colors.is_empty() else get_next_id()
		if id == -1:
			break
		if province_to_color.has(id): continue
		
		var colors_barred: Dictionary[Vector2i, bool] = {}
		for adj: int in adjacency_list[id]:
			if province_to_color.has(adj):
				colors_barred[province_to_color[adj]] = true
			else:
				provinces_to_colors.push_back(adj)
		var avail_color: Vector2i = get_unselected_atlas(colors_barred)
		province_to_color[id] = avail_color
	
	
	for tile: Vector2i in tile_to_province:
		var id: int = tile_to_province[tile]
		call_deferred("set_cell", tile, 0, province_to_color[id])
