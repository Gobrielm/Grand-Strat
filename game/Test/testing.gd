extends Node

var map: TileMapLayer

func _init(new_map: TileMapLayer):
	map = new_map
	#test()

func test():
	print("runtime_test         ✔️")
	runtime_test()
	print("train_algorithm_test ✔️")
	train_algorithm_test()

func build_rail(coords: Vector2i, orientation: int):
	map.set_cell_rail_placer_server(coords, orientation, 0, 1)

func build_station(coords: Vector2i, orientation: int):
	map.set_cell_rail_placer_server(coords, orientation, 2, 1)

func build_many_rails(start: Vector2i, end: Vector2i):
	map.place_to_end_rail(start, end)

func clear_test_stuff():
	map.rail_placer.clear_all_real()

func runtime_test():
	var start: float = Time.get_ticks_msec()
	var point1 = Vector2i(0, 0)
	var point2 = Vector2i(400, -100)
	var point3 = Vector2i(500, -100)
	var point4 = Vector2i(500, 100)
	var point5 = Vector2i(300, 200)
	build_many_rails(point1, point2)
	build_many_rails(point2, point3)
	build_many_rails(point3, point4)
	build_many_rails(point4, point5)
	build_many_rails(point5, point1)
	build_many_rails(point1, point3)
	clear_test_stuff()
	var end: float = Time.get_ticks_msec()
	print(str((end - start) / 1000) + " Seconds passed")

func train_algorithm_test():
	var start: float = Time.get_ticks_msec()
	var point1 = Vector2i(0, 0)
	var point2 = Vector2i(300, 0)
	
	var end_stop = Vector2i(300, -50)
	for i in 100:
		build_many_rails(point1, point2)
		point2.y = -i
	point2 = Vector2i(0, -300)
	for i in 300:
		build_many_rails(point1, point2)
		point2.x = i
	point1 = Vector2i(0, 0)
	var end: float = Time.get_ticks_msec()
	print(str((end - start) / 1000) + " Seconds passed to build")
	start = Time.get_ticks_msec()
	
	map.create_train.rpc(point1)

	var train_obj = map.get_node("Train0")
	train_obj.add_stop(end_stop)
	train_obj.start_train()
	end = Time.get_ticks_msec()
	print(str((end - start) / 1000) + " Seconds passed to pathfind")
	
	
