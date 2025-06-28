extends Window

func show_army(army_obj: army) -> void:
	update_army(army_obj)
	update_unit_list(army_obj)
	popup()

func update_army(army_obj: army) -> void:
	if army_obj != null:
		var army_info_dict: Dictionary = army_obj.get_army_client_dict()
		$stat_label.text = "Manpower: " + str(army_info_dict["manpower"]) + '\n' + "Morale: " + str(army_info_dict["morale"]) + '\n' + "Organization: " + str(army_info_dict["org"])
		$destination_label.text = "destination:\n" + str(army_info_dict["dest"])

func update_unit_list(army_obj: army) -> void:
	if army_obj != null:
		$unit_list.clear()
		for unit: base_unit in army_obj.get_units():
			$unit_list.add_item(unit_to_string(unit))

func unit_to_string(unit: base_unit) -> String:
	var toReturn: String = ""
	
	toReturn += str(unit)
	toReturn += "(" + unit.get_level_as_string() + ")"
	toReturn += "    "
	var manpower_percentage: float = float(unit.get_manpower()) / unit.get_max_manpower()
	for i: int in range(0, 5):
		var index: float = i / 5.0
		if index <= manpower_percentage:
			toReturn += "◼️"
		else:
			toReturn += "◻️"
	return toReturn

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	hide()

func _on_close_requested() -> void:
	hide()
