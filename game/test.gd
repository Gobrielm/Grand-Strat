extends Node

func _ready() -> void:
	var army_obj: army = army.new()
	army_obj.add_unit(infantry.new(Vector2i(0, 0), 1))
	army_obj.add_unit(infantry.new(Vector2i(0, 0), 1))
	army_obj.add_unit(infantry.new(Vector2i(0, 0), 1))
	army_obj.add_unit(calvary.new(Vector2i(0, 0), 1))
	army_obj.add_unit(calvary.new(Vector2i(0, 0), 1))
	army_obj.add_unit(calvary.new(Vector2i(0, 0), 1))
	army_obj.add_unit(artillery.new(Vector2i(0, 0), 1))
	army_obj.add_unit(artillery.new(Vector2i(0, 0), 1))
	army_obj.add_unit(artillery.new(Vector2i(0, 0), 1))
	
	var new_army: army = army_obj.split()
	
	print(army_obj)
	print(new_army)
	
	army_obj.merge(new_army)
	
	print(army_obj)
	print(new_army)
