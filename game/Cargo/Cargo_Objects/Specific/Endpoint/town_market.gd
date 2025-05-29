class_name town_market extends hold

var cash: float = 1000
var desired: Dictionary[int, float] = {} #Maps types -> Amount that was wanted last month
var supply: Array[int] = []
var demand: Array[int] = []
var prices: Array[float] = []

func _init() -> void:
	#Neither parameter matters
	super._init(Vector2i(0, 0), 0)
	var base_prices: Dictionary = terminal_map.base_prices
	for base_price: float in base_prices.values():
		supply.append(0)
		demand.append(0)
		prices.append(base_price)

func update_size(p_size: int) -> void:
	change_max_storage(p_size)

# === Cash Transactions ===
func add_cash(amount: float) -> void:
	cash += amount

func remove_cash(amount: float) -> void:
	cash -= amount

func get_cash() -> float:
	return cash

# === Trading ===
func get_fulfillment(type: int) -> float:
	if supply[type] == 0:
		return 5
	return float(demand[type]) / supply[type]

#From perspecitive of market
func report_attempt_to_sell(type: int, amount: int) -> void:
	demand[type] += amount

func get_desired_cargo_to_load(type: int, price_per: float) -> int:
	if is_price_acceptable(type, price_per):
		var amount_could_get: int = min(max_amount - get_cargo_amount(type), get_amount_can_buy(price_per))
		return min(desired[type], amount_could_get)
	return 0

func get_local_price(type: int) -> float:
	return prices[type]

#Assuming they are buying
func is_price_acceptable(type: int, price_per: float) -> bool:
	return get_local_price(type) >= price_per

func buy_cargo(type: int, amount: int, price: float) -> void:
	add_cargo(type, amount)
	supply[type] += amount
	remove_cash(round(amount * price))

#Returns with the amount of cargo sold
func sell_cargo(type: int, amount: int, price: float) -> int:
	amount = transfer_cargo(type, amount)
	add_cash(round(price * amount))
	return amount

func adjust_prices() -> void:
	var base_prices: Dictionary = terminal_map.base_prices
	for cargo_name: String in base_prices:
		var type: int = terminal_map.get_cargo_type(cargo_name)
		var percentage: float = float(demand[type]) / supply[type]
		percentage = min(percentage, 1.5)
		percentage = max(percentage, 0.5)
		prices[type] = base_prices[cargo_name] * percentage
		supply[type] = 0
		demand[type] = 0

func month_tick() -> void:
	adjust_prices()
