extends Window
var type_selected
var map: TileMapLayer
# Called when the node enters the scene tree for the first time.
func _ready():
	map = get_parent()
	hide()

func _on_close_requested():
	hide()

func clear_type_selected():
	type_selected = null

func _on_infantry_button_pressed():
	type_selected = 0
	map.start_building_units()
	hide()


func _on_calvary_button_pressed():
	type_selected = 1
	map.start_building_units()
	hide()


func _on_artillery_button_pressed():
	type_selected = 2
	map.start_building_units()
	hide()


func _on_engineer_button_pressed():
	type_selected = 3
	map.start_building_units()
	hide()


func _on_officer_button_pressed():
	type_selected = 4
	map.start_building_units()
	hide()

func get_type_selected():
	return type_selected
