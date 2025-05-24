class_name cargo_controller extends Node

static var singleton_instance: cargo_controller

var map_node: Node
var clock: clock_singleton = clock_singleton.get_instance()
var ticks: float = 0.0
var game_speed: int = 1

@export var SECONDS_IN_DAY: int = 1

func _init() -> void:
	assert(singleton_instance == null, "Cannot create multiple instances of singleton!")
	singleton_instance = self

static func get_instance() -> cargo_controller:
	assert(singleton_instance != null, "Train_Manager has not be created, and has been accessed")
	return singleton_instance

func _ready() -> void:
	pass

func change_speed(p_speed: int) -> void:
	game_speed = p_speed

func get_game_speed() -> int:
	return clock_singleton.get_instance().get_game_speed()

func day_tick() -> void:
	terminal_map._on_day_tick_timeout()
	clock.iterate_day()
	if clock.is_next_month():
		_on_month_tick_timeout()

func _on_month_tick_timeout() -> void:
	terminal_map._on_month_tick_timeout()
	Utils.unit_map._on_month_tick_timeout()

func _process(delta: float) -> void:
	ticks += delta * game_speed
	
	if train_manager.has_instance():
		train_manager.get_instance().process(delta)
	
	if ticks >= SECONDS_IN_DAY:
		day_tick()
		ticks = 0

func get_tick_length() -> float:
	return $basic_tick.wait_time
