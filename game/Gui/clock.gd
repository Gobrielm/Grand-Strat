class_name clock_singleton extends Control

static var singleton_instance: clock_singleton

var day: int = 1 #1 indexed
var month: int = 1 #1 indexed
var year: int = 1845

var month_lengths: Array[int] = [0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]

func _init() -> void:
	assert(singleton_instance == null, "Cannot create multiple instances of singleton!")
	singleton_instance = self

static func get_instance() -> clock_singleton:
	assert(singleton_instance != null, "Train_Manager has not be created, and has been accessed")
	return singleton_instance

@rpc("authority", "call_remote", "unreliable")
func iterate_day() -> void:
	day += 1
	if day > month_lengths[month]:
		#If february, is a leap year, and is feb 29, then 
		if month == 2 and is_leap_year() and day == 29:
			day += 1
			return
		month += 1
		day = 1
	if month > 12:
		month = 1
		year += 1
	update_gui()

func get_days_in_current_month() -> int:
	return month_lengths[month]

func is_leap_year() -> bool:
	return year % 4

func update_gui() -> void:
	var date: String = ""
	if month < 10:
		date += "0"
	date += str(month) + "/"
	
	if day < 10:
		date += "0"
	date += str(day) + "/" + str(year)
	
	$Label.text = date

func is_next_month() -> bool:
	return day == 1

func _on_game_speed_drag_ended(value_changed: bool) -> void:
	if value_changed:
		cargo_controller.get_instance().change_speed(1 / $game_speed.value)
		
