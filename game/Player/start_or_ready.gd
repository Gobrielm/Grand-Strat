extends Button
@export var active = false


func _on_pressed():
	active = !active
	if active:
		modulate = Color(1, 0.75, 1, 1)
	else:
		modulate = Color(1, 1, 1, 1)

func unpress():
	active = false
	modulate = Color(1, 1, 1, 1)
