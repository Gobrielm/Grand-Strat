class_name terminal extends Node

var location: Vector2i
var player_owner: int

func _init(_location: Vector2i, _player_owner: int) -> void:
	location = _location
	player_owner = _player_owner

func get_location() -> Vector2i:
	return location

func update_location(new_location: Vector2i) -> void:
	location = new_location

func get_player_owner() -> int:
	return player_owner
