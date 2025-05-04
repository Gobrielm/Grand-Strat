extends Node

func _ready() -> void:
	var ds_set: disjoint_set = disjoint_set.new()
	ds_set.add_member(0)
	ds_set.add_member(1)
	ds_set.add_member(2)
	ds_set.add_member(3)
	ds_set.add_member(4)
	ds_set.union(0, 1)
	ds_set.union(3, 4)
	ds_set.union(2, 0)
	print(ds_set.backing_array)
