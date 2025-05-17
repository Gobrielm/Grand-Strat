extends Node

func _ready() -> void:
	var queue: priority_queue = priority_queue.new()
	queue.insert_element('a', 5.0)
	queue.insert_element('b', 1.0)
	print(queue.backing_array)
