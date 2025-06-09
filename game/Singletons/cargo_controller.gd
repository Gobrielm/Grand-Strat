class_name cargo_controller extends Node

static var singleton_instance: cargo_controller

var map_node: Node
var clock: clock_singleton = clock_singleton.get_instance()
var ticks: float = 0.0
var game_speed: int = 1
var day_thread: Thread = Thread.new()

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

func day_tick() -> void: #Crash after first month tick
	TerminalMap.get_instance()._on_day_tick_timeout()
	clock.iterate_day()
	if clock.is_next_month():
		_on_month_tick_timeout()

var start: float = 0.0
var end: float = 0.0

func _on_month_tick_timeout() -> void:
	start = Time.get_ticks_msec()
	TerminalMap.get_instance()._on_month_tick_timeout()
	end = Time.get_ticks_msec()
	print(str((end - start) / 1000) + " Seconds passed for one month cycle")
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
