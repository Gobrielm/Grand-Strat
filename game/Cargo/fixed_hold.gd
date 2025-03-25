class_name fixed_hold extends hold

var accepts: Dictionary #Cargo the hold accepts

func _init(new_location: Vector2i, _player_owner: int):
	super._init(new_location, _player_owner)
	accepts = {}

#Return amount added
func add_cargo(type: int, amount: int) -> int:
	if accepts.has(type):
		var amount_to_add = min(max_amount - storage[type], amount)
		storage[type] += amount_to_add
		return amount_to_add
	return 0

func add_cargo_ignore_accepts(type: int, amount: int) -> int:
	var amount_to_add = min(max_amount - storage[type], amount)
	storage[type] += amount_to_add
	return amount_to_add

func transfer_cargo(type: int, amount: int) -> int:
	var new_amount = min(storage[type], amount)
	storage[type] -= new_amount
	return new_amount

func get_desired_cargo(type: int) -> int:
	if accepts.has(type):
		return max_amount - storage[type]
	return 0

func reset_accepts():
	accepts = {}

func get_accepts() -> Dictionary:
	return accepts

func add_accept(type: int):
	accepts[type] = true

func remove_accept(type: int):
	accepts.erase(type)

func does_accept(type: int) -> bool:
	return accepts.has(type)
