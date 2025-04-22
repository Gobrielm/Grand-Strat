extends Node

var map: TileMapLayer

func _init(new_map: TileMapLayer) -> void:
	map = new_map
	#test()

func test() -> void:
	var ownership: TileMapLayer = Utils.tile_ownership
	ownership.add_player_to_country(1, Vector2i(0, 0))
	print("runtime_test")
	runtime_test()
	print("-------------------- ✔️")
	
	print("train_algorithm_test")
	train_algorithm_test()
	print("-------------------- ✔️")
	
	print("ai_train_network_test")
	ai_rail_test()
	print("-------------------- ✔️")
	

func build_rail(coords: Vector2i, orientation: int) -> void:
	map.set_cell_rail_placer_server(coords, orientation, 0, 1)

func build_station(coords: Vector2i, orientation: int) -> void:
	map.set_cell_rail_placer_server(coords, orientation, 2, 1)

func build_many_rails(start: Vector2i, end: Vector2i) -> void:
	map.place_to_end_rail(start, end)

func clear_test_stuff() -> void:
	map.rail_placer.clear_all_real()

func runtime_test() -> void:
	var start: float = Time.get_ticks_msec()
	var point1: Vector2i = Vector2i(0, 0)
	var point2: Vector2i = Vector2i(400, -100)
	var point3: Vector2i = Vector2i(500, -100)
	var point4: Vector2i = Vector2i(500, 100)
	var point5: Vector2i = Vector2i(300, 200)
	build_many_rails(point1, point2)
	build_many_rails(point2, point3)
	build_many_rails(point1, point3)
	build_many_rails(point1, point4)
	build_many_rails(point1, point5)
	build_many_rails(point1, point2)
	clear_test_stuff()
	var end: float = Time.get_ticks_msec()
	print(str((end - start) / 1000) + " Seconds passed")

func train_algorithm_test() -> void:
	var start: float = Time.get_ticks_msec()
	var point1: Vector2i = Vector2i(0, 0)
	var point2: Vector2i = Vector2i(300, 0)
	
	var end_stop: Vector2i = Vector2i(300, -50)
	for i: int in 100:
		build_many_rails(point1, point2)
		point2.y = -i
	point2 = Vector2i(0, -300)
	for i: int in 300:
		build_many_rails(point1, point2)
		point2.x = i
	var end: float = Time.get_ticks_msec()
	print(str((end - start) / 1000) + " Seconds passed to build")
	start = Time.get_ticks_msec()
	
	map.create_train(point1)
	var train_obj: train = map.get_node("Train0")
	var orientation: Array = Utils.rail_placer.get_track_connections(end_stop)
	map.remove_rail(end_stop, orientation.find(true), 0)
	map.place_rail_general(end_stop, orientation.find(true), 2)
	train_obj.add_stop(end_stop)
	train_obj.start_train()
	end = Time.get_ticks_msec()
	print(str((end - start) / 1000) + " Seconds passed to pathfind")
	
func ai_rail_test() -> void:
	var point1: Vector2i = Vector2i(0, 0)
	#build_many_rails(point1, point2)
	#build_many_rails(point1, point2)
	#build_many_rails(point1, point2)
	#build_many_rails(point1, point2)
	#build_many_rails(point1, point2)
	var start: float = Time.get_ticks_msec()
	var test_to_run: Node = preload("res://Test/ai_rail_test.gd").new()
	test_to_run.run()
	
	var end: float = Time.get_ticks_msec()
	print(str((end - start) / 1000) + " Seconds passed to create network")
