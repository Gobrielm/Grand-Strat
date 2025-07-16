extends Node

func _ready() -> void:
	randomize()
	$Unit_battler.set_cw(16)
	var army1: army = army.new(1, Vector2i(0, 0))
	army1.add_unit(infantry.new())
	#for i: int in range(0, 10):
		#var num: int = randi() % 3
		#if num == 0:
			#army1.add_unit(infantry.new())
		#elif num == 1:
			#army1.add_unit(calvary.new())
		#else:
			#army1.add_unit(artillery.new())
	$Unit_battler.add_atk_army(army1)
	
	var army2: army = army.new(2, Vector2i(0, 0))
	army2.add_unit(infantry.new())
	#for i: int in range(0, 10):
		#var num: int = randi() % 3
		#if num == 0:
			#army2.add_unit(infantry.new())
		#elif num == 1:
			#army2.add_unit(calvary.new())
		#else:
			#army2.add_unit(artillery.new())
	$Unit_battler.add_def_army(army2)
	
	$Unit_battler.start_battle()


func _on_timer_timeout() -> void:
	$Unit_battler.day_tick()
