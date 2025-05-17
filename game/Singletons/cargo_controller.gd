class_name cargo_controller extends Node

static var singleton_instance: cargo_controller

var map_node: Node
var clock: clock_singleton = clock_singleton.get_instance()

#Add second tick instead for use

@export var DAY_LENGTH_TIME: int = 1

func _init() -> void:
	assert(singleton_instance == null, "Cannot create multiple instances of singleton!")
	singleton_instance = self

static func get_instance() -> cargo_controller:
	assert(singleton_instance != null, "Train_Manager has not be created, and has been accessed")
	return singleton_instance

func _ready() -> void:
	$day_tick.wait_time = DAY_LENGTH_TIME

func get_day_length() -> int:
	return DAY_LENGTH_TIME

func assign_map_node(_map_node: Node) -> void:
	map_node = _map_node
	$day_tick.autostart = true

func change_speed(p_speed: float) -> void:
	$day_tick.wait_time = p_speed

func get_game_speed() -> int:
	return clock_singleton.get_instance().get_game_speed()

func _process(delta: float) -> void:
	train_manager.get_instance().process(delta * get_game_speed())

func _on_day_tick_timeout() -> void:
	terminal_map._on_day_tick_timeout()
	map_node._on_day_tick_timeout()
	#Calls server first as it needs correct day
	clock.iterate_day()
	if clock.is_next_month():
		_on_month_tick_timeout()
	clock.iterate_day.rpc()

func _on_month_tick_timeout() -> void:
	terminal_map._on_month_tick_timeout()
	map_node._on_month_tick_timeout()
