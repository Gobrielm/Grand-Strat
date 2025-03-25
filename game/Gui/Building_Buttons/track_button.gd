extends Button
@export var active = false


func _on_pressed():
	unpress_other_buttons()
	active = !active
	if active:
		modulate = Color(1, 0.75, 1, 1)
	else:
		modulate = Color(1, 1, 1, 1)

func unpress():
	active = false
	modulate = Color(1, 1, 1, 1)

func unpress_other_buttons():
	get_parent().get_parent().unpress_all_buttons()
