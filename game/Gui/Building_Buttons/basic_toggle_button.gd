extends Button
@export var active: bool = false

func _on_pressed() -> void:
	active = !active
	if active:
		modulate = Color(1, 0.75, 1, 1)
	else:
		modulate = Color(1, 1, 1, 1)

func unpress() -> void:
	active = false
	modulate = Color(1, 1, 1, 1)
