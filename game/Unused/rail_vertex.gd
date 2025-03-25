#class_name rail_vertex extends Node
#var location: Vector2i
#var connections: Dictionary = {}
#
#func _init(new_location: Vector2i):
	#location = new_location
#
#func get_coordinates() -> Vector2i:
	#return location
#
#func move_vertex(new_location: Vector2i):
	#var change = 0 if new_location == location else 1
	#location = new_location
	#for vertex in connections:
		#change_connection(vertex, change)
		#vertex.change_connection(self, change)
#
#func add_vertex_connection_if_shorter(other_vertex: rail_vertex, direction: int, distance: int):
	#if (connections.has(other_vertex) and get_length(other_vertex) > distance) or !connections.has(other_vertex):
		#add_connection(other_vertex, direction, distance)
#
#func add_vertex_loop_if_shorter(direction: int, distance: int, direction2: int):
	#if (connections.has(self) and get_length(self) > distance) or !connections.has(self):
		#add_loop_connection(direction, distance, direction2)
#
#func add_connection(other_vertex: rail_vertex, direction: int, distance: int):
	#connections[other_vertex] = [direction, distance]
#
#func add_loop_connection(direction: int, distance: int, direction2: int):
	#connections[self] = [direction, distance, direction2]
#
#func change_connection(vertex_that_changed, distance_change):
	#connections[vertex_that_changed][1] += distance_change
#
#func get_connection() -> rail_vertex:
	#if get_connections_count() == 1:
		#return connections.keys()[0]
	#return null
#
#func is_connected_to(vertex: rail_vertex) -> bool:
	#return connections.has(vertex)
#
#func get_length(vertex: rail_vertex) -> int:
	#if connections.has(vertex):
		#return connections[vertex][1]
	#return -1
#
#func get_direction(vertex: rail_vertex) -> int:
	#if connections.has(vertex):
		#return connections[vertex][0]
	#return -1
#
#func get_direction_self() -> int:
	#if connections.has(self):
		#return connections[self][2]
	#return -1
#
#func get_connections() -> Dictionary:
	#return connections
#
#func get_connections_count() -> int:
	#return connections.keys().size()
#
#func remove_all_connections():
	#connections = {}
#
#func remove_connection(other_vertex: rail_vertex):
	#connections.erase(other_vertex)
