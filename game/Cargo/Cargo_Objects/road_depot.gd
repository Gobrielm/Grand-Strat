#class_name road_depot extends Station
#
##TODO: Make a wrapped c++ road_depot then implement most in here
#
#var supplied_tiles: Dictionary[Vector2i, int] = {}
#
#const MAX_DISTANCE: int = 5
##If distance is 5 then 5 * 1 = 5 cargo delievered
#const CARGO_DELIEVERED_PER_UNIT_DISTANCE: int = 1
#
#func _init(coords: Vector2i, _player_owner: int) -> void:
	#assert(false, "Not finished implementing")
	#super._init(coords, _player_owner)
#
#func distribute_on_roads() -> void:
	#for type: int in get_current_hold():
		#distribute_type(type)
#
#func distribute_type(type: int) -> void:
	#for tile: Vector2i in supplied_tiles:
		#var broker_obj: Broker = terminal_map.get_instance().get_broker(tile)
		#if broker_obj != null and broker_obj.does_accept(type):
			##Only sends stuff inside country
			#if tile_ownership.get_instance().is_owned(player_owner, broker_obj.get_location()):
				#distribute_type_to_broker(type, broker_obj)
#
#func distribute_type_to_broker(type: int, broker_obj: Broker) -> void:
	#var coords: Vector2i = broker_obj.get_location()
	#var amount: int = broker_obj.get_amount_to_add(type, supplied_tiles[coords])
	#amount = transfer_cargo(type, amount)
	#broker_obj.add_cargo(type, amount)
#
#func day_tick() -> void:
	#pass
