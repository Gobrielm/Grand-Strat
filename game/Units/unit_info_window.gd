extends Window

func show_unit(unit_info_array: Array) -> void:
	update_unit(unit_info_array)
	popup()

func update_unit(unit_info_array: Array) -> void:
	if !unit_info_array.is_empty():
		$stat_label.text = "Manpower: " + str(unit_info_array[3]) + '\n' + "Morale: " + str(unit_info_array[4]) + '\n' + "Experience: " + str(unit_info_array[5])
		$destination_label.text = "destination: " + str(unit_info_array[2])
		

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	hide()

func _on_close_requested() -> void:
	hide()
