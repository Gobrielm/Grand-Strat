extends Node


func _ready() -> void:
	var pop: BasePop = BasePop.create(1, null)
	var d: Dictionary[int, float] = {0: 1}
	BasePop.create_base_needs(d)
	print(pop)
	print(pop.get_desired(0, 10.0))
