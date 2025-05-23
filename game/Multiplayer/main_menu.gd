extends Control
@onready var join_game_button: Button = $ColorRect/Join_Game
@onready var lobby: Node = get_parent()

var c: int = 0

func _on_create_game_pressed() -> void:
	if c == 1:
		lobby.start_game.rpc()
	else:
		lobby.create_game()
		c += 1


func _on_join_game_pressed() -> void:
	if join_game_button.get_child_count() == 0:
		var ip_box: TextEdit = TextEdit.new()
		join_game_button.add_child(ip_box)
		ip_box.size = Vector2(200, 50)
		ip_box.position.x -= ip_box.size.x / 2 - 25
		ip_box.position.y += 100
	else:
		var ip_box_text: String = join_game_button.get_child(0).text
		var ip_address: String = parse_valid_ip_address(ip_box_text)
		if ip_address == "-1":
			return
		lobby.join_game(ip_address)

func parse_valid_ip_address(input: String) -> String:
	if input.is_empty():
		#PBUG: DO NOT KEEP THIS
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
	
	
