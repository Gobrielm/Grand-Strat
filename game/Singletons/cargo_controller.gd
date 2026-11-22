class_name cargo_controller extends Node

static var singleton_instance: cargo_controller

var map_node: Node
var clock: clock_singleton = clock_singleton.get_instance()
var paused: bool = false
var ticks: float = 0.0
var game_speed: int = 1
var m: Mutex = Mutex.new()
var threads_request_pause: int = 0
var thread: Thread = Thread.new()

@export var SECONDS_IN_DAY: int = 1

func _init() -> void:
	assert(singleton_instance == null, "Cannot create multiple instances of singleton!")
	singleton_instance = self
	backend_pause()

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
	paused = p_pause

func frontend_pause(p_pause: bool) -> void:
	if p_pause:
		backend_pause()
	else:
		backend_unpause()

func backend_pause() -> void:
	m.lock()
	threads_request_pause += 1
	change_pause(true)
	m.unlock()

func backend_unpause() -> void:
	m.lock()
	threads_request_pause -= 1
	if threads_request_pause < 0: push_error("Backend Pause requests are unsynced")
	if threads_request_pause == 0:
		change_pause(false)
	m.unlock()

func get_game_speed() -> int:
	return clock_singleton.get_instance().get_game_speed()

func day_tick() -> void:
	TerminalMap.get_instance()._on_day_tick_timeout()
	clock.iterate_day()
	if clock.is_next_month():
		_on_month_tick_timeout()

func _on_month_tick_timeout() -> void:
	if thread.is_started():
		thread.wait_to_finish()
	thread.start(call_month_tick.bind())

func call_month_tick() -> void:
	backend_pause()
	var start: float = Time.get_ticks_msec()
	CountryManager.get_instance().month_tick()
	DataCollector.get_instance().month_tick()
	RoadMap.get_instance().month_tick()
	TerminalMap.get_instance()._on_month_tick_timeout()
	Utils.unit_map._on_month_tick_timeout()
	var time_taken: float = (Time.get_ticks_msec() - start) / 1000
	print(str(time_taken) + " seconds have passed for one whole month.")
	backend_unpause()

func _process(delta: float) -> void:
	if paused:
		return
	ticks += delta * (game_speed * 6) # * 6
	
	#if train_manager.has_instance():
		#train_manager.get_instance().process(delta)
	
	if ticks >= SECONDS_IN_DAY:
		day_tick()
		ticks = 0

func get_tick_length() -> float:
	return $basic_tick.wait_time
