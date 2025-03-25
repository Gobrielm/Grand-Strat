extends TileMapLayer
func _ready():
	pass # Replace with function body.


func load_city(city):
	for x in 100:
		for y in 100:
			set_cell(Vector2i(x, y), 1, Vector2i(city[x][y], 0), 0)
