extends Camera2D
var last_mouse_position: Vector2
@onready var coords_label: Label = $CanvasLayer/Coordinate_Label
# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	if multiplayer.get_unique_id() == 1:
		$nation_picker/start_or_ready.text = "Start"
	else:
		$nation_picker/start_or_ready.text = "Ready"
	$CanvasLayer/Desync_Label.visible = false

func _process(_delta: float) -> void:
	if is_mouse_hovering():
		state_machine.hovering_over_gui_active()
	else:
		state_machine.hovering_over_gui_inactive()
	if Input.is_action_pressed("pan_left") and !state_machine.is_gui_pressed():
		position.x -= 5 / zoom.x
	elif Input.is_action_pressed("pan_right") and !state_machine.is_gui_pressed():
		position.x += 5 / zoom.x
	elif Input.is_action_pressed("pan_down") and !state_machine.is_gui_pressed():
		position.y -= 5 / zoom.x
	elif Input.is_action_pressed("pan_up") and !state_machine.is_gui_pressed():
		position.y += 5 / zoom.x
	if Input.is_action_just_released("zoom_in") and zoom.x < 2 and !state_machine.is_gui_pressed():
		zoom.x += 0.05
		zoom.y += 0.05
	elif Input.is_action_just_released("zoom_out") and zoom.x > 0.06 and !state_machine.is_gui_pressed():
		zoom.x -= 0.05
		zoom.y -= 0.05
	elif Input.is_action_pressed("pan_mouse") and last_mouse_position and !state_machine.is_gui_pressed():
		var instant_mouse_movement: Vector2 = last_mouse_position - get_viewport().get_mouse_position()
		position.x += 1 / zoom.x * instant_mouse_movement.x
		position.y += 1 / zoom.y * instant_mouse_movement.y
	last_mouse_position = get_viewport().get_mouse_position()
	update_resolution()

func update_resolution() -> void:
	var viewport_size: Vector2 = get_viewport_rect().size
	$CanvasLayer.offset = viewport_size - Vector2(150, 70)
	$CanvasLayer/Coordinate_Label.position.x = -viewport_size.x + 150
	$CanvasLayer/Cash_Label.position.y = -viewport_size.y + 100

func is_mouse_hovering() -> bool:
	var state: bool = false
	for element: Node in $CanvasLayer.get_children():
		if element is Button:
			state = state or element.is_hovered()
	return state or $nation_picker/start_or_ready.is_hovered()

func unpress_all_buttons() -> void:
	for element: Node in $CanvasLayer.get_children():
		if element is Button and element.has_method("unpress") and element.active:
			element.unpress()
			state_machine.unpress_gui()

func are_all_buttons_unpressed() -> bool:
	var toReturn: bool = true
	var button_to_ignore: Button = $CanvasLayer/toggle_ownership
	for element: Node in $CanvasLayer.get_children():
		if element is Button and element != button_to_ignore:
			toReturn = toReturn && !element.active
	return toReturn

func update_coord_label(coords: Vector2i) -> void:
	coords_label.text = str(coords)
@rpc("authority", "unreliable")
func update_cash_label(new_cash: int) -> void:
	$CanvasLayer/Cash_Label.text = str(new_cash)

func update_desync_label(amount: int) -> void:
	$CanvasLayer/Desync_Label.text = str(amount)

func _on_station_button_pressed() -> void:
	state_machine.station_button_toggled()

func _on_track_button_pressed() -> void:
	state_machine.many_track_button_toggled()

func _on_depot_button_pressed() -> void:
	state_machine.depot_button_toggled()

func _on_single_track_button_pressed() -> void:
	state_machine.track_button_toggled()

func _on_factory_button_pressed() -> void:
	state_machine.factory_button_toggled()


func _on_start_or_ready_pressed() -> void:
	Utils.world_map.get_parent().disable_nation_picker()
	$nation_picker/start_or_ready.visible = false

func _on_road_depot_button_pressed() -> void:
	state_machine.building_road_depot_toggled()

func _on_music_pressed() -> void:
	Utils.click_music()
	if Utils.is_music_playing():
		$CanvasLayer2/Music.icon = preload("res://Gui/Icons/no.png")
	else:
		$CanvasLayer2/Music.icon = preload("res://Gui/Icons/go.png")
		
