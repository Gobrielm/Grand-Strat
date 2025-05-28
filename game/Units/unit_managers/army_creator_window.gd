extends Window
var type_selected: int
# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	hide()

func _on_close_requested() -> void:
	hide()

func clear_type_selected() -> void:
	type_selected = -1

func _on_infantry_button_pressed() -> void:
	type_selected = 0
	province_machine.start_building_units()
	hide()


func _on_calvary_button_pressed() -> void:
	type_selected = 1
	province_machine.start_building_units()
	hide()


func _on_artillery_button_pressed() -> void:
	type_selected = 2
	province_machine.start_building_units()
	hide()


func _on_engineer_button_pressed() -> void:
	type_selected = 3
	province_machine.start_building_units()
	hide()


func _on_officer_button_pressed() -> void:
	type_selected = 4
	province_machine.start_building_units()
	hide()

func get_type_selected() -> int:
	return type_selected
