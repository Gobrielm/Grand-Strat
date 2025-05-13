class_name ai_train extends train

var network_id: int

var cargo_to_load: Dictionary[int, int] = {}

func _input(event: InputEvent) -> void:
	if event.is_action_pressed("click") and visible:
		open_menu(map.get_mouse_local_to_map())

func get_last_stop() -> Vector2i:
	return stops.back()

func get_first_stop() -> Vector2i:
	return stops.front()

func start_loading() -> void:
	super.start_loading()
	create_cargo_to_load()

func done_loading() -> void:
	super.done_loading()
	cargo_to_load.clear()

func create_cargo_to_load() -> void:
	#Decide what cargo we should load here then follow in each load tick
	var storage_to_fill: int = cargo_hold.max_amount - cargo_hold.get_current_hold_total()
	var amount_filled: int = 0
	var station_obj: station = terminal_map.get_station(location)
	var current_station_num: int = (stop_number + 1) % stops.size()
	#Start with next station and go until filled or until loop
	var other_station: station = terminal_map.get_station(stops[current_station_num])
	while other_station != self and storage_to_fill > amount_filled:
	
		var stack: sorted_stack = sorted_stack.new()
		for type: int in station_obj.get_current_hold():
			var profit_rate: float = get_price_diff_between_stations(other_station, station_obj, type)
			if profit_rate > 0.0:
				stack.insert_element(type, profit_rate)
		
		
		while !stack.is_empty():
			var type: int = stack.pop_top()
			var amount: int = min(station_obj.get_cargo_amount(type), storage_to_fill - amount_filled, other_station.get_desired_cargo_from_train(type))
			amount_filled += amount
			cargo_to_load[type] += amount
			if storage_to_fill == amount_filled:
				break
		
		current_station_num = (stop_number + 1) % stops.size()
		other_station = terminal_map.get_station(stops[current_station_num])

func get_price_diff_between_stations(dest_stat: station, source_stat: station, type: int) -> float:
	if dest_stat.does_accept(type):
		return 1 - (source_stat.get_local_price(type) / dest_stat.get_local_price(type))
	return 0.0

func load_tick() -> void:
	var amount_loaded: int = 0
	var station_obj: station = terminal_map.get_station(location)
	var current_hold: Dictionary = station_obj.get_current_hold()
	if hold_is_empty(current_hold):
		done_loading()
	for type: int in cargo_to_load:
		if cargo_to_load.has(type):
			var price: float = station_obj.get_local_price(type) #PBUG: Needs to use price and money
			var amount: int = min(LOAD_TICK_AMOUNT - amount_loaded,current_hold[type], cargo_to_load[type])  #PBUG: Needs to know how much it can load here
			station_obj.local_pricer.report_attempt(type, amount)
			var amount_actually_loaded: int = cargo_hold.add_cargo(type, amount)
			cargo_to_load[type] -= amount_actually_loaded
			if cargo_to_load[type] <= 0:
				cargo_to_load.erase(type)
			amount_loaded += amount_actually_loaded
			station_obj.sell_cargo(type, amount_actually_loaded, price) #AAAAAAAAAAA
			if amount_loaded == LOAD_TICK_AMOUNT:
				break

func unload_tick(obj: station) -> void:
	var amount_unloaded: int = 0
	var accepts: Dictionary = obj.get_accepts()
	for type: int in accepts:
		var price: float = obj.get_local_price(type)
		var amount_desired: int = obj.get_desired_cargo_from_train(type)
		obj.local_pricer.report_attempt(type, amount_desired)
		var amount_to_transfer: int = min(amount_desired, LOAD_TICK_AMOUNT - amount_unloaded)
		var amount: int = cargo_hold.transfer_cargo(type, amount_to_transfer)
		obj.buy_cargo(type, amount, price)
		money_controller.get_instance().remove_money_from_player(id, round(amount * price))
		amount_unloaded += amount
		
		if amount_unloaded == LOAD_TICK_AMOUNT:
			return
	if amount_unloaded < LOAD_TICK_AMOUNT:
		done_unloading()
