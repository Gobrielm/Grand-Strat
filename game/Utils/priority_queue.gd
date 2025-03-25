class_name priority_queue extends Node

var backing_array: Array

func _init():
	backing_array = []

func add_item(key, item):
	for index in get_size():
		if backing_array[index][0] > key:
			backing_array.insert(index, [key, item])
			return
	backing_array.append([key, item])

func pop_top():
	if get_size() == 0:
		return null
	return backing_array.pop_front()[1]

func get_size():
	return backing_array.size()

func is_empty() -> bool:
	return get_size() == 0
