extends Node

func _ready() -> void:
	randomize()
	$Unit_battler.set_cw(16)
	var army1: army = army.new(1, Vector2i(0, 0))
	for i: int in range(0, 10):
		if randi() % 2 == 0:
			army1.add_unit(infantry.new())
		else:
			army1.add_unit(calvary.new())
	$Unit_battler.add_atk_army(army1)
	
	var army2: army = army.new(2, Vector2i(0, 0))
	for i: int in range(0, 10):
		if randi() % 2 == 0:
			army2.add_unit(infantry.new())
		else:
			army2.add_unit(calvary.new())
	$Unit_battler.add_def_army(army2)
	
	$Unit_battler.start_battle()


func _on_timer_timeout() -> void:
	$Unit_battler.day_tick()
