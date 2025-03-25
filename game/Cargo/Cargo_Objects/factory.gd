class_name factory extends factory_template

func _init(new_location: Vector2i, _player_owner: int, new_inputs: Dictionary, new_outputs: Dictionary):
	super._init(new_location, _player_owner, new_inputs, new_outputs)

func check_recipe() -> bool:
	return check_inputs() and check_outputs()

func check_inputs() -> bool:
	for index in inputs:
		var amount = inputs[index]
		if storage[index] < amount:
			return false
	return true

func check_outputs() -> bool:
	for index in outputs:
		var amount = outputs[index]
		if max_amount - storage[index] < amount:
			return false
	return true

func create_recipe():
	var batch_size = get_batch_size()
	remove_inputs(batch_size)
	add_outputs(batch_size)

func day_tick():
	if check_recipe():
		create_recipe()
	if trade_orders.size() != 0:
		distribute_cargo()

func month_tick():
	for type in inputs:
		local_pricer.vary_input_price(get_monthly_demand(type), type)
	for type in outputs:
		local_pricer.vary_output_price(type)
