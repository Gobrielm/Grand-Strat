class_name apex_factory extends factory_template

func _init(new_location: Vector2i, _player_owner: int, new_inputs: Dictionary):
	super._init(new_location, _player_owner, new_inputs, {})
	local_pricer = local_price_controller.new(inputs, {})

func withdraw():
	remove_inputs(get_level())

func day_tick():
	withdraw()

func month_tick():
	for type in inputs:
		local_pricer.vary_input_price(get_monthly_demand(type), type)
