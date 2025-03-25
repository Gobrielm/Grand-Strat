extends Node

func _ready() -> void:
	var term = Terminal.create(Vector2i(0, 0), 1)
	print(term)
