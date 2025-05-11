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

func get_next_stop() -> Vector2i:
	return stops[(stop_number + 1) % stops.size()]

func start_loading() -> void:
	super.start_loading()
	create_cargo_to_load()

func create_cargo_to_load() -> void:
	#Decide what cargo we should load here then follow in each load tick
	var storage_to_fill: int = cargo_hold.max_amount - cargo_hold.get_current_hold_total()
	var station_obj: station = terminal_map.get_station(location)
	var next_stop: station = terminal_map.get_station(get_next_stop())
	var stack: sorted_stack = sorted_stack.new()
	for type: int in station_obj.get_current_hold():
		var profit_rate: float = get_price_diff_between_stations(next_stop, station_obj, type)
		if profit_rate > 0.0:
			stack.insert_element(type, profit_rate)
	
	var amount_filled: int = 0
	#Generally dedicate about 75% of hold towards next stop
	var amount_to_load: int = round(storage_to_fill * 0.75)
	while !stack.is_empty():
		var type: int = stack.pop_top()
		var amount: int = min(station_obj.get_cargo_amount(type), amount_to_load - amount_filled)
		amount_filled += amount
	
	storage_to_fill -= amount_filled
	#Fill rest for next station then next station

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
	for type: int in current_hold:
		if !station_obj.does_accept(type):
			var amount: int = min(LOAD_TICK_AMOUNT - amount_loaded, current_hold[type])
			var amount_actually_loaded: int = cargo_hold.add_cargo(type, amount)
			amount_loaded += amount_actually_loaded
			station_obj.remove_cargo(type, amount_actually_loaded)
			if amount_loaded == LOAD_TICK_AMOUNT:
				break
