extends Node

func _ready() -> void:
	$RoadMap.place_road(Vector2i(0, 0))
	$RoadMap.place_road(Vector2i(0, 1))
	$RoadMap.place_road(Vector2i(1, 1))
	$RoadMap.place_road(Vector2i(1, 0))
