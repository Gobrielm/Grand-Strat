class_name ai_base extends RefCounted

#AI provinces
var id: int
var country_id: int #Otherwise known as player
var thread: Thread
var stored_tile: Vector2i
var pending_deferred_calls: int = 0

func _init(p_id: int, p_country_id: int) -> void:
	id = p_id
	country_id = p_country_id

func get_country_id() -> int:
	return country_id
