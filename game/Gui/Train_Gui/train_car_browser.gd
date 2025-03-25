extends Window
signal purchase

# Called when the node enters the scene tree for the first time.
func _ready():
	hide()

func _on_close_requested():
	hide()



# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	$ItemList.size.y = size.y * 0.9
	$ItemList.size.x = size.x
	$ItemList.position.y = size.y * 0.1
	$ColorRect.size.y = size.y * 0.1
	$ColorRect.size.x = size.x
