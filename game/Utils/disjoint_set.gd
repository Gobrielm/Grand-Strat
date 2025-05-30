class_name disjoint_set extends RefCounted

var backing_dict: Dictionary[int, int] #ids -> parent/size

func _init() -> void:
	backing_dict = {}

func add_member(id: int) -> void:
	backing_dict[id] = -1

func union(id1: int, id2: int) -> void:
	#Same id, just return
	if id1 == id2:
		return
	#Both are disjoint
	elif backing_dict[id1] == backing_dict[id2] and backing_dict[id1] == -1:
		backing_dict[id1] = -2
		backing_dict[id2] = id1
	#One is disjoint
	elif backing_dict[id1] == -1:
		var parent_id: int = get_parent_index(id2)
		backing_dict[id1] = parent_id
		increment_size_parent_tree(parent_id)
	elif backing_dict[id2] == -1:
		var parent_id: int = get_parent_index(id1)
		backing_dict[id2] = parent_id
		increment_size_parent_tree(parent_id)
	#Both are in same set, Do nothing
	elif get_parent_index(id1) == get_parent_index(id2):
		assert(false, "Should never have same parents but not id1 == id2")
		return
	#Both are in different sets
	#Merge to the id1 tree, as it is arbitrary
	else:
		var parent_id: int = get_parent_index(id1)
		var size_to_move: int = get_tree_size(id2)
		backing_dict[id2] = parent_id
		increment_size_parent_tree(parent_id, size_to_move)
		move_all_from(id2, id1)

func increment_size_parent_tree(id: int, amount: int = 1) -> void:
	var parent_id: int = get_parent_index(id)
	backing_dict[parent_id] -= amount

func decrement_size_parent_tree(id: int, amount: int = 1) -> void:
	var parent_id: int = get_parent_index(id)
	backing_dict[parent_id] += amount

func get_parent_index(id: int) -> int:
	while backing_dict[id] > 0:
		id = backing_dict[id]
	return id

func get_tree_size(id: int) -> int:
	var parent_id: int = get_parent_index(id)
	return backing_dict[parent_id] * -1

func get_ids_that_have_parent(parent_id: int) -> Array[int]:
	var toReturn: Array[int]
	for id: int in backing_dict:
		if does_id_have_parent(id, parent_id):
			toReturn.append(id)
	return toReturn

func does_id_have_parent(id: int, parent_id: int) -> bool:
	while backing_dict[id] > 0:
		id = backing_dict[id]
		if id == parent_id:
			return true
	return false

func move_all_from(from_id: int, to_id: int) -> void:
	for id: int in get_ids_that_have_parent(from_id):
		backing_dict[id] = to_id

func get_biggest_parent_id() -> int:
	var biggest_size: int = 1 #It will be negitive
	var index: int = -1
	for id: int in backing_dict:
		if biggest_size > backing_dict[id]:
			biggest_size = backing_dict[id]
			index = id
	return index
