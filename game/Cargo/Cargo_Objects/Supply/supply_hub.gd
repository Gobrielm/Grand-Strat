#class_name supply_hub extends Station
#
##TODO: Make road depot have orders have be way more similar to station
##Can buy, but only send cargo to units
##Owned things give supply, but need supply hubs for privatly owned goods
#
#func _init(coords: Vector2i, _player_owner: int) -> void:
	#super._init(coords, _player_owner)
#
#func distribute_cargo() -> void:
	#supply_armies()
#
#func place_order(type: int, amount: int, buy: bool, max_price: float) -> void:
	#if buy:
		#super.place_order(type, amount, buy, max_price)
#
#func edit_order(type: int, amount: int, buy: bool, max_price: float) -> void:
	#if buy:
		#super.edit_order(type, amount, buy, max_price)
