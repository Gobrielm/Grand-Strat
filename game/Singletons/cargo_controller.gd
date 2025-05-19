class_name cargo_controller extends Node

static var singleton_instance: cargo_controller

var map_node: Node
var clock: clock_singleton = clock_singleton.get_instance()
var ticks: int = 0

@export var TICKS_IN_DAY: int = 40

func _init() -> void:
	assert(singleton_instance == null, "Cannot create multiple instances of singleton!")
	singleton_instance = self

static func get_instance() -> cargo_controller:
	assert(singleton_instance != null, "Train_Manager has not be created, and has been accessed")
	return singleton_instance

func _ready() -> void:
	$basic_tick.wait_time = 1.0 / TICKS_IN_DAY

func change_speed(p_speed: float) -> void:
	$basic_tick.wait_time = 1.0 / TICKS_IN_DAY / p_speed

func get_game_speed() -> int:
	return clock_singleton.get_instance().get_game_speed()

func _process(delta: float) -> void:
	train_manager.get_instance().process(delta * get_game_speed())

func day_tick() -> void:
	terminal_map._on_day_tick_timeout()
	#Calls server first as it needs correct day
	clock.iterate_day()
	if clock.is_next_month():
		_on_month_tick_timeout()
	clock.iterate_day.rpc()

func _on_month_tick_timeout() -> void:
	terminal_map._on_month_tick_timeout()

func _on_basic_tick_timeout() -> void:
	ticks += 1
	train_manager.get_instance().process(get_tick_length())
	
	if ticks == TICKS_IN_DAY:
		day_tick()
		ticks = 0

func get_tick_length() -> float:
	return $basic_tick.wait_time
