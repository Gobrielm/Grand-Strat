class_name base_factory extends factory_template

func _init(new_location: Vector2i, _player_owner: int, new_outputs: Dictionary):
	super._init(new_location, _player_owner, {}, new_outputs)
	local_pricer = local_price_controller.new({}, outputs)

func produce():
	add_outputs(get_level())

func day_tick():
	produce()
	if trade_orders.size() > 0:
		distribute_cargo()

func month_tick():
	for type in outputs:
		local_pricer.vary_output_price(type)
