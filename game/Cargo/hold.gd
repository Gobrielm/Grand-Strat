class_name hold extends firm

const DEFAULT_MAX_STORAGE: int = 50

var storage: Dictionary = {} #Cargo the hold has
var max_amount: int = DEFAULT_MAX_STORAGE #Max Amount of cargo the hold can hold

func _init(new_location: Vector2i, _player_owner: int, p_max_amount: int = max_amount) -> void:
	super._init(new_location, _player_owner)
	max_amount = p_max_amount
	for i: int in terminal_map.get_number_of_goods():
		storage[i] = 0

#Return amount added
func add_cargo(type: int, amount: int) -> int:
	var amount_to_add: int = get_amount_to_add(type, amount)
	storage[type] += amount_to_add
	return amount_to_add

func get_cargo_amount(type: int) -> int:
	return storage[type]

func remove_cargo(type: int, amount: int) -> void:
	storage[type] -= amount

func transfer_cargo(type: int, amount: int) -> int:
	var val: int = min(amount, storage[type])
	remove_cargo(type, val)
	return val

func get_amount_to_add(_type: int, amount: int) -> int:
	return min(max_amount - get_current_hold_total(), amount)

func get_current_hold() -> Dictionary:
	return storage

func get_current_hold_total() -> int:
	var total: int = 0
	for index: int in storage:
		total += storage[index]
	return total

func is_full() -> bool:
	return get_current_hold_total() >= max_amount

func is_empty() -> bool:
	return get_current_hold_total() == 0

func change_max_storage(p_amount: int) -> void:
	max_amount += p_amount

func does_accept(_type: int) -> bool:
	return get_current_hold_total() < max_amount
