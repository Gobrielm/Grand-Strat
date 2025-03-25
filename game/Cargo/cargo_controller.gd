extends Node

var map_node

func assign_map_node(_map_node):
	map_node = _map_node
	$day_tick.autostart = true
	$month_tick.autostart = true

func _on_day_tick_timeout():
	terminal_map._on_day_tick_timeout()
	map_node._on_day_tick_timeout()

func _on_month_tick_timeout():
	terminal_map._on_month_tick_timeout()
	map_node._on_month_tick_timeout()
