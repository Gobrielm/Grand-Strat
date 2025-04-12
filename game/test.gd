extends Node

func _ready() -> void:
	var stack: sorted_stack = sorted_stack.new()
	stack.insert_element(0, 5)
	stack.insert_element(1, 6)
	stack.insert_element(2, 7)
	stack.insert_element(3, 4)
	stack.insert_element(4, 3)
	stack.insert_element(0, 9)
	print(stack)
	stack.insert_element(1, 7)
	print(stack)
	stack.insert_element(2, 8)
	print(stack)
	stack.insert_element(3, 10)
	print(stack)
	stack.insert_element(4, 2)
	print(stack)
	for i: int in range(50):
		stack.insert_element(4, randi() % 30)
	print(stack)
