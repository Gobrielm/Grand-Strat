class_name ai_manager extends Node

static var singleton_instance: ai_manager

var ai_timer: Timer

#potentially unnecessary
var country_ais: Dictionary[int, Dictionary] = {} #Country id -> Dictionary[int, bool] ie Set

#AI id's are only negitive
var ai_instances: Dictionary[int, ai_base] #All Ai's put in one set to check for ids, and to interate over

func _init() -> void:
	assert(singleton_instance == null, "Cannot create multiple instances of singleton!")
	singleton_instance = self
	ai_timer = Timer.new()
	ai_timer.wait_time = 0.05
	ai_timer.connect("timeout", _on_ai_timer_timeout)
	#ai_timer.autostart = true

static func get_instance() -> ai_manager:
	assert(singleton_instance != null, "Train_Manager has not be created, and has been accessed")
	return singleton_instance

func create_new_economy_ai(country_id: int) -> int:
	var ai_id: int = get_unique_id()
	money_controller.get_instance().add_peer(ai_id)
	var new_ai: economy_ai = economy_ai.new(ai_id, country_id)
	if !country_ais.has(country_id):
		country_ais[country_id] = {}
	country_ais[country_id][ai_id] = true
	ai_instances[ai_id] = new_ai
	return ai_id

func destory_economy_ai(ai_id: int) -> void:
	money_controller.get_instance().delete_peer(ai_id)
	var country_id: int = ai_instances[ai_id].country_id
	country_ais[country_id].erase(ai_id)
	ai_instances.erase(ai_id)

func get_unique_id() -> int:
	#Ids -1 -> -10 are saved for specific purposes
	var toReturn: int = (randi() % 100000000) * -1
	while ai_instances.has(toReturn):
		toReturn = (randi() % 100000000) * -1
	return toReturn

func acknowledge_pending_deferred_call(ai_id: int) -> void:
	#If ai_instance is destoryed, still fine to call
	if ai_instances.has(ai_id):
		ai_instances[ai_id].acknowledge_pending_deferred_call()

func get_ai(ai_id: int) -> ai_base:
	if ai_instances.has(ai_id):
		return ai_instances[ai_id]
	return null

func _on_ai_timer_timeout() -> void:
	for ai: ai_base in ai_instances.values():
		ai.process()
