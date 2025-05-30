class_name town_market extends hold

var cash: float = 1000
var desired: Dictionary[int, float] = {} #Maps types -> Amount that was wanted last month
var supply: Array[int] = []
var demand: Array[int] = []
var prices: Array[float] = []
var mutex: Mutex = Mutex.new()

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

func get_supply() -> Array[int]:
	var toReturn: Array[int]
	mutex.lock()
	toReturn = supply.duplicate()
	mutex.unlock()
	return toReturn

func get_cargo_amount(type: int) -> int:
	var toReturn: int = 0
	mutex.lock()
	toReturn = storage[type]
	mutex.unlock()
	return toReturn

# === Cash Transactions ===
func add_cash(amount: float) -> void:
	mutex.lock()
	cash += amount
	mutex.unlock()

func remove_cash(amount: float) -> void:
	mutex.lock()
	cash -= amount
	mutex.unlock()

func get_cash() -> float:
	var toReturn: float = 0.0
	mutex.lock()
	toReturn = cash
	mutex.unlock()
	return toReturn

# === Trading ===
func get_fulfillment(type: int) -> float:
	mutex.lock()
	if supply[type] == 0:
		mutex.unlock()
		return 5
	var toReturn: float = float(demand[type]) / supply[type]
	mutex.unlock()
	return toReturn

#From perspecitive of market
func report_attempt_to_sell(type: int, amount: int) -> void:
	mutex.lock()
	demand[type] += amount
	mutex.unlock()

func get_desired_cargo_to_load(type: int, price_per: float) -> int:
	if is_price_acceptable(type, price_per) and desired.has(type):
		var amount_could_get: int = min(max_amount - get_cargo_amount(type), get_amount_can_buy(price_per))
		return min(desired[type], amount_could_get)
	return 0

func get_local_price(type: int) -> float:
	var toReturn: float = 0.0
	mutex.lock()
	toReturn = prices[type]
	mutex.unlock()
	return toReturn

#Assuming they are buying
func is_price_acceptable(type: int, price_per: float) -> bool:
	return get_local_price(type) >= price_per

func buy_cargo(type: int, amount: int, price: float) -> void:
	add_cargo(type, amount)
	mutex.lock()
	supply[type] += amount
	mutex.unlock()
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
		mutex.lock()
		prices[type] = base_prices[cargo_name] * percentage
		supply[type] = 0
		demand[type] = 0
		mutex.unlock()

func month_tick() -> void:
	adjust_prices()
