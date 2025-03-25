#extends Node
#
#var map: TileMapLayer
#
#func _init(new_map: TileMapLayer):
	#map = new_map
	##test()
	##runtime_test()
	##train_algorithm_test()
#
#func test():
	#print("testing")
	#test_basic_unordered()
	#print("test_basic_unordered         ✔️")
	#test_huge_unordered()
	#print("test_huge_unordered          ✔️")
	#basic_test_ordered()
	#print("basic_test_ordered           ✔️")
	#basic_test_ordered1()
	#print("basic_test_ordered1          ✔️")
	#basic_loop()
	#print("basic_loop                   ✔️")
	#basic_loop_with_station()
	#print("basic_loop_with_station      ✔️")
	#print("runtime_test                 ✔️")
	#runtime_test()
	#train_algorithm_test()
#
#func build_rail(coords: Vector2i, orientation: int):
	#map.set_cell_rail_placer_server(coords, orientation, 0, 1)
#
#func build_station(coords: Vector2i, orientation: int):
	#map.set_cell_rail_placer_server(coords, orientation, 2, 1)
#
#func build_many_rails(start: Vector2i, end: Vector2i):
	#map.place_to_end_rail(start, end)
#
#func clear_test_stuff():
	#map.rail_placer.clear_all_real()
	#map.rail_placer.rail_graph.clear_and_clean_all_vertices()
#
#func test_basic_unordered():
	#var point1 = Vector2i(-1, -1)
	#var point2 = Vector2i(-3, -3)
	#build_many_rails(point1, point2)
	#var connections: Dictionary = map.rail_placer.rail_graph.rail_vertices
	#var vertex1: rail_vertex = connections[point1]
	#var vertex2: rail_vertex = connections[point2]
	#assert(vertex1.is_connected_to(vertex2), "Not connected vertex")
	#assert(vertex2.is_connected_to(vertex1), "Not connected vertex")
	#assert(vertex1.get_length(vertex2) == 3, "Length Wrong")
	#assert(connections.size() == 2, "Missing vertex or extra vertex")
	#clear_test_stuff()
#
#func test_huge_unordered():
	#var point1 = Vector2i(-1, -1)
	#var point2 = Vector2i(-200, -200)
	#build_many_rails(point1, point2)
	#var connections: Dictionary = map.rail_placer.rail_graph.rail_vertices
	#var vertex1: rail_vertex = connections[point1]
	#var vertex2: rail_vertex = connections[point2]
	#assert(vertex1.is_connected_to(vertex2), "Not connected vertex")
	#assert(vertex2.is_connected_to(vertex1), "Not connected vertex")
	#assert(vertex1.get_length(vertex2) == 299, "Length Wrong")
	#assert(connections.size() == 2, "Missing vertex or extra vertex")
	#clear_test_stuff()
	#
#func basic_test_ordered():
	#var point1 = Vector2i(-1, -1)
	#var point2 = Vector2i(0, -1)
	#var point3 = Vector2i(1, -2)
	#var point4 = Vector2i(2, -2)
	#build_rail(point1, 1)
	#build_rail(point2, 4)
	#build_rail(point3, 1)
	#build_rail(point4, 4)
	#
	#build_rail(point2, 1)
	#build_rail(point3, 4)
	#
	#var connections: Dictionary = map.rail_placer.rail_graph.rail_vertices
	#var vertex1: rail_vertex = connections[point1]
	#var vertex2: rail_vertex = connections[point4]
	#
	#assert(vertex1.is_connected_to(vertex2), "Not connected vertex")
	#assert(vertex2.is_connected_to(vertex1), "Not connected vertex")
	#assert(vertex1.get_length(vertex2) == 3, "Length Wrong")
	#assert(connections.size() == 2, "Missing vertex or extra vertex")
	#clear_test_stuff()
#
#func basic_test_ordered1():
	#var point1 = Vector2i(-1, -1)
	#var point2 = Vector2i(0, -1)
	#var point3 = Vector2i(1, -2)
	#build_rail(point1, 1)
	#build_rail(point2, 4)
	#build_rail(point3, 4)
	#build_rail(point2, 1)
	#
	#var connections: Dictionary = map.rail_placer.rail_graph.rail_vertices
	#var vertex1: rail_vertex = connections[point1]
	#var vertex2: rail_vertex = connections[point3]
	#
	#assert(vertex1.is_connected_to(vertex2), "Not connected vertex")
	#assert(vertex2.is_connected_to(vertex1), "Not connected vertex")
	#assert(vertex1.get_length(vertex2) == 2, "Length Wrong")
	#assert(connections.size() == 2, "Missing vertex or extra vertex")
	#clear_test_stuff()
#
#func basic_loop():
	#var point1 = Vector2i(3, -2)
	#var point2 = Vector2i(4, -1)
	#var point3 = Vector2i(4, 0)
	#var point4 = Vector2i(3, 0)
	#var point5 = Vector2i(2, 0)
	#var point6 = Vector2i(2, -1)
	#var point7 = Vector2i(1, 0)
	#build_many_rails(point1, point2)
	#build_many_rails(point2, point3)
	#build_many_rails(point3, point4)
	#build_many_rails(point4, point5)
	#build_many_rails(point5, point6)
	#build_many_rails(point6, point1)
	#
	#var connections: Dictionary = map.rail_placer.rail_graph.rail_vertices
	#assert(connections.size() == 0, "Missing vertex or extra vertex")
	#
	#build_many_rails(point7, point5)
	#
	#var vertex1: rail_vertex = connections[point5]
	#var vertex2: rail_vertex = connections[point7]
	#
	#assert(vertex1.is_connected_to(vertex2), "Not connected vertex")
	#assert(vertex2.is_connected_to(vertex1), "Not connected vertex")
	#assert(vertex1.get_length(vertex2) == 1, "Length Wrong")
	#assert(connections.size() == 2, "Missing vertex or extra vertex")
	#clear_test_stuff()
#
#func basic_loop_with_station():
	#var point1 = Vector2i(3, -2)
	#var point2 = Vector2i(4, -1)
	#var point3 = Vector2i(5, -1)
	#var point4 = Vector2i(5, 1)
	#var point5 = Vector2i(3, 2)
	#var point6 = Vector2i(1, 1)
	#var point8 = Vector2i(-3, 1)
	#build_many_rails(point1, point2)
	#build_station(point2, 2)
	#build_rail(point3, 5)
	#build_many_rails(point3, point4)
	#build_many_rails(point4, point5)
	#build_many_rails(point5, point6)
	#build_many_rails(point6, point1)
	#
	#var connections: Dictionary = map.rail_placer.rail_graph.rail_vertices
	#assert(connections.size() == 1, "Missing vertex or extra vertex")
	#
	#build_many_rails(point6, point8)
	#var vertex1: rail_vertex = connections[point2]
	#var vertex2: rail_vertex = connections[point6]
	#var vertex3: rail_vertex = connections[point8]
	#assert(connections.size() == 3, "Missing vertex or extra vertex")
	#assert(vertex1.is_connected_to(vertex2), "Not connected vertex")
	#assert(vertex2.is_connected_to(vertex1), "Not connected vertex")
	#assert(vertex3.is_connected_to(vertex2), "Not connected vertex")
	#clear_test_stuff()
#
#
#func runtime_test():
	#var start: float = Time.get_ticks_msec()
	#var point1 = Vector2i(0, 0)
	#var point2 = Vector2i(400, -100)
	#var point3 = Vector2i(500, -100)
	#var point4 = Vector2i(500, 100)
	#var point5 = Vector2i(300, 200)
	#build_many_rails(point1, point2)
	#build_many_rails(point2, point3)
	#build_many_rails(point3, point4)
	#build_many_rails(point4, point5)
	#build_many_rails(point5, point1)
	#build_many_rails(point1, point3)
	#clear_test_stuff()
	#var end: float = Time.get_ticks_msec()
	#print(str((end - start) / 1000) + " Seconds passed")
#
#func train_algorithm_test():
	#var point1 = Vector2i(0, 0)
	#var point2 = Vector2i(300, 0)
	#
	#var end_stop = Vector2i(300, -50)
	#for i in 100:
		#build_many_rails(point1, point2)
		#point2.y = -i
	#point2 = Vector2i(0, -300)
	#for i in 300:
		#build_many_rails(point1, point2)
		#point2.x = i
	#point1 = Vector2i(0, 0)
	#var start: float = Time.get_ticks_msec()
	#
	#map.create_train.rpc(point1)
#
	#var train = map.get_node("Train0")
	#train.add_stop(end_stop)
	#train.start_train()
	#var end: float = Time.get_ticks_msec()
	#print(str((end - start) / 1000) + " Seconds passed")
