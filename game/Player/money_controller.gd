class_name money_controller extends Node

var money: Dictionary = {}

const INTIAL_AMOUNT_OF_MONEY: int = 100000

static var singleton_instance: money_controller

func _init(peers: Array) -> void:
	assert(singleton_instance == null, "Cannot create multiple instances of singleton!")
	singleton_instance = self
	peers.append(1)
	for peer: int in peers:
		money[peer] = INTIAL_AMOUNT_OF_MONEY

static func get_instance() -> money_controller:
	assert(singleton_instance != null, "Money_Manager has not be created, and has been accessed")
	return singleton_instance

func add_peer(new_id: int) -> void:
	assert(!money.has(new_id))
	money[new_id] = INTIAL_AMOUNT_OF_MONEY

func delete_peer(id: int) -> void:
	assert(money.has(id))
	money.erase(id)

func add_money_to_player(id: int, amount: float) -> void:
	money[id] += amount
	var world_map: TileMapLayer = Utils.world_map
	world_map.update_money_label.rpc_id(id, get_money(id))

func remove_money_from_player(id: int, amount: float) -> void:
	add_money_to_player(id, -amount)

func get_money(id: int) -> float:
	return money[id]

func get_money_dictionary() -> Dictionary:
	return money

func player_has_enough_money(id: int, amount: int) -> bool:
	return get_money(id) >= amount
