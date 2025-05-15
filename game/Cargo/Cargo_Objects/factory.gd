class_name factory extends factory_template

func _init(new_location: Vector2i, _player_owner: int, new_inputs: Dictionary, new_outputs: Dictionary) -> void:
	super._init(new_location, _player_owner, new_inputs, new_outputs)

func check_recipe() -> bool:
	return check_inputs() and check_outputs()

func check_inputs() -> bool:
	for index: int in inputs:
		var amount: int = inputs[index]
		if storage[index] < amount:
			return false
	return true

func check_outputs() -> bool:
	for index: int in outputs:
		var amount: int = outputs[index]
		if max_amount - storage[index] < amount:
			return false
	return true

func create_recipe() -> void:
	var batch_size: int = get_batch_size()
	remove_inputs(batch_size)
	add_outputs(batch_size)

func day_tick() -> void:
	if check_recipe():
		create_recipe()
	if trade_orders.size() != 0:
		distribute_cargo()

func month_tick() -> void:
	for type: int in inputs:
		local_pricer.vary_input_price(get_monthly_demand(type), type)
	for type: int in outputs:
		local_pricer.vary_output_price(get_monthly_supply(type), type)
