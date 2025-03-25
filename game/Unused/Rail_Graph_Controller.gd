#class_name rail_graph_controller extends Node
#
#var map: TileMapLayer
#var rail_vertices = {}
#
#func _init(new_map: TileMapLayer):
	#map = new_map
#
#func clear_and_clean_all_vertices():
	#for vertex in rail_vertices.values():
		#vertex.queue_free()
	#rail_vertices.clear()
#
#func add_stationary_vertex(coordinates: Vector2i):
	#rail_vertices[coordinates] = rail_vertex_stationary.new(coordinates)
	#search_for_connections(rail_vertices[coordinates])
	#var vertex = get_vertex(coordinates)
	#for vert in vertex.get_connections():
		#vert.remove_all_connections()
		#search_for_connections(vert)
#
#func add_rail_vertex(coordinates: Vector2i):
	#rail_vertices[coordinates] = rail_vertex.new(coordinates)
	#search_for_connections(rail_vertices[coordinates])
	#var vertex = get_vertex(coordinates)
	#for vert in vertex.get_connections():
		#vert.remove_all_connections()
		#search_for_connections(vert)
	#check_convert_endpoint_intersection(coordinates)
#
#func check_convert_endpoint_intersection(coordinates: Vector2i):
	#var vertex = get_vertex(coordinates)
	#var count = vertex.connections.size()
	#if count < 2:
		#rail_vertices[coordinates] = rail_vertex_endpoint.new(vertex)
	#else:
		#rail_vertices[coordinates] = rail_vertex_intersection.new(vertex)
#
#func move_rail_vertex(coordinates: Vector2i, new_coordinates: Vector2i):
	#rail_vertices[coordinates].move_vertex(new_coordinates)
	#rail_vertices[new_coordinates] = rail_vertices[coordinates]
	#rail_vertices.erase(coordinates)
#
#func get_vertex(coordinates: Vector2i) -> rail_vertex:
	#if rail_vertices.has(coordinates):
		#return rail_vertices[coordinates]
	#return null
#
#func does_vertex_have_no_connections(coordinates: Vector2i) -> bool:
	#return get_vertex(coordinates).get_connections_count() == 0
#
#func is_tile_vertex(coordinates: Vector2i) -> bool:
	#return rail_vertices.has(coordinates)
#
#func is_tile_stationary_vertix(coordinates: Vector2i) -> bool:
	#return rail_vertices.has(coordinates) and rail_vertices[coordinates] is rail_vertex_stationary
#
#func is_tile_endpoint(coordinates: Vector2i) -> bool:
	#return rail_vertices.has(coordinates) and rail_vertices[coordinates] is rail_vertex_endpoint
#
#func is_tile_intersection(coordinates: Vector2i) -> bool:
	#return rail_vertices.has(coordinates) and rail_vertices[coordinates] is rail_vertex_intersection
#
#func get_length(vertex1: rail_vertex, vertex2: rail_vertex) -> int:
	#var length = 0
	#if vertex1.get_connection() != null:
		#length += vertex1.get_connection().get_length(vertex1)
	#if vertex2.get_connection() != null:
		#length += vertex2.get_connection().get_length(vertex2)
	#return length + 1
#
#func search_for_connections(vertex: rail_vertex):
	#if vertex == null:
		#return
	#var coordinates = vertex.get_coordinates()
	#var queue = []
	#var visited = {}
	#visited[coordinates] = -1
	#var from_vertex = {}
	#from_vertex[coordinates] = -1
	#var distance_away = {}
	#var current
	#queue.append(coordinates)
	#distance_away[coordinates] = 0
	#while !queue.is_empty():
		#current = queue.pop_front()
		#var rail_connections: Array = map.get_tile_connections(current)
		#for direction in rail_connections.size():
			##If there is connection in that direction could the train traverse there?
			#if rail_connections[direction]:
				#var tile = get_neighbor_cell_given_direction(current, direction)
				##Does this tile also connect back to path_find_pos
				#if can_direction_reach(visited[current], direction) and map.get_tile_connections(tile)[(direction + 3) % 6]:
					#if is_tile_vertex(tile):
						#var dir = from_vertex[current]
						#if from_vertex[current] == -1:
							#dir = direction
						#var vert = get_vertex(tile)
						#var dist = distance_away[current] + 1
						#vert.add_vertex_connection_if_shorter(vertex, (direction + 3) % 6, dist)
						#vertex.add_vertex_connection_if_shorter(vert, dir, dist)
					#elif !visited.has(tile):
						#if from_vertex[current] == -1:
							#from_vertex[tile] = direction
						#else:
							#from_vertex[tile] = from_vertex[current]
						#queue.append(tile)
						#visited[tile] = direction
						#distance_away[tile] = distance_away[current] + 1
					#else:
						#var dir = from_vertex[tile]
						#var other_dir = from_vertex[current]
						#var dist = distance_away[tile] + distance_away[current] + 2
						#vertex.add_vertex_loop_if_shorter(dir, dist, other_dir)
#
#func can_direction_reach(direction: int, other_direction: int) -> bool:
	#return direction == -1 or direction == other_direction or (direction + 1) % 6 == other_direction or (direction + 5) % 6 == other_direction
#
#func get_neighbor_cell_given_direction(coords: Vector2i, num: int) -> Vector2i:
	#if num == 0:
		#return map.get_neighbor_cell(coords, TileSet.CELL_NEIGHBOR_TOP_SIDE)
	#elif num == 1:
		#return map.get_neighbor_cell(coords, TileSet.CELL_NEIGHBOR_TOP_RIGHT_SIDE)
	#elif num == 2:
		#return map.get_neighbor_cell(coords, TileSet.CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE)
	#elif num == 3:
		#return map.get_neighbor_cell(coords, TileSet.CELL_NEIGHBOR_BOTTOM_SIDE)
	#elif num == 4:
		#return map.get_neighbor_cell(coords, TileSet.CELL_NEIGHBOR_BOTTOM_LEFT_SIDE)
	#elif num == 5:
		#return map.get_neighbor_cell(coords, TileSet.CELL_NEIGHBOR_TOP_LEFT_SIDE)
	#return Vector2i(0, 0)
#
#func delete_rail_vertex(coordinates: Vector2i):
	#var vertex = get_vertex(coordinates)
	#if vertex == null:
		#return
	#var vertices_searched = {}
	#rail_vertices.erase(coordinates)
	#for connected_vertex: rail_vertex in vertex.get_connections():
		#if !vertices_searched.has(connected_vertex) and connected_vertex != vertex:
			#connected_vertex.remove_connection(vertex)
			#vertices_searched[connected_vertex] = 1
			#search_for_connections(connected_vertex)
		#
	#vertex.queue_free()
	#
	#
