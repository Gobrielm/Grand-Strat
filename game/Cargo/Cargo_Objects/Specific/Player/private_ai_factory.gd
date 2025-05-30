class_name private_ai_factory extends ai_factory

#represents privately owned factory with the owner working there

func _init(p_location: Vector2i, p_inputs: Dictionary, p_outputs: Dictionary) -> void:
	super._init(p_location, 0, p_inputs, p_outputs)

var cash: float = 1000

func add_cash(amount: float) -> void:
	cash += amount

func remove_cash(amount: float) -> void:
	cash -= amount

func get_cash() -> float:
	return cash
