extends AudioStreamPlayer

var dir := DirAccess.open("res://Music")
var songs := []
var paused_status = 0.0

func _ready():
	for file: String in dir.get_files():
		if file.ends_with(".mp3"):
			songs.append(file)
	choose_random_song()
	Utils.assign_background_music(self)

func _on_finished():
	choose_random_song()
	play()

func click():
	if playing:
		paused_status = get_playback_position()
		stop()
	else:
		play(paused_status)
		paused_status = 0.0

func get_state() -> bool:
	return playing

func choose_random_song():
	var random_song_name: String = songs.pick_random()
	stream = load("res://Music/" + random_song_name)
