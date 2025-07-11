extends Node

func _ready() -> void:
	$Unit_battler.set_cw(16)
	var army1: army = army.new(1, Vector2i(0, 0))
	army1.add_unit(infantry.new())
	army1.add_unit(infantry.new())
	army1.add_unit(infantry.new())
	$Unit_battler.add_atk_army(army1)
	
	var army2: army = army.new(2, Vector2i(0, 0))
	army2.add_unit(infantry.new())
	army2.add_unit(infantry.new())
	army2.add_unit(infantry.new())
	$Unit_battler.add_def_army(army2)
	
	$Unit_battler.start_battle()


func _on_timer_timeout() -> void:
	$Unit_battler.day_tick()
