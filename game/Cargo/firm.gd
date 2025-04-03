class_name firm extends terminal

const INITIAL_CASH: int = 1000

var cash: int

func _init(new_location: Vector2i, _player_owner: int) -> void:
	super._init(new_location, _player_owner)
	cash = INITIAL_CASH

func get_amount_can_buy(amount_per: float) -> int:
	return floor(float(cash) / amount_per)

func add_cash(amount: int) -> void:
	cash += amount

func remove_cash(amount: int) -> void:
	cash -= amount

func get_cash() -> int:
	return cash

func transfer_cash(amount: int) -> int:
	amount = min(get_cash(), amount)
	remove_cash(amount)
	return amount
