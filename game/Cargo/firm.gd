class_name firm extends terminal

func _init(new_location: Vector2i, _player_owner: int) -> void:
	super._init(new_location, _player_owner)

func get_amount_can_buy(amount_per: float) -> int:
	return floor(get_cash() / amount_per)

func add_cash(amount: float) -> void:
	money_controller.get_instance().add_money_to_player(player_owner, amount)

func remove_cash(amount: float) -> void:
	money_controller.get_instance().remove_money_from_player(player_owner, amount)

func get_cash() -> float:
	return money_controller.get_instance().get_money(player_owner)

func transfer_cash(amount: float) -> float:
	amount = min(get_cash(), amount)
	remove_cash(amount)
	return amount
