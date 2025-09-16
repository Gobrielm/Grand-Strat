extends Window

func show_win(stats: Dictionary[String, int]) -> void:
	$Result.text = "Win"
	show_stats(stats)
	popup()

func show_loss(stats: Dictionary) -> void:
	$Result.text = "Loss"
	show_stats(stats)
	popup()

func show_stats(stats: Dictionary[String, int]) -> void:
	$Control/Casualties.set_item_text(1, "inf: " + str(stats.inf_dead))
	$Control/Casualties.set_item_text(2, "cav: " + str(stats.cav_dead))
	$Control/Casualties.set_item_text(3, "art: " + str(stats.art_dead))
	$Control/Casualties.set_item_text(4, "total: " + str(stats.art_dead + stats.cav_dead + stats.inf_dead))
	
	$Control/Kills.set_item_text(1, "inf: " + str(stats.inf_killed))
	$Control/Kills.set_item_text(2, "cav: " + str(stats.cav_killed))
	$Control/Kills.set_item_text(3, "art: " + str(stats.art_killed))
	$Control/Kills.set_item_text(4, "total: " + str(stats.art_killed + stats.cav_killed + stats.inf_killed))
