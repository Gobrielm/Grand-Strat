extends Control
@onready var join_game_button: Button = $TextureRect/Join_Game
@onready var lobby: Node = get_parent()

func _ready() -> void:
	randomize_texture_rect()

func _on_create_game_pressed() -> void:
	lobby.create_game()

func _on_join_game_pressed() -> void:
	var ip_box_text: String = $TextureRect/Join_Game/TextEdit.text
	var ip_address: String = parse_valid_ip_address(ip_box_text)
	if ip_address == "-1":
		return
	lobby.join_game(ip_address)

func parse_valid_ip_address(input: String) -> String:
	if input.is_empty():
		#TODO: DO NOT KEEP THIS
		input = "10.100.0.236"
	var ip_address_blocks: Array = ["0", "0", "0", "0"]
	var curr_block: int = 0
	var tracker: int = 0
	for letter: String in input:
		if letter == '.' and tracker <= 3:
			tracker = 0
			curr_block += 1
			continue
		elif letter == '.' and tracker > 3:
			return "-1"
		elif !letter.is_valid_int():
			return "-1"
		
		if tracker == 0:
			ip_address_blocks[curr_block] = letter
		else:
			ip_address_blocks[curr_block] += letter
		tracker += 1
	if curr_block != 3 or tracker > 3:
		return "-1"
	var address: String = ""
	for block: String in ip_address_blocks:
		address += block + "."
	address[-1] = ""
	return address
	
func randomize_texture_rect() -> void:
	var images: Array = []
	var dir: DirAccess = DirAccess.open("res://external_images")
	dir.list_dir_begin()
	var file_name: String = dir.get_next()
	while file_name != "":
		if !dir.current_is_dir() and (file_name.ends_with(".jpg") or file_name.ends_with(".png")):
			images.append(file_name)
		file_name = dir.get_next()
	
	$TextureRect.texture = load("res://external_images/" + str(images.pick_random()))
