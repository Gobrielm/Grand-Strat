class_name cargo_controller extends Node

static var singleton_instance: cargo_controller

var map_node: Node
var clock: clock_singleton = clock_singleton.get_instance()
var paused: bool = false
var ticks: float = 0.0
var game_speed: int = 1
var m: Mutex = Mutex.new()
var threads_request_pause: int = 0

@export var SECONDS_IN_DAY: int = 1

func _init() -> void:
	assert(singleton_instance == null, "Cannot create multiple instances of singleton!")
	singleton_instance = self

static func get_instance() -> cargo_controller:
	assert(singleton_instance != null, "Cargo Controller has not be created, and has been accessed")
	return singleton_instance

func _ready() -> void:
	pass

func change_speed(p_speed: int) -> void:
	m.lock()
	game_speed = p_speed
	m.unlock()

func change_pause(p_pause: bool) -> void:
	m.lock()
	paused = p_pause
	m.unlock()

func frontend_pause(p_pause: bool) -> void:
	if p_pause:
		backend_pause()
	else:
		backend_unpause()

func backend_pause() -> void:
	m.lock()
	threads_request_pause += 1
	m.unlock()
	change_pause(true)

func backend_unpause() -> void:
	m.lock()
	threads_request_pause -= 1
	var status: bool = threads_request_pause == 0
	if threads_request_pause < 0: push_error("Backend Pause requests are unsynced")
	m.unlock()
	if status:
		change_pause(false)

func get_game_speed() -> int:
	return clock_singleton.get_instance().get_game_speed()

func day_tick() -> void:
	TerminalMap.get_instance()._on_day_tick_timeout()
	clock.iterate_day()
	if clock.is_next_month():
		_on_month_tick_timeout()

var start: float = 0.0
var end: float = 0.0

func _on_month_tick_timeout() -> void:
	CountryManager.get_instance().month_tick()
	DataCollector.get_instance().month_tick()
	ProvinceManager.get_instance().month_tick()
	RoadMap.get_instance().month_tick()
	TerminalMap.get_instance()._on_month_tick_timeout()
	Utils.unit_map._on_month_tick_timeout()

func _process(delta: float) -> void:
	if paused:
		return
	ticks += delta * (game_speed * 6) # * 6
	
	if train_manager.has_instance():
		train_manager.get_instance().process(delta)
	
	if ticks >= SECONDS_IN_DAY:
		day_tick()
		ticks = 0

func get_tick_length() -> float:
	return $basic_tick.wait_time
