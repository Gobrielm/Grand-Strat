class_name sorted_stack extends Node

var backing_array: Array[weighted_value]

func _init() -> void:
	backing_array = []

func pop_top() -> Variant:
	return (backing_array.pop_front() as weighted_value).val

func is_empty() -> bool:
	return backing_array.is_empty()

func get_size() -> int:
	return backing_array.size()

func get_element(index: int) -> weighted_value:
	return backing_array[index]

func get_weight(index: int) -> float:
	return backing_array[index].weight

func get_value(index: int) -> Variant:
	return backing_array[index].val

func insert_element(val: Variant, weight: float) -> void:
	var element: weighted_value = weighted_value.new(val, weight)
	if get_size() == 0:
		backing_array.insert(0, element)
		return
	@warning_ignore("integer_division")
	var i: int = get_size() / 2
	if get_weight(i) > weight:
		bsearch_insert(element, i, get_size())
	elif get_weight(i) < weight:
		bsearch_insert(element, 0, i)
	else:
		backing_array.insert(i, element)

func get_middle(top: int, bot: int) -> int:
	@warning_ignore("integer_division")
	return bot - (bot - top) / 2

func bsearch_insert(element: weighted_value, top: int, bot: int) -> void:
	var weight: float = element.weight
	var i: int = get_middle(top, bot)
	if i == get_size():
		backing_array.push_back(element)
		return
	
	var other_weight: float = get_weight(i)
	if i == bot or i == top:
		if weight < other_weight:
			backing_array.insert(i - 1, element)
		elif weight > other_weight:
			if weight > get_weight(i - 1) and i != 0:
				backing_array.insert(i - 1, element)
			else:
				backing_array.insert(i, element)
		return
	
	if other_weight < weight:
		bsearch_insert(element, top, i)
	elif other_weight > weight:
		bsearch_insert(element, i, bot)
	else:
		backing_array.insert(i, element)

func get_array_of_elements() -> Array:
	var toReturn: Array = []
	for element: weighted_value in backing_array:
		toReturn.push_back(element.val)
	return toReturn

func _to_string() -> String:
	return str(backing_array)
